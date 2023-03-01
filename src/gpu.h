#ifndef GPU_H_
#define GPU_H_

#include <stdint.h>

#define GPU_TARGET_FPS (6 * 5)
#define GPU_X 320
#define GPU_Y 240
#define GPU_MIN (GPU_X / 2 < GPU_Y ? GPU_X : GPU_Y)
#define GPU_BORDER_X (GPU_X / 2 > GPU_Y ? (GPU_X / 2 - GPU_Y) / 2 : 0)
#define GPU_BORDER_Y (GPU_X / 2 < GPU_Y ? (GPU_Y - GPU_X / 2) / 2 : 0)
#define GPU_REST (GPU_X / 2 - 2 * GPU_BORDER_X)
#define S1_CENTER_X (GPU_X / 4)
#define S1_CENTER_Y (GPU_Y / 2)
#define S2_CENTER_X (3 * GPU_X / 4)
#define S2_CENTER_Y (GPU_Y / 2)
#define B_WIDTH 8

#define GPU_Color uint16_t

#define RGB(r, g, b)                                                           \
  ((uint16_t)((r)&0xf8) | (((uint16_t)((b)&0x00f8)) << 5) |                    \
   (((g)&0x00e0) >> 5) | (((uint16_t)((g)&0x001c)) << 6))
#define C_WHITE RGB(255, 255, 255)
#define C_BLACK RGB(0, 0, 0)
#define C_BLUE RGB(0, 0, 255)
#define C_YELLOW RGB(255, 255, 0)
#define C_RED RGB(255, 0, 0)
#define C_GREEN RGB(0, 255, 0)
#define C_GRAY_LIGHT RGB(192, 192, 192)
#define C_GRAY_MEDIUM RGB(128, 128, 128)
#define C_GRAY_DARK RGB(64, 64, 64)
#define C_C1 RGB(39, 153, 216)
#define C_C2 RGB(216, 39, 153)
#define C_C3 RGB(153, 216, 39)

void GPU_Init();
void GPU_ClearFramebuffers();
void GPU_Render();

void GPU_DrawFilledCircle(GPU_Color color, int16_t x, int16_t y,
                          int16_t radius);
void GPU_DrawFilledSquare(GPU_Color color, int16_t x, int16_t y, int16_t width,
                          int16_t height);

#endif // GPU_H_
