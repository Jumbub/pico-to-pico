#ifndef GAMESHARED_H_
#define GAMESHARED_H_

#include "gpu.h"
#include "interface.h"
#include <stdint.h>

typedef enum {
  GS_NOT_STARTED = 0,
  GS_RUNNING = 1,
  GS_DONE = 2,
  GS_ANIMATING = 3
} GameState;

typedef struct {
  uint32_t memblocks[16 * 1024];
} SharedMemory;

typedef struct {
  SharedMemory sharedMemory;
  uint8_t touchPressed;
  uint16_t touch_X;
  uint16_t touch_Y;
  // vars starting with _ should be handled by each game
  GameState _gameState;
  uint32_t _lastRenderedTime;
  uint32_t _lastRenderedTick;
} MainMemory;

extern MainMemory mainMemory;

#endif // GAMESHARED_H_
