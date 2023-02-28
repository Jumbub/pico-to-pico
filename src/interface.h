#ifndef INTERFACE_H_
#define INTERFACE_H_

#define ONE_SECOND (1000 * 1000)

#include <stdint.h>
#include <stdio.h>

void IF_Setup();
uint32_t IF_GetCurrentTime();
uint32_t IF_Random();

void IF_DrawScreen(uint8_t *_framebuffer, size_t _framebufferSize);

uint8_t IF_Touchscreen();

#endif // INTERFACE_H_
