#ifndef SCREEN_DEMO_H
#define SCREEN_DEMO_H

#include "lcd_ili9341.h"

/* Debugger observables, not shared interrupt state. HAL_OK is transport-only. */
extern volatile HAL_StatusTypeDef screen_demo_status;
extern volatile uint16_t screen_demo_row;
extern volatile uint8_t screen_demo_pattern;

HAL_StatusTypeDef ScreenDemo_Init(SPI_HandleTypeDef *spi);
void ScreenDemo_Process(void);

#endif
