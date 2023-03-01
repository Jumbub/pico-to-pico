// Paint.NOT

#include "../mainLoop.h"
#include "../gpu.h"
#include "../interface.h"
#include "../gameshared.h"
#include <stdint.h>

GPU_Color _selected_color = 0;

#define NUM_TICKS 50
#define MAX_FRAMERATE 30
#define TIME_PERIOD (ONE_SECOND/MAX_FRAMERATE)

static const GPU_Color _color_list[10] = {C_WHITE, C_BLACK, C_GRAY_LIGHT, C_BLUE, C_GRAY_DARK, C_RED, C_GRAY_MEDIUM, C_GREEN, C_C1, C_C2};

#define MAX_DRAW_QUEUE 20
int8_t drawQueueI = 0;
struct Draw{
  uint16_t x;
  uint16_t y;
  GPU_Color color;
};
struct Draw drawQueue[MAX_DRAW_QUEUE];

static void G_Init(void)
{
    mainMemory._lastRenderedTime = 0;
    mainMemory.touch_X = 0;
    mainMemory.touch_Y = 0;
    _selected_color = C_WHITE;

    GPU_ClearFramebuffers();

    for (uint8_t i=0; i<10; i++)
    {
    	GPU_DrawFilledSquare(_color_list[i], 0, i*24, 20, 20);
    }
	mainMemory._lastRenderedTime = 0;
}


static void G_Update(void)
{
    if (mainMemory.touchPressed)
    {
    	if (mainMemory.touch_X >= 0 && mainMemory.touch_X < 24 && mainMemory.touch_Y >= 0 && mainMemory.touch_Y < 240)
		{
    		_selected_color = _color_list[mainMemory.touch_Y/24];
		}
    	if (mainMemory.touch_X >= 26 && mainMemory.touch_X < 360 && mainMemory.touch_Y >= 0 && mainMemory.touch_Y < 240)
    	{
        if (drawQueueI < MAX_DRAW_QUEUE) {
          drawQueue[drawQueueI].x = mainMemory.touch_X;
          drawQueue[drawQueueI].y = mainMemory.touch_Y;
          drawQueue[drawQueueI].y = _selected_color;
          drawQueueI++;

          GPU_DrawFilledCircle(_selected_color, mainMemory.touch_X, mainMemory.touch_Y, 3);
        }
    	}
    }
}

static void G_Draw(void)
{
	// only render, everything is already in the framebuffer, limit to 3FPS
	// GPU_ClearFramebuffers(); - use framebuffer as memory
	uint32_t time_now = IF_GetCurrentTime();
	if (time_now - mainMemory._lastRenderedTime > TIME_PERIOD)
	{
		mainMemory._lastRenderedTime = IF_GetCurrentTime();
	}
}

static void G_Deinit(void)
{

}

const Callbacks G10_Callbacks = { &G_Init, &G_Update, &G_Draw, &G_Deinit };
