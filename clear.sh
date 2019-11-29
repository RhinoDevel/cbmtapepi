#!/bin/sh

# Marcel Timm, RhinoDevel, 2019aug22

# Hardware driver files:

rm hardware/armtimer/armtimer.o
rm hardware/systimer/systimer.o
rm hardware/busywait/busywait.o
rm hardware/gpio/gpio.o
rm hardware/mailbox/mailbox.o
rm hardware/miniuart/miniuart.o
rm hardware/pl011uart/pl011uart.o
rm hardware/framebuffer/framebuffer.o
rm hardware/watchdog/watchdog.o
rm hardware/irqcontroller/irqcontroller.o
rm hardware/sdcard/sdcard.o

# Hardware-independent library:

rm lib/str/str.o
rm lib/calc/calc.o
rm lib/console/console.o
rm lib/mem/mem.o
rm lib/alloc/alloc.o
rm lib/alloc/alloc_mem.o
rm lib/alloc/nodemem.o
rm lib/basic/basic.o
rm lib/video/video.o
rm lib/xmodem/xmodem.o
rm lib/ymodem/ymodem.o
rm lib/ff14/source/diskio.o
rm lib/ff14/source/ff.o

# Application-specific files:

rm app/boot.o
rm app/kernel_main.o
rm app/statetoggle/statetoggle.o
rm app/tape/tape_fill_buf.o
rm app/tape/tape_sample.o
rm app/tape/tape_send.o
rm app/tape/tape_send_buf.o
rm app/tape/tape_init.o
rm app/tape/tape_input.o
rm app/tape/tape_receive_buf.o
rm app/tape/tape_receive.o
rm app/tape/tape_extract_buf.o
rm app/ui/ui_send_sample.o
rm app/ui/ui_receive_test.o
rm app/ui/ui_send_test.o
rm app/ui/ui_terminal_to_commodore.o
rm app/ui/ui_commodore_to_terminal.o
rm app/ui/ui.o
rm app/cbm/cbm_receive.o
rm app/cbm/cbm_send.o

# Resulting kernel files:

rm kernel.elf
rm kernel.img
rm kernel7.img
rm kernel.ihx
