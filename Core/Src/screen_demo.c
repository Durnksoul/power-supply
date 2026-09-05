#include "screen_demo.h"

volatile HAL_StatusTypeDef screen_demo_status = HAL_ERROR;
volatile uint16_t screen_demo_row;
volatile uint8_t screen_demo_pattern;

static uint16_t pixels[LCD_WIDTH];
static uint32_t hold_start;
static const uint16_t colors[] = {0xF800U, 0x07E0U, 0x001FU, 0xFFFFU, 0x0000U};
static const uint16_t bars[] = {0xFFFFU, 0xFFE0U, 0x07FFU, 0x07E0U,
                               0xF81FU, 0xF800U, 0x001FU, 0x0000U};

HAL_StatusTypeDef ScreenDemo_Init(SPI_HandleTypeDef *spi)
{
    screen_demo_row = 0U;
    screen_demo_pattern = 0U;
    screen_demo_status = LCD_ILI9341_Init(spi);
    return screen_demo_status;
}

void ScreenDemo_Process(void)
{
    if (screen_demo_status != HAL_OK) {
        return; /* Stop after first transport failure; status remains inspectable. */
    }
    if (screen_demo_row >= LCD_HEIGHT) {
        if ((uint32_t)(HAL_GetTick() - hold_start) < 1000U) {
            return;
        }
        screen_demo_pattern = (uint8_t)((screen_demo_pattern + 1U) % 6U);
        screen_demo_row = 0U;
    }
    for (uint32_t x = 0U; x < LCD_WIDTH; ++x) {
        pixels[x] = (screen_demo_pattern < 5U)
            ? colors[screen_demo_pattern]
            : bars[x / (LCD_WIDTH / 8U)];
    }
    screen_demo_status = LCD_ILI9341_WriteRow(screen_demo_row, pixels);
    if (screen_demo_status == HAL_OK) {
        ++screen_demo_row;
        if (screen_demo_row == LCD_HEIGHT) {
            hold_start = HAL_GetTick();
        }
    }
}
