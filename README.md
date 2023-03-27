# Raspberry Pico Wifi Painter

A Raspberry Pico __RP2040__ and __ILI9341__ based painter drawing live updates to and from any other online Pico.

Forked from [this excellent Pocket Gaming project](https://github.com/GrgoMariani/Raspberry-Pico-Pocket-Gamer)

Case printed from [this Printables model](https://www.printables.com/model/147389-case-for-esp8266-nodemcu-12-e-with-ili9341-28-scre)

Short demo [video](https://photos.app.goo.gl/JW611p4d572n7nzj9)

![PXL_20230326_122822473](https://user-images.githubusercontent.com/8686526/227824570-4a2a6dbd-091f-4ba6-8d2d-0f6340522fc9.jpg)
![PXL_20230326_041922028 (1)](https://user-images.githubusercontent.com/8686526/227824564-06d9fc75-5f30-4705-a48a-7ff66ec215b8.jpg)

## Hardware
Required components:
* Raspberry Pico W (__RP2040__)
* __ILI9341__ controller with __XPT2046__ touch module (any size at or below 510x510 pixel resolution)

## How to compile/run ?

```bash
git clone git@github.com:raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init

# modify ./lib/lwip/src/include/lwip/opt.h:509
# FROM:      #define MEMP_NUM_SYS_TIMEOUT            LWIP_NUM_SYS_TIMEOUT_INTERNAL
# TO:        #define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL+1)
# until this issue is resolved: https://github.com/raspberrypi/pico-sdk/issues/1281

git clone git@github.com:jumbub/pico-to-pico.git
cd pico-to-pico
mkdir build
cd build
PICO_SDK_PATH=../../pico-sdk cmake -DPICO_BOARD=pico_w -DWIFI_SSID="ssid" -DWIFI_PASSWORD="password" -DMQTT_CLIENT="picow-1234" -DMQTT_TOPIC="draw-app-1234" ..
make
```

### Debugging

```bash
git clone https://github.com/raspberrypi/openocd.git
./bootstrap
./configure --enable-ftdi --enable-sysfsgpio --enable-bcm2835gpio
src/openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -s tcl -c "adapter speed 5000"
```

```bash
gdb-multiarch main.elf
target extended-remote :3333
(gdb) load # loads application
(gdb) monitor reset init # restart pico
(gdb) continue # run
```

After which you should be able to find `build/pico-to-pico.uf2` file to upload to your Pico microcontroller.

## Pinout
You should be able to find a compressed __gerber__ file with `extras` directory of the repository and order a new PCB online on which you can solder all the elements easily.

ILI9341 to Pico pin connections looks like this:

```
 ILI9341         Pico (RP2040)

 T_IRQ  <-----> PIN 20
 T_DO   <-----> PIN 8
 T_DIN  <-----> PIN 11
 T_CS   <-----> PIN 21
 T_CLK  <-----> PIN 10
 SDO    <-----> PIN 4
 LED    <-----> PIN 26 (or directly to 3v3)
 SCK    <-----> PIN 6
 SDI    <-----> PIN 7
 DC     <-----> PIN 18
 RESET  <-----> PIN 17
 CS     <-----> PIN 16
 GND    <-----> GND
 VCC    <-----> 3V3

```

In case you need to change them they are all defined within their own source files: `src/interface.c`, `src/interface/ili9341.c` and `src/interface/ili9341_touchscreen.c`.
