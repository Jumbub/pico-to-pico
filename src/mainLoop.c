#include "mainLoop.h"

#include "gpu.h"
#include "interface.h"
#include "gameshared.h"

Callbacks _callbacks = { 0, 0, 0, 0 };

void SetupCallbacks(const Callbacks * callbacksNew)
{
    // Maybe a previous game needs to clear the heap
    if (_callbacks.deinitCallback) (*_callbacks.deinitCallback)();
    // Initialize/reset a new game
    if (callbacksNew->initCallback) (*callbacksNew->initCallback)();
    _callbacks = *callbacksNew;
}

void ResetGame()
{
    SetupCallbacks(&_callbacks);
}

static uint32_t _lastDrawTime;
static uint32_t _lastNetTime;

void DoMainLoop()
{
    _lastDrawTime = IF_GetCurrentTime();
    while (1)
    {
        // Register touch events
        IF_Touchscreen();
        // Do main loop logic
        if (_callbacks.updateCallback) (*_callbacks.updateCallback)();
        // Render screen at desired FPS
        if (_callbacks.drawCallback)
        {
            uint32_t new_draw_time = IF_GetCurrentTime();
            if ( new_draw_time - _lastDrawTime > (ONE_SECOND/GPU_TARGET_FPS) )
            {
                _lastDrawTime = new_draw_time;
                (*_callbacks.drawCallback)();
                GPU_Render();
            }
        };
        uint32_t new_net_time = IF_GetCurrentTime();
        if ( new_net_time - _lastNetTime > (ONE_SECOND*1) )
        {
            _lastNetTime = new_net_time;
        }
    }
}
