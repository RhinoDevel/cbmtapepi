# CBM Tape Pi for Linux OS (e.g. Raspbian)
*Marcel Timm, RhinoDevel, 2024, [rhinodevel.com](http://rhinodevel.com/)*

This folder contains the source code of the Linux OS port of CBM Tape Pi.

The compatibility mode of CBM Tape Pi must be used at least once to transfer the
fast mode wedge PRG to the Commodore machine.

Doing this with the CPU only is not possible as the timing requirements are not
met, because the Linux OS scheduler interrupts sending the signal.

But it can be done with DMA, e.g. via [pigpio](http://abyz.me.uk/rpi/pigpio/).

Initial tests with DMA were successful, but work on the first version of the
Linux port (which will give us easy access to cool features like WLAN) is not
done, yet.
