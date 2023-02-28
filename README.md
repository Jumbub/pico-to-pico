# Raspberry Pico Wifi Painter

A Raspberry Pico __RP2040__ and __ILI9341__ based painter drawing live updates to and from any other online Pico.

[Forked from this excellent Pocket Gaming project](https://github.com/GrgoMariani/Raspberry-Pico-Pocket-Gamer)

## Description

The idea behind this project is to allow live collaborative drawing over the internet.

## Hardware
Required components:
* Raspberry Pico (__RP2040__)
* __ILI9341__ controller with __XPT2046__ touch module (I am using 3.2'' version)
* five switches (preferably mechanical switches)

> Optional components:
> * Printed __PCB__ (gerber available within `extras` folder of this repository)
> * Button (switch) 6mm

## How to compile/run ?

```bash
cd pico-sdk
git submodule update --init
cd ..
mkdir build
cd build
cmake ..
export PICO_SDK_PATH=../../pico-sdk
make
```

After which you should be able to find `build/pico-to-pico.uf2` file to upload to your Pico microcontroller.

> __HINT__: In case you want to try the project without the Pico board `cd` into the `extras` folder and do a `make run`. I've adapted the interface and the main function to run on top of the olc's PixelGameEngine so you can try it directly from your Linux PC. Controls are `QWASD` and mouse click.

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
