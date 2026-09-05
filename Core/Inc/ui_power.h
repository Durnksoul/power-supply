#ifndef UI_POWER_H
#define UI_POWER_H

#include <stdint.h>

/* Display-only model: no connection to any power-output control API. */
void UI_Power_Create(void);
void UI_Power_SetUptime(uint32_t seconds);
void UI_Power_SetRefreshRate(uint32_t tenths_fps);

#endif
