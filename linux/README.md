# CBM Tape Pi for Linux OS (e.g. Raspbian)
*Marcel Timm, RhinoDevel, 2024, [rhinodevel.com](http://rhinodevel.com/)*

This folder contains the source code of the Linux OS port of CBM Tape Pi which
has work-in-progress state.

The compatibility mode of CBM Tape Pi must be used at least once to transfer the
fast mode wedge PRG to the Commodore machine.

Doing this with the CPU only is not possible as the timing requirements are not
met, because the Linux OS scheduler interrupts sending the signal.

But it can be done with DMA, which is finally **implemented**!!

### Current state (February 13., 2024):

- Sending program files to the PET 3032 (BASIC v4) **works**!
- Raspberry Pi detects, if fast mode wedge got loaded on the Commodore and
  enters fast mode.
- Raspberry Pi retrieves command ("name") via fast mode protocol correctly.
- Execute ```sudo dtoverlay gpio-no-irq``` before loading a fast mode wedge to
  the Commodore, if your Raspberry Pi freezes while doing
  that (see [this](https://github.com/raspberrypi/linux/issues/2550) issue on
  GitHub).

#### Next steps:

- Source code clean-up.
- Implement fast mode command handling.
- ...

### Nice things may happening in the future:

- Access a USB stick via Commodore.
- Internet/network access for the Commodore via Raspberry Pi.
- Use DMA controller to speed up fast mode further.
- Use Commodore as Linux terminal for Raspbian on the Raspberry Pi.
- ...
