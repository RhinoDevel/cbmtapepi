# CBM Tape Pi
Use a Raspberry Pi as datassette drive with your Commodore computer!

CBM Tape Pi is a Commodore datassette/tape drive replacement.

It is written in bare-metal C and independent of any OS (like Raspbian).

Use it like this:
- Connect some serial interface to Raspberry Pi's GPIO port (details will follow!).
- Connect Commodore tape port to Raspberry Pi's GPIO port (details will follow!). There are only some resistors necessary!
- Put compiled kernel.img (or kernel7.img for Raspberry Pi 2) on an SD card holding other necessary Pi boot files (easiest way is to use an SD card having Raspbian installed and overwrite the kernel file).
- Enter LOAD on Commodore computer.
- Send a PRG file to Raspberry Pi via serial connection and YMODEM protocol (example will follow!).
- Wait for Commodore computer to load your PRG file from Raspberry Pi!

Supported Commodore computers:
- CBM / PET computers (tested with 3032)
- VIC 20 / VC20
- C64
- other Commodore computer with tape interface (not tested, yet)

Supported Raspberry Pis:
- 1
- 2
- other Raspberry Pis (not tested, yet)
