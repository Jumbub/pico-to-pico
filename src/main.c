#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/cyw43_arch.h"

#include "gameshared.h"
#include "interface.h"
#include "mainLoop.h"
#include "mainLoop.h"
#include "gpu.h"

int main()
{
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }

    IF_Setup();
    GPU_Init();
    SetupCallbacks(&G10_Callbacks);

    DoMainLoop();
    return 0;
}
