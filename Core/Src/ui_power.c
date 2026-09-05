#include "ui_power.h"
#include "lvgl.h"

LV_FONT_DECLARE(ui_font_zh_16);
static lv_obj_t *uptime_label;
static lv_obj_t *fps_label;

static lv_obj_t *label(lv_obj_t *parent, int32_t x, int32_t y,
                       const char *text, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *obj = lv_label_create(parent);
    lv_label_set_text(obj, text);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_obj_set_pos(obj, x, y);
    return obj;
}

static lv_obj_t *panel(lv_obj_t *parent, int32_t x, int32_t y,
                       int32_t width, int32_t height, uint32_t color)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, width, height);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, 10, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

void UI_Power_Create(void)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_remove_style_all(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B1018), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    label(screen, 12, 10, "数控电源", &ui_font_zh_16, 0xEDF2F7);
    lv_obj_t *badge = panel(screen, 253, 7, 55, 24, 0x243345);
    label(badge, 11, 3, "演示", &ui_font_zh_16, 0xACC8E8);

    lv_obj_t *voltage = panel(screen, 12, 40, 144, 112, 0x14212D);
    label(voltage, 10, 9, "电压", &ui_font_zh_16, 0x8DA7B8);
    label(voltage, 9, 31, "0.00", &lv_font_montserrat_40, 0x55D5F0);
    label(voltage, 120, 51, "V", &lv_font_montserrat_16, 0x55D5F0);
    label(voltage, 10, 89, "设定 12.00 V", &ui_font_zh_16, 0xC0CDD9);

    lv_obj_t *current = panel(screen, 164, 40, 144, 112, 0x252017);
    label(current, 10, 9, "电流", &ui_font_zh_16, 0xB4A58A);
    label(current, 9, 39, "0.000", &lv_font_montserrat_28, 0xFFCA72);
    label(current, 120, 51, "A", &lv_font_montserrat_16, 0xFFCA72);
    label(current, 10, 89, "限流 1.500 A", &ui_font_zh_16, 0xD6CBB6);

    lv_obj_t *summary = panel(screen, 12, 160, 296, 38, 0x151E29);
    label(summary, 10, 11, "0.00 W", &lv_font_montserrat_16, 0xE4ECF4);
    label(summary, 97, 11, "温度 --", &ui_font_zh_16, 0x8FA1B5);
    label(summary, 207, 11, "输出关闭", &ui_font_zh_16, 0xAAB8C7);

    fps_label = label(screen, 12, 215, "刷新率 -- FPS", &ui_font_zh_16, 0x91A4B9);
    label(screen, 168, 215, "运行", &ui_font_zh_16, 0x91A4B9);
    uptime_label = label(screen, 206, 215, "00:00:00", &lv_font_montserrat_14, 0x91A4B9);
}

void UI_Power_SetUptime(uint32_t seconds)
{
    /* Wrap the display at 100 hours to preserve its fixed layout. */
    lv_label_set_text_fmt(uptime_label, "%02lu:%02lu:%02lu",
                         (unsigned long)((seconds / 3600U) % 100U),
                         (unsigned long)((seconds / 60U) % 60U),
                         (unsigned long)(seconds % 60U));
}

void UI_Power_SetRefreshRate(uint32_t tenths_fps)
{
    lv_label_set_text_fmt(fps_label, "刷新率 %lu.%lu FPS",
        (unsigned long)(tenths_fps / 10U), (unsigned long)(tenths_fps % 10U));
}
