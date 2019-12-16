# CBM Tape Pi for Linux OS (e.g. Raspbian)
*Marcel Timm, RhinoDevel, 2019, [rhinodevel.com](http://rhinodevel.com/)*

This folder contains the source code of the Linux OS port of CBM Tape Pi.

If it would work reliable, you could use (e.g.) Raspbian running directly on a
Raspberry Pi and transfer files to and from Commodore machine.

Unfortunately the pulse generation for the tape data signal is not reliable,
because a Linux OS has a scheduler and switches between different processes.
This leads to pulses being sporadically longer than wanted.

I could not find a (simple) solution for that problem, yet.

There may be none.

Maybe it is possible with a custom Linux kernel module.

If you would like to investigate, take a look at function
```timer_wait_microseconds()``` in file [main.c](./main.c).
