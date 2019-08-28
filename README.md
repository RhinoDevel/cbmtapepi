README IS WORK IN PROGRESS, PLEASE COME BACK, SOON!

# CBM Tape Pi
*Marcel Timm, RhinoDevel, 2019, [rhinodevel.com](http://rhinodevel.com/)*

Use a Raspberry Pi as datassette drive with your Commodore computer!

<img src="./docs/Photo%20of%20connected%20Raspberry%20Pi%201.jpg" alt="Photo" width="400"/>

CBM Tape Pi is a Commodore datassette/tape drive emulator.

You can transfer PRG files between your modern computer and your Commodore machine via serial connection and a Raspberry Pi.

It is written in bare-metal C and independent of any OS (like Raspbian).

## What you need

One goal of this project is to require a minimum of hardware effort. It requires no ICs and soldering is optional.

You need:

- some cables
- a breadboard (if you do not want to solder)
- one LED
- one BJT NPN transistor
- one push button
- three 10000 Ohm resistors
- one 80000 Ohm resistor
- one 100000 Ohm resistor
- one 1000 Ohm resistor
- a Commodore tape port adapter
- a Raspberry Pi 1 or 2 (more recent models may work, not tested, yet)
- a modern computer with a serial interface to connect to Raspberry Pi (another Raspberry Pi is an option)

## How to use

- setup connections (see photo above and picture below)
- put compiled kernel.img (or kernel7.img for Raspberry Pi 2) on an SD card holding other necessary Pi boot files (easiest way is to use an SD card having Raspbian installed and overwrite the kernel file)

TODO: Add more info!

## Connections
Connect sender (e.g. your PC), Raspberry Pi and Commodore machine this way:
![Wiring](./docs/Serial%20to%20CBM%20tape%20via%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png)

## Example

How to use your computer's Linux commandline and a USB to serial interface at /dev/ttyUSB0 to send a PRG to Commodore machine:

TODO: Add examples using scripts and push button!

## Supported Commodore machines

- CBM / PET computers (tested with 3032)
- VIC 20 / VC20
- C64
- other Commodore computer with tape interface (not tested, yet)

## Supported Raspberry Pis

- 1
- 2
- other Raspberry Pis (not tested, yet)

## Possible future features

- read from / write to Raspberry Pi's SD card
- read from / write to a USB memory stick connected to Raspberry Pi
- "real"/complete tape drive emulation (e.g. to use fast loaders)
