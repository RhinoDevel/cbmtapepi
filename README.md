***Warning**: This README is **work-in-progress**, I am currently in the middle of updating for CBM/PET fast mode support via wedge, please come back soon to check out v1.7.0!*

# CBM Tape Pi
*Marcel Timm, RhinoDevel, 2021, [rhinodevel.com](http://rhinodevel.com/)*

![CBM Tape Pi hardware](./docs/title.jpg)

Use a Raspberry Pi as storage device with your Commodore 8-bit computer via tape port connector!

The project's priority definitely is to deliver a **fast mass storage device for the CBM/PET machines** (check out the fast mode via wedge),
because there is no other datassette-port-only solution for them (as far as I know).

- Older video (compatibility mode, v1.6.1): [CBM 3032 (please excuse the cuts, there was no fast mode, yet..)](https://youtu.be/CkLR3lkHjh4)

- Latest release: CBM Tape Pi v1.7.0 **coming soon!**

## Features

- Access Pi's SD card from your Commodore to:
  - Load PRGs.
  - Save PRGs.
  - Traverse directories.
  - List current directory's content.
  - Delete files.
  - **Fast mode** via wedge (currently implemented for CBM/PET machines).
  - Compatibility mode for CBM/PET, VIC-20, C64, etc.

- Easy to build hardware interface:
  - No ICs, just a minimum amount of discrete components.
  - Only five connections need to be soldered (wires on the tape port plug).

## Requirements

You need:

- Some cables (not that many, if you want to solder).
- 1x breadboard (if you do not want to solder) or a circuit board.
- 1x LED.
- 3x NPN transistor (e.g. a BC547 or a PN2222A).
- 1x PNP transistor (e.g. a BC557).
- 10x 10k Ohm resistors (two of these are used to get a 20k Ohm resistor).
- 1x Commodore cassette port plug.
- 1x Raspberry Pi 1, Zero or 2 (more recent models may work, not tested, yet).
- 1x SD card (I am using an old 64MB card).
- Optional: A push button (to be able to soft-reset the Raspberry Pi).

## How to use in fast mode

*Description coming soon.*

## How to use in compatibility mode

- **Warning**: When using CBM Tape Pi, don't toggle the Commodore datassette connector ports that are (by Commodore default) used as inputs into output mode and vice versa (e.g. via POKEs), because that can cause damage to your Raspberry Pi or even to your precious Commodore (this is nothing special, it should also never be done, if a real datassette device is attached)!
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
      SAVE"MYCBMAPP.PRG":LOAD
      RUN
  ```
  For non-relocatable (non-BASIC) PRGs on a VIC 20 or a more recent machine (e.g. a C64):

  ```
      SAVE"MYCBMAPP.PRG":LOAD,1,1
      ...
  ```
- **SAVE**: E.g. a PRG file named ```mynewapp```:

  ```
      SAVE"+MYNEWAPP"
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
- **MODE**: Toggle mode (to be entered during start-up), e.g. to CBM/PET BASIC v4 fast mode:

  ```
      SAVE"MODE PET4"
  ```

## Connections
Connect Raspberry Pi and Commodore machine this way:
![Wiring](./docs/CBM%20tape%20to%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png)

- [Raspberry Pi GPIO connections](./docs/CBM%20Tape%20Pi%20-%20Raspberry%20Pi%20GPIO%20connections.png)
- [Commodore tape port connections](./docs/CBM%20Tape%20Pi%20-%20Commodore%20machine%20cassette%20port%20connections.png)

## Supported Commodore machines

- CBM / PET computers (tested with 3032 and 8032-SK).
- VIC 20 / VC20.
- C64.
- Other Commodore computer with tape interface (not tested, yet).

## Supported Raspberry Pis

- Raspberry Pi 1.
- Raspberry Pi 2 (v1.2 excluded).
- Raspberry Pi Zero.
- Other Raspberry Pis (not tested, yet).

## Next project goals

- Support loading PRG files with filenames longer than 12 characters.
- List directory content without erasing possibly existing PRG in memory.
- Optionally select top of RAM for storing the fast mode wedge.
- Read/write from/to tape storage files like TAP, etc.
- Probably impossible, at least for compatibility mode (kernel module?): Raspbian port of CBM Tape Pi, see [Linux README](./linux/README.md).

![RhinoDevel](./data/rhino.bmp)
