# CBM Tape Pi v1.5.1
*Marcel Timm, RhinoDevel, 2019, [rhinodevel.com](http://rhinodevel.com/)*

Use a Raspberry Pi as datassette drive with your Commodore computer!

<img src="./docs/Photo%20of%20connected%20Raspberry%20Pi%201.jpg" alt="Photo" title="Photo with overlays containing all relevant informations to build CBM Tape Pi connections yourself." width="800"/>

CBM Tape Pi is a Commodore datassette/tape drive emulator.

- you can transfer PRG files between your modern computer and your Commodore machine via serial connection and a Raspberry Pi, just by using built-in LOAD and SAVE datassette commands on your Commodore machine
- it is written in bare-metal C and independent of any OS on Raspberry Pi (like Raspbian)
- it works with any OS on your modern computer, if an application is installed that can transfer files via serial connection and YMODEM protocol

## Project goals

### Done

- load and save PRG files for "all" Commodore 8-bit machines without the need for extra software on Commodore machine
- use simplest and cheapest way to built such an interface
- use as few hardware components as possible

### To-do

- read from / write to Raspberry Pi's SD card (make serial interface "obsolete")
- read from / write to a USB memory stick connected to Raspberry Pi (make serial interface "obsolete")
- "real"/complete tape drive emulation (e.g. to use fast loaders)
- MAYBE IMPOSSIBLE: Raspbian port of CBM Tape Pi, see [Linux README](./linux/README.md)

## Requirements

The hardware effort is kept as low as possible. There are no ICs required and soldering is optional. A beginner can put this together!

You need:

- some cables (not that many, if you want to solder)
- a breadboard (if you do not want to solder) or a circuit board
- one LED
- one NPN transistor (e.g. a PN2222A or a BC547)
- one push button
- three 10k Ohm resistors
- one 80k Ohm resistor (you can put this together by using multiple resistors)
- one 100k Ohm resistor
- one 1k Ohm resistor
- a Commodore cassette port plug
- a Raspberry Pi 1 or 2 (more recent models may work, not tested, yet)
- an SD card (I am using an old 64MB card)
- a modern computer with a serial interface to connect to Raspberry Pi (another Raspberry Pi is an option, because a serial port is included)

## How to use

- setup connections (see [photo](./docs/Photo%20of%20connected%20Raspberry%20Pi%201.jpg) above or [picture](./docs/Serial%20to%20CBM%20tape%20via%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png) below)
- put compiled kernel.img (or kernel7.img for Raspberry Pi 2) on an SD card holding other necessary Pi boot files (easiest way is to use an SD card having Raspbian installed and overwrite the kernel file)
- **LOAD** - Sending PRG file to Commodore machine:
  - if the LED is on, SAVE mode is active and you need to press the button to enable LOAD mode  
  - enter LOAD on Commodore computer
  - send a PRG file to Raspberry Pi via serial connection and YMODEM protocol (see examples below)
  - wait for Commodore computer to load your PRG file from Raspberry Pi
  - repeat this as often as you want
- **SAVE** - Receiving PRG file from Commodore machine:
  - if the LED is off, LOAD mode is active and you need to press the button to enabled SAVE mode
  - start application waiting for file to receive via serial connection and YMODEM protocol (see examples below)
  - enter SAVE on Commodore machine (it is a good idea to add a name for the PRG file, too)
  - wait for your modern computer to retrieve your PRG file from Raspberry Pi
  - repeat this as often as you want
- pressing the button to toggle between LOAD and SAVE mode also cancels a maybe running operation - if something went wrong, just press the button twice (wait for the LED to toggle between presses) to reset the Raspberry Pi into the same mode
- on mode (LOAD or SAVE) initialization (triggered by cancel or mode toggle via button by user or triggered by an error) the status LED will flash some times to indicate entering mode

## Connections
Connect sender (e.g. your PC), Raspberry Pi and Commodore machine this way:
![Wiring](./docs/Serial%20to%20CBM%20tape%20via%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png)

## Examples

- An easy way is to use a Linux OS and the CBM Tape Pi Bash scripts that are included in the release package, just follow the instructions shown at commandline during execution:
  - Use [send.sh](./send.sh) to send files to Raspberry Pi / Commodore machine.
  - Use [retrieve.sh](./retrieve.sh) to receive files from Raspberry Pi / Commodore machine.
  - You can modify stuff in the script files (e.g. the serial interface device to use).

- How to manually use your computer's Linux commandline and a USB to serial interface at /dev/ttyUSB0:

  - Setup serial interface once (115200 baud, 8 data bits, no parity, 1 stop bit):
      ```shell
      stty -F /dev/ttyUSB0 115200
      ```
  - Send PRG file to Raspberry Pi:
      ```shell
      sx --ymodem mycbmapp.prg < /dev/ttyUSB0 > /dev/ttyUSB0
      ```
  - Receive PRG file from Raspberry Pi:
      ```shell
      rx --ymodem < /dev/ttyUSB0 > /dev/ttyUSB0
      ```

- If you want to use Microsoft Windows OS instead of some Linux OS (or some other Unix derivative), you may want to give [TeraTerm](https://ttssh2.osdn.jp/) a try (it has YMODEM send and receive features). 

- Using [Minicom](https://salsa.debian.org/minicom-team/minicom) is another option to send and receive via YMODEM protocol. 

## Supported Commodore machines

- CBM / PET computers (tested with 3032)
- VIC 20 / VC20
- C64
- other Commodore computer with tape interface (not tested, yet)

## Supported Raspberry Pis

- 1
- 2
- other Raspberry Pis (not tested, yet)

![RhinoDevel](./data/rhino.bmp)
