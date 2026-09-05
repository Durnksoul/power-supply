#ifndef LCD_ILI9341_H
#define LCD_ILI9341_H

#include "main.h"

#define LCD_WIDTH  320U
#define LCD_HEIGHT 240U

/* Synchronous, main-context-only API. RGB565 pixels are host-endian. */
HAL_StatusTypeDef LCD_ILI9341_Init(SPI_HandleTypeDef *spi);
HAL_StatusTypeDef LCD_ILI9341_WriteRow(uint16_t y, const uint16_t *pixels);
/* RGB565 bytes in panel order (MSB first), tightly packed rectangle. */
HAL_StatusTypeDef LCD_ILI9341_WriteArea(uint16_t x, uint16_t y,
                                      uint16_t width, uint16_t height,
                                      const uint8_t *pixels);

#endif
