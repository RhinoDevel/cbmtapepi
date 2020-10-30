***CBM Tape Pi** is **not** abandoned! I am planning on continuing work on the fast loader (etc.) after the next [Musical Keyboard](https://github.com/RhinoDevel/keyboard/blob/master/README.md) release is out. Stay tuned!*

# CBM Tape Pi v1.6.1
*Marcel Timm, RhinoDevel, 2019, [rhinodevel.com](http://rhinodevel.com/)*

![CBM Tape Pi hardware](./docs/title.jpg)

Use a Raspberry Pi as datassette drive with your Commodore 8-bit computer!

- Video: [CBM 3032 (please excuse the cuts, there is no fast loader, yet..)](https://youtu.be/CkLR3lkHjh4)

- Latest release: [CBM Tape Pi v1.6.1](https://github.com/RhinoDevel/cbmtapepi/releases/tag/v1.6.1)

## Features

- Access Pi's SD card from your Commodore to:
  - Load PRGs.
  - Save PRGs.
  - Traverse directories.
  - List current directory's content.
  - Delete files.

- Simple as possible, easy to build hardware interface:
  - No ICs, just a minimum amount of discrete components.
  - Only five connections need to be soldered (wires on the tape port plug).

## Project goals

### Done

- Load and save PRG files for "all" Commodore 8-bit machines without the need for extra software on Commodore machine.
- Use simplest and cheapest way to built such an interface.
- Use as few hardware components as possible.
- Use a Raspberry Pi and one of its mass storage devices.

### To-do

- Support loading PRG files with filenames longer than 12 characters.
- Fast load (and save).
- Custom commands (via wedge) instead of (mis-)using Commodore SAVE command.
- "Real"/complete tape drive emulation.
- MAYBE IMPOSSIBLE (kernel module?): Raspbian port of CBM Tape Pi, see [Linux README](./linux/README.md).

## Requirements

You need:

- Some cables (not that many, if you want to solder).
- 1x breadboard (if you do not want to solder) or a circuit board.
- 1x LED.
- 1x NPN transistor (e.g. a PN2222A or a BC547).
- 2x 10k Ohm resistors.
- 1x 11k Ohm resistor (e.g. use a 10k Ohm and a 1k Ohm resistor in series).
- 1x 16k Ohm resistor (you can also put this one together by using multiple resistors).
- 1x 1k Ohm resistor.
- 1x Commodore cassette port plug.
- 1x Raspberry Pi 1, Zero or 2 (more recent models may work, not tested, yet).
- 1x SD card (I am using an old 64MB card).

## How to use

- **Warning**: When using CBM Tape Pi, don't toggle the Commodore datassette connector ports that are (by Commodore default) used as inputs into output mode (e.g. via POKEs), because the 5V from the Commodore may harm the connected Raspberry Pi GPIO pins (max. of 3.3V allowed)!
- Setup connections (see [picture](./docs/CBM%20tape%20to%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png) below).
- Put compiled kernel.img (or kernel7.img for Raspberry Pi 2) from [latest release](https://github.com/RhinoDevel/cbmtapepi/releases/tag/v1.6.1) on an SD card's boot partition (easiest way is to use an SD card having Raspbian installed).
- Delete (if existing) config.txt and cmdline.txt from boot partition.
- Overwrite fixup.dat, start.elf and bootcode.bin with the files having the same names from [Raspberry Pi firmware release 1.20171029](https://github.com/raspberrypi/firmware/tree/1.20171029/boot).
- The Pi's ACT LED will flash every second to show that CBM Tape Pi is running.
- The interface's LED will be on, if waiting for commands (via SAVE).
- The interface's LED will be off during transfer of a PRG to Commodore (LOAD).
- The interface's LED will flash to indicate an error that occurred during last command execution (e.g. file not found), but it will still wait for the next command from the Commodore.
- Currently, the commands to the Pi will be send via Commodore SAVE command, if you have a (big) PRG loaded in memory, consider executing ```NEW``` before ```SAVE```, to avoid that the PRG will unnecessarily be sent to the Pi.
- **LOAD**: E.g. a PRG file named ```mycbmapp.prg```:

  ```
      SAVE"*MYCBMAPP.PRG":LOAD
      RUN
  ```
  For non-relocatable (non-BASIC) PRGs on a VIC 20 or a more recent machine (e.g. a C64):

  ```
      SAVE"*MYCBMAPP.PRG":LOAD,1,1
      ...
  ```
- **SAVE**: E.g. a PRG file named ```mynewapp```:

  ```
      SAVE"MYNEWAPP"
  ```
- **LIST**: List content of current directory:

  ```
      SAVE"$":LOAD
      RUN
  ```
- **CD**: Change directory, e.g. to subfolder named "petprgs":

  ```
      SAVE"CD PETPRGS"
  ```
  To return from a subfolder type:

  ```
      SAVE"CD .."
  ```
- **RM**: Remove a file, e.g. the file named "oldfile.prg":

  ```
      SAVE"RM OLDFILE.PRG"
  ```

## Connections
Connect Raspberry Pi and Commodore machine this way:
![Wiring](./docs/CBM%20tape%20to%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png)

- [Raspberry Pi GPIO connections](./docs/CBM%20Tape%20Pi%20-%20Raspberry%20Pi%20GPIO%20connections.png)
- [Commodore tape port connections](./docs/CBM%20Tape%20Pi%20-%20Commodore%20machine%20cassette%20port%20connections.png)

## Supported Commodore machines

- CBM / PET computers (tested with 3032).
- VIC 20 / VC20.
- C64.
- Other Commodore computer with tape interface (not tested, yet).

## Supported Raspberry Pis

- Raspberry Pi 1.
- Raspberry Pi 2 (v1.2 excluded).
- Raspberry Pi Zero.
- Other Raspberry Pis (not tested, yet).

![RhinoDevel](./data/rhino.bmp)
