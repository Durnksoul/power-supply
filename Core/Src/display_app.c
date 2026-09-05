#include "display_app.h"
#include "lcd_ili9341.h"
#include "ui_power.h"
#include "lvgl.h"

/* Single synchronous partial buffer: no DMA and no full frame allocation. */
static uint8_t draw_buffer[320U * 10U * 2U] __attribute__((aligned(4)));
volatile HAL_StatusTypeDef display_app_status = HAL_ERROR;
static uint32_t last_handler;
static uint32_t last_seconds;
static uint32_t fps_start;
static uint32_t completed_frames;

static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels)
{
    uint16_t width = (uint16_t)(area->x2 - area->x1 + 1);
    uint16_t height = (uint16_t)(area->y2 - area->y1 + 1);
    if (display_app_status == HAL_OK) {
        lv_draw_sw_rgb565_swap(pixels, (uint32_t)width * height);
        display_app_status = LCD_ILI9341_WriteArea((uint16_t)area->x1,
            (uint16_t)area->y1, width, height, pixels);
    }
    if (display_app_status == HAL_OK && lv_display_flush_is_last(display)) {
        ++completed_frames;
    }
    /* Release LVGL even on SPI failure; the application latches the error. */
    lv_display_flush_ready(display);
}

HAL_StatusTypeDef DisplayApp_Init(SPI_HandleTypeDef *spi)
{
    display_app_status = LCD_ILI9341_Init(spi);
    if (display_app_status != HAL_OK) return display_app_status;
    lv_init();
    lv_tick_set_cb(HAL_GetTick);
    lv_display_t *display = lv_display_create(320, 240);
    if (display == NULL) {
        display_app_status = HAL_ERROR;
        return display_app_status;
    }
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, draw_buffer, NULL, sizeof(draw_buffer),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush);
    UI_Power_Create();
    last_handler = HAL_GetTick();
    last_seconds = 0;
    fps_start = last_handler;
    completed_frames = 0;
    return HAL_OK;
}

void DisplayApp_Process(void)
{
    if (display_app_status != HAL_OK) return;
    uint32_t now = HAL_GetTick();
    uint32_t elapsed = now - fps_start;
    if (elapsed >= 1000U) {
        UI_Power_SetRefreshRate((uint32_t)((uint64_t)completed_frames * 10000U / elapsed));
        completed_frames = 0;
        fps_start = now;
    }
    uint32_t seconds = now / 1000U;
    if (seconds != last_seconds) {
        last_seconds = seconds;
        UI_Power_SetUptime(seconds);
    }
    if ((uint32_t)(now - last_handler) >= 5U) {
        last_handler = now;
        lv_timer_handler();
    }
}
