***Warning**: This README is **work-in-progress**, I am currently creating the release that will bring you fast mode via wedge and Pi 3 support! Please come back soon to check out v1.7.0.*

# CBM Tape Pi
*Marcel Timm, RhinoDevel, 2021, [rhinodevel.com](http://rhinodevel.com/)*

![CBM Tape Pi hardware](./docs/title.jpg)

Use a Raspberry Pi as fast storage device with your Commodore 8-bit computer (CBM/PET, VIC 20, C64, etc.) via tape port connector!

CBM Tape Pi is using a very simple hardware interface and Open Source software, only.

- Latest release: CBM Tape Pi v1.7.0 **coming soon!**

- Older video (compatibility mode, v1.6.1): [CBM 3032 (please excuse the cuts, there was no fast mode, yet..)](https://youtu.be/CkLR3lkHjh4)

## Features

- Access Pi's SD card from your Commodore to:
  - Load PRGs.
  - Save PRGs.
  - Traverse directories.
  - List current directory's content.
  - Delete files.
  - **Fast mode** via wedge for CBM/PET, VIC-20 and C64.
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
- 1x Raspberry Pi 1, 2, 3 or Zero (other models may also work, not tested, yet).
- 1x SD card (I am using an old 64MB card).
- Optional: A push button (to be able to soft-reset the Raspberry Pi).

## Warning and disclaimer

- When using CBM Tape Pi, **don't** toggle the Commodore datassette connector ports that are (by Commodore default) used as inputs into output mode and vice versa (e.g. via POKEs), because that can cause damage to your Raspberry Pi or even to your precious Commodore (this is nothing special, it should also never be done, if a real datassette device is attached)!
- **I take no responsibility** for any damage caused by using CBM Tape Pi. But: I am sure that, if you follow this README and make no mistakes, there will be **no** damage.

## Preparations

- Setup connections (see [picture](./docs/CBM%20tape%20to%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png) below).
- Put compiled kernel.img (or kernel7.img for Raspberry Pi 2 and 3) from **latest release** *(coming soon!)* on an SD card's boot partition (easiest way is to use an SD card having Raspbian installed).
- Delete (if existing) config.txt and cmdline.txt from boot partition.
- Overwrite fixup.dat, start.elf and bootcode.bin with the files having the same names from [Raspberry Pi firmware release 1.20171029](https://github.com/raspberrypi/firmware/tree/1.20171029/boot).

## The LEDs

- The Pi's ACT LED will flash every second to show that CBM Tape Pi is running.
- The interface's LED will be on, if waiting for commands.
- The interface's LED will be off during transfer of a PRG to Commodore (LOAD).
- The interface's LED will flash to indicate an error that occurred during last command execution (e.g. file not found), but it will still wait for the next command from the Commodore.

## How to enter and use fast mode

*Description coming soon.*

## How to use compatibility mode

- The commands to the Pi will be send via Commodore SAVE command.
- If you have a (big) PRG loaded in memory, consider executing ```NEW``` before ```SAVE```, to avoid that the PRG will unnecessarily be sent to the Pi.
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

- CBM / PET computers (tested with BASIC v1, v2 & v4 on a 2001 and a 3032, tested with BASIC v4 on an 8032-SK).
- VIC 20.
- C64.
- Other Commodore computers with tape interface (not tested, yet).

## Supported Raspberry Pis

- Raspberry Pi 1 (successfully tested with Pi 1 Model B).
- Raspberry Pi 2 (successfully tested with Pi 2 Model B v1.1).
- Raspberry Pi 3 (successfully tested with Pi 3 Model B v1.2).
- Raspberry Pi Zero (successfully tested with Pi Zero W v1.1).
- Other Raspberry Pis (not tested, yet).

## Next project goals

- Make sure that ACT LED works on Raspberry Pi Zero and 3.
- Support loading PRG files with filenames longer than 12 characters.
- List directory content without erasing possibly existing PRG in memory when using fast mode.
- Show destination memory addresses and sizes of PRG files in directory listing.
- Increase loading speed for fast mode wedges, if top-of-memory shall be used.
- Show a result text on Commodore machine for at least some commands in fast mode. 
- Read/write from/to tape storage files like TAP, etc.
- Probably impossible, at least for compatibility mode (kernel module?): Raspbian port of CBM Tape Pi, see [Linux README](./linux/README.md).

![RhinoDevel](./data/rhino.bmp)
