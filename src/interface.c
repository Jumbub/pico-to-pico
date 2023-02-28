#include "interface.h"

#include <stdio.h>
#include <stdlib.h>

#include "gameshared.h"
#include "interface/ili9341.h"
#include "interface/ili9341_touchscreen.h"

#include "pico/time.h"

static uint32_t startTime = 0;
void IF_Setup()
{
	startTime = IF_GetCurrentTime();
	ili9341_init_SPI();
	ili9341_init_display();
	ili9341_init_drawing();
	ILI9341_T_Init();
}

uint32_t IF_GetCurrentTime()
{
	return to_us_since_boot(get_absolute_time());
}

#include "gpu.h"

void IF_DrawScreen(uint8_t * _framebuffer, size_t _framebufferSize)
{
	ili9341_write_buffer(_framebuffer, _framebufferSize);
}

static uint32_t last_seed = 0xdeadb0d1;

uint32_t IF_Random()
{
    uint32_t now = IF_GetCurrentTime();
    uint32_t result = ((uint32_t)(142*now * now + 13*now + 203) >> 4)+last_seed;
    last_seed = result;
    return result;
}

uint8_t IF_Touchscreen()
{
	uint8_t result = ILI9341_T_TouchGetCoordinates(&mainMemory.touch_X, &mainMemory.touch_Y);
	mainMemory.touchPressed = result;
	return result;
}
