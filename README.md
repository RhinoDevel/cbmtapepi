# CBM Tape Pi
Use a Raspberry Pi as datassette drive with your Commodore computer!

CBM Tape Pi is a Commodore datassette/tape drive emulator.

With just two resistors and some cables (see picture below) you can send PRG files from your modern computer to a Commodore machine via serial connection and a Raspberry Pi.

It is written in bare-metal C and independent of any OS (like Raspbian).

## How to use

- Connect some serial interface to Raspberry Pi's GPIO port (see picture below).
- Connect Commodore tape port to Raspberry Pi's GPIO port (see picture below). There are only two resistors necessary!
- Put compiled kernel.img (or kernel7.img for Raspberry Pi 2) on an SD card holding other necessary Pi boot files (easiest way is to use an SD card having Raspbian installed and overwrite the kernel file).
- Enter LOAD on Commodore computer.
- Send a PRG file to Raspberry Pi via serial connection and YMODEM protocol (see example below).
- Wait for Commodore computer to load your PRG file from Raspberry Pi!
- Repeat this as often as you want.

## Connections
Connect sender (e.g. your PC), Raspberry Pi and Commodore machine this way:
![Wiring](./docs/Serial%20to%20CBM%20tape%20via%20Raspberry%20Pi%20(Marcel%20Timm%2C%20RhinoDevel).png)

## Example

How to use your computer's Linux commandline and a USB to serial interface at /dev/ttyUSB0 to send a PRG to Commodore machine:

Setup serial interface once (115200 baud, 8 data bits, no parity, 1 stop bit):
```shell
stty -F /dev/ttyUSB0 115200
```
Send PRG file to Raspberry Pi:
```shell
sx --ymodem mycbmapp.prg < /dev/ttyUSB0 > /dev/ttyUSB0
```
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

- Send files from Commodore machine to your modern computer.
- Read from / write to Raspberry Pi's SD card.
- Read from / write to a USB memory stick connected to Raspberry Pi.
- "Real"/complete tape drive emulation (e.g. to use fast loaders).
