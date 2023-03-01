#ifndef GAMESHARED_H_
#define GAMESHARED_H_

#include "gpu.h"
#include "interface.h"
#include <stdint.h>

typedef struct {
  uint8_t touchPressed;
  uint16_t touch_X;
  uint16_t touch_Y;
  uint32_t _lastRenderedTime;
  uint32_t _lastRenderedTick;
} MainMemory;

extern MainMemory mainMemory;

#endif // GAMESHARED_H_
