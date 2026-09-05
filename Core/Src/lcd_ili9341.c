#include "lcd_ili9341.h"

/* MSP2807 vendor F407 example's panel power/gamma sequence, ported to HAL.
 * Wiring follows the user's SPI2 schematic, NOT the vendor SPI1 example.
 * LED is wired to 3.3 V: no software backlight GPIO exists.
 */
static SPI_HandleTypeDef *lcd_spi;
static uint8_t row_bytes[LCD_WIDTH * 2U];
static uint8_t initialized;

typedef struct {
    uint8_t command;
    uint8_t size;
    uint8_t data[15];
} PanelCommand;

static const PanelCommand panel_init[] = {
    {0xCF, 3, {0x00, 0xC9, 0x30}},
    {0xED, 4, {0x64, 0x03, 0x12, 0x81}},
    {0xE8, 3, {0x85, 0x10, 0x7A}},
    {0xCB, 5, {0x39, 0x2C, 0x00, 0x34, 0x02}},
    {0xF7, 1, {0x20}},
    {0xEA, 2, {0x00, 0x00}},
    {0xC0, 1, {0x1B}},
    {0xC1, 1, {0x00}},
    {0xC5, 2, {0x30, 0x30}},
    {0xC7, 1, {0xB7}},
    {0x36, 1, {0xA8}}, /* Landscape rotated 180 degrees: MY | MV | BGR. */
    {0x3A, 1, {0x55}}, /* RGB565. */
    {0xB1, 2, {0x00, 0x1A}},
    {0xB6, 2, {0x0A, 0xA2}},
    {0xF2, 1, {0x00}},
    {0x26, 1, {0x01}},
    {0xE0, 15, {0x0F,0x2A,0x28,0x08,0x0E,0x08,0x54,0xA9,
                0x43,0x0A,0x0F,0x00,0x00,0x00,0x00}},
    {0xE1, 15, {0x00,0x15,0x17,0x07,0x11,0x06,0x2B,0x56,
                0x3C,0x05,0x10,0x0F,0x3F,0x3F,0x0F}}
};

static HAL_StatusTypeDef command(uint8_t cmd, const uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;
    /* Both devices share SPI2. Touch stays deselected during LCD traffic. */
    HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
    status = HAL_SPI_Transmit(lcd_spi, &cmd, 1U, 20U);
    if ((status == HAL_OK) && (size != 0U)) {
        HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
        /* HAL's transmit API is non-const, but does not modify the payload. */
        status = HAL_SPI_Transmit(lcd_spi, (uint8_t *)data, size, 20U);
    }
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    return status;
}

HAL_StatusTypeDef LCD_ILI9341_Init(SPI_HandleTypeDef *spi)
{
    initialized = 0U;
    if ((spi == NULL) || (spi->Instance != SPI2)) {
        return HAL_ERROR;
    }
    lcd_spi = spi;
    HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(20U);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(120U);

    for (uint32_t i = 0U; i < sizeof(panel_init) / sizeof(panel_init[0]); ++i) {
        HAL_StatusTypeDef status = command(panel_init[i].command,
                                           panel_init[i].data, panel_init[i].size);
        if (status != HAL_OK) {
            return status;
        }
    }
    HAL_StatusTypeDef status = command(0x11U, NULL, 0U); /* Sleep out. */
    if (status != HAL_OK) {
        return status;
    }
    HAL_Delay(120U);
    status = command(0x29U, NULL, 0U); /* Display on. */
    if (status != HAL_OK) {
        return status;
    }
    HAL_Delay(20U);
    initialized = 1U;
    return HAL_OK;
}

HAL_StatusTypeDef LCD_ILI9341_WriteRow(uint16_t y, const uint16_t *pixels)
{
    if ((initialized == 0U) || (pixels == NULL) || (y >= LCD_HEIGHT)) {
        return HAL_ERROR;
    }
    const uint16_t last_x = LCD_WIDTH - 1U;
    const uint8_t columns[4] = {0U, 0U, (uint8_t)(last_x >> 8), (uint8_t)last_x};
    const uint8_t rows[4] = {(uint8_t)(y >> 8), (uint8_t)y,
                             (uint8_t)(y >> 8), (uint8_t)y};
    HAL_StatusTypeDef status = command(0x2AU, columns, sizeof(columns));
    if (status != HAL_OK) {
        return status;
    }
    status = command(0x2BU, rows, sizeof(rows));
    if (status != HAL_OK) {
        return status;
    }
    for (uint32_t x = 0U; x < LCD_WIDTH; ++x) {
        row_bytes[2U * x] = (uint8_t)(pixels[x] >> 8);
        row_bytes[2U * x + 1U] = (uint8_t)pixels[x];
    }
    /* One row takes about 4 ms at 1.3125 MHz; no full framebuffer required. */
    return command(0x2CU, row_bytes, sizeof(row_bytes));
}

HAL_StatusTypeDef LCD_ILI9341_WriteArea(uint16_t x, uint16_t y,
                                      uint16_t width, uint16_t height,
                                      const uint8_t *pixels)
{
    if ((initialized == 0U) || (pixels == NULL) || (width == 0U) ||
        (height == 0U) || (x >= LCD_WIDTH) || (y >= LCD_HEIGHT) ||
        (width > LCD_WIDTH - x) || (height > LCD_HEIGHT - y)) {
        return HAL_ERROR;
    }
    const uint16_t last_x = x + width - 1U;
    const uint16_t last_y = y + height - 1U;
    const uint8_t columns[4] = {(uint8_t)(x >> 8), (uint8_t)x,
                              (uint8_t)(last_x >> 8), (uint8_t)last_x};
    const uint8_t rows[4] = {(uint8_t)(y >> 8), (uint8_t)y,
                           (uint8_t)(last_y >> 8), (uint8_t)last_y};
    HAL_StatusTypeDef status = command(0x2AU, columns, sizeof(columns));
    if (status != HAL_OK) return status;
    status = command(0x2BU, rows, sizeof(rows));
    if (status != HAL_OK) return status;

    uint8_t write_memory = 0x2CU;
    HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
    status = HAL_SPI_Transmit(lcd_spi, &write_memory, 1U, 20U);
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
    /* Keep CS asserted across the rectangle; each transfer is at most one row. */
    const uint16_t row_size = width * 2U;
    for (uint16_t row = 0U; (row < height) && (status == HAL_OK); ++row) {
        status = HAL_SPI_Transmit(lcd_spi, (uint8_t *)pixels, row_size, 20U);
        pixels += row_size;
    }
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    return status;
}
