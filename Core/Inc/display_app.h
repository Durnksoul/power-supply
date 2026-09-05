#ifndef DISPLAY_APP_H
#define DISPLAY_APP_H
#include "main.h"
extern volatile HAL_StatusTypeDef display_app_status;
HAL_StatusTypeDef DisplayApp_Init(SPI_HandleTypeDef *spi);
void DisplayApp_Process(void);
#endif
