README IS WORK IN PROGRESS, PLEASE COME BACK, SOON!

# CBM Tape Pi
*Marcel Timm, RhinoDevel, 2019, [rhinodevel.com](http://rhinodevel.com/)*

Use a Raspberry Pi as datassette drive with your Commodore computer!

<img src="./docs/Photo%20of%20connected%20Raspberry%20Pi%201.jpg" alt="Photo" title="Photo with overlays containing all relevant informations to build CBM Tape Pi connections yourself." width="800"/>

CBM Tape Pi is a Commodore datassette/tape drive emulator.

You can transfer PRG files between your modern computer and your Commodore machine via serial connection and a Raspberry Pi, just by using built-in LOAD and SAVE datassette commands on your Commodore machine!

It is written in bare-metal C and independent of any OS (like Raspbian).

## Project goals

### Reached

- load and save PRG files for "all" Commodore 8-bit machines without the need for extra software on Commodore machine
- use simplest way to built such an interface
- use cheapest way to built such an interface
- use as few hardware components as possible

### To-do

- read from / write to Raspberry Pi's SD card (make serial interface "obsolete")
- read from / write to a USB memory stick connected to Raspberry Pi (make serial interface "obsolete")
- "real"/complete tape drive emulation (e.g. to use fast loaders)
- MAYBE IMPOSSIBLE: Raspbian port of CBM Tape Pi, see [Linux README](./linux/README.md).

## What you need

The hardware effort is kept as low as possible. There are no ICs required and soldering is optional. A beginner can put this together!

You need:

- some cables (not that many, if you want to solder)
- a breadboard (if you do not want to solder)
- one LED
- one NPN transistor (e.g. a PN2222A)
- one push button
- three 10000 Ohm resistors
- one 80000 Ohm resistor
- one 100000 Ohm resistor
- one 1000 Ohm resistor
- a Commodore tape port adapter
- a Raspberry Pi 1 or 2 (more recent models may work, not tested, yet)
- a modern computer with a serial interface to connect to Raspberry Pi (another Raspberry Pi is an option)

## How to use

- setup connections (see photo above or picture below)
- put compiled kernel.img (or kernel7.img for Raspberry Pi 2) on an SD card holding other necessary Pi boot files (easiest way is to use an SD card having Raspbian installed and overwrite the kernel file)

TODO: Add more info!

## Connections
Connect sender (e.g. your PC), Raspberry Pi and Commodore machine this way:
![Wiring](./docs/Serial%20to%20CBM%20tape%20via%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png)

## Example

How to use your computer's Linux commandline and a USB to serial interface at /dev/ttyUSB0 to send a PRG to Commodore machine:

TODO: Add examples using scripts and push button!

If you want to use Microsoft Windows OS instead of some Linux OS (or some other Unix derivative), you may want to give [TeraTerm](https://ttssh2.osdn.jp/) a try (it has YMODEM send and receive features). 

## Supported Commodore machines

- CBM / PET computers (tested with 3032)
- VIC 20 / VC20
- C64
- other Commodore computer with tape interface (not tested, yet)

## Supported Raspberry Pis

- 1
- 2
- other Raspberry Pis (not tested, yet)
