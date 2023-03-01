cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_C_STANDARD 11)

project(main C CXX ASM)

pico_sdk_init()

add_definitions(-DWIFI_SSID="${WIFI_SSID}" -DWIFI_PASSWORD="${WIFI_PASSWORD}")
add_definitions(-DMQTT_CLIENT="${MQTT_CLIENT}" -DMQTT_TOPIC="${MQTT_TOPIC}")
add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        # -Os
        -g
        )
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

pico_enable_stdio_uart(main 1)
pico_enable_stdio_usb(main 1)

target_link_libraries(main
  pico_cyw43_arch_lwip_poll
  pico_stdlib
  # pico_lwip_mbedtls
  # pico_mbedtls
  pico_lwip_mqtt
  hardware_spi
)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_uf2_output(main)
