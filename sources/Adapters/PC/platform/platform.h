
#ifndef _PLATFORM_PC_H_
#define _PLATFORM_PC_H_

#include <stdint.h>
#include "System/Process/SysMutex.h"
#include "gpio.h"

void platform_init();
void platform_reboot();
void platform_bootloader();
SysMutex *platform_mutex();
uint32_t millis(void);
uint32_t micros(void);
void platform_brightness(uint8_t value);
int16_t battery_health();

#endif
