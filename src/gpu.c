#include "gpu.h"
#include "interface.h"

#include <string.h>
#include <math.h>


static uint16_t _framebuffer[GPU_X*GPU_Y];  // GPU VRAM (16bits per color)


void GPU_Init()
{
    GPU_ClearFramebuffers();
}

void GPU_ClearFramebuffers()
{
    memset(_framebuffer, 0, sizeof(_framebuffer));
}



void GPU_Render()
{
    IF_DrawScreen((uint8_t *)_framebuffer, sizeof(_framebuffer));
}

void GPU_DrawPixel(GPU_Color color, uint16_t x, uint16_t y)
{
    if (x >= GPU_X || y >= GPU_Y)
        return;
    uint16_t * pix = &_framebuffer[(GPU_X-x-1)*GPU_Y+y];
    (*pix) = (uint16_t) color;
}

void GPU_DrawLine(GPU_Color color, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    // Bresenhams line algorithm
    x0 = (x0 >= GPU_X) ? GPU_X-1 : x0;
    x1 = (x1 >= GPU_X) ? GPU_X-1 : x1;
    y0 = (y0 >= GPU_Y) ? GPU_Y-1 : y0;
    y1 = (y1 >= GPU_Y) ? GPU_Y-1 : y1;
    int16_t dx =  x1>x0 ? x1-x0 : x0-x1;
    int16_t sx = x0<x1 ? 1 : -1;
    int16_t dy = y1 > y0 ? -(y1-y0): y1-y0;
    int16_t sy = y0<y1 ? 1 : -1;
    int16_t err = dx+dy;  /* error value e_xy */
    while (1)   /* loop */
    {
        GPU_DrawPixel(color, x0, y0);
        if (x0 == x1 && y0 == y1)
            break;
        int16_t e2 = 2*err;
        if (e2 >= dy) /* e_xy+e_x > 0 */
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) /* e_xy+e_y < 0 */
        {
            err += dx;
            y0 += sy;
        }
    }
}

void GPU_DrawFilledSquare(GPU_Color color, int16_t x, int16_t y, int16_t width, int16_t height)
{
    for (int16_t i=0; i<width; i++)
    {
        for (int16_t j=0; j<height; j++)
        {
            GPU_DrawPixel(color, x+i, y+j);
        }
    }
}

void GPU_DrawEmptyCircle(GPU_Color color, int16_t x, int16_t y, int16_t radius)
{
    uint16_t xd = 0, yd = radius;
    int16_t d = 3 - 2 * radius;
    GPU_DrawPixel(color, x+xd, y+yd);
    GPU_DrawPixel(color, x-xd, y+yd);
    GPU_DrawPixel(color, x+xd, y-yd);
    GPU_DrawPixel(color, x-xd, y-yd);
    GPU_DrawPixel(color, x+yd, y+xd);
    GPU_DrawPixel(color, x-yd, y+xd);
    GPU_DrawPixel(color, x+yd, y-xd);
    GPU_DrawPixel(color, x-yd, y-xd);
    while (yd >= xd)
    {
        xd++;
        if (d > 0)
        {
            yd--;
            d = d + 4 * (xd - yd) + 10;
        }
        else
            d = d + 4 * xd + 6;
        GPU_DrawPixel(color, x+xd, y+yd);
        GPU_DrawPixel(color, x-xd, y+yd);
        GPU_DrawPixel(color, x+xd, y-yd);
        GPU_DrawPixel(color, x-xd, y-yd);
        GPU_DrawPixel(color, x+yd, y+xd);
        GPU_DrawPixel(color, x-yd, y+xd);
        GPU_DrawPixel(color, x+yd, y-xd);
        GPU_DrawPixel(color, x-yd, y-xd);
    }
}

void GPU_DrawFilledCircle(GPU_Color color, int16_t x, int16_t y, int16_t radius)
{
    uint16_t xd = 0, yd = radius;
    int16_t d = 3 - 2 * radius;
    int16_t mxxd = x-xd > 0 ? x-xd : 0;
    int16_t myyd = y-yd > 0 ? y-yd : 0;
    int16_t mxyd = x-yd > 0 ? x-yd : 0;
    int16_t myxd = y-xd > 0 ? y-xd : 0;
    GPU_DrawLine(color, mxxd, myyd, x+xd, myyd);
    GPU_DrawLine(color, mxxd, y+yd, x+xd, y+yd);
    GPU_DrawLine(color, mxyd, myxd, x+yd, myxd);
    GPU_DrawLine(color, mxyd, y+xd, x+yd, y+xd);
    while (yd >= xd)
    {
        xd++;
        if (d > 0)
        {
            yd--;
            d = d + 4 * (xd - yd) + 10;
        }
        else
            d = d + 4 * xd + 6;

        mxxd = x-xd > 0 ? x-xd : 0;
        myyd = y-yd > 0 ? y-yd : 0;
        mxyd = x-yd > 0 ? x-yd : 0;
        myxd = y-xd > 0 ? y-xd : 0;
        GPU_DrawLine(color, x-xd, y-yd, x+xd, y-yd);
        GPU_DrawLine(color, x-xd, y+yd, x+xd, y+yd);
        GPU_DrawLine(color, x-yd, y-xd, x+yd, y-xd);
        GPU_DrawLine(color, x-yd, y+xd, x+yd, y+xd);
    }
}
