# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Os")

include(pico_sdk_import.cmake)

project(main C CXX ASM)

pico_sdk_init()

add_executable(main
        src/main.c
        src/interface/ili9341.c
        src/interface/ili9341_touchscreen.c
        src/gameshared.c
        src/gpu.c
        src/interface.c
        src/mainLoop.c
        src/games/game10.c
        )

pico_set_program_name(main "PicoOnlinePainter")
pico_set_program_version(main "1.0")

pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 0)

target_link_libraries(main
  pico_cyw43_arch_lwip_threadsafe_background
  # pico_cyw43_arch_none
  pico_stdlib
  hardware_spi
)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

pico_add_extra_outputs(main)
