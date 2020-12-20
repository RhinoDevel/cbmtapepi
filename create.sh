#!/bin/sh

# Marcel Timm, RhinoDevel, 2019aug22

# Create a cross compiler as described at:
# http://wiki.osdev.org/GCC_Cross-Compiler
#
# E.g. like this:
# ---------------
#
# wget https://mirror.checkdomain.de/gnu/binutils/binutils-2.31.1.tar.bz2
# wget https://ftp.gnu.org/gnu/gcc/gcc-8.2.0/gcc-8.2.0.tar.xz
#
#
#export PREFIX="$HOME/Documents/pios/cross/result"
#export TARGET=arm-none-eabi
#export PATH="$PREFIX/bin:$PATH"
#
#cd $HOME/Documents/pios/cross
#tar xvjf binutils-2.31.1.tar.bz2
#mkdir build-binutils
#cd build-binutils
#../binutils-2.31.1/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
#make
#make install
#
#cd $HOME/Documents/pios/cross
#
#tar xvf gcc-8.2.0.tar.xz
#mkdir build-gcc
#cd build-gcc
#../gcc-8.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
#make all-gcc
#make all-target-libgcc
#make install-gcc
#make install-target-libgcc

# Get bootcode.bin, start.elf and fixup.dat (necessary?) from:
# https://github.com/raspberrypi/firmware/tree/master/boot

# Get (Intel HEX) bootloader for Raspberry Pi 2 from:
# https://github.com/dwelch67/raspberrypi/tree/master/boards/pi2/bootloader07
#
# OR
#
# Get (Intel HEX) bootloader for Raspberry Pi 1 from:
# https://github.com/dwelch67/raspberrypi/tree/master/boards/pi1/bootloader07

#set -x

echo Clearing..

./clear.sh

set -e

export PREFIX="$HOME/Documents/pios/cross/result"
export PATH="$PREFIX/bin:$PATH"

# WORKS:
#
# raspi1 / BCM2835 / ARM1176JZF-S:
#
export MTUNE=arm1176jzf-s
export MFPU=vfp
export MARCH=armv6zk
#
# # WORKS:
# #
# # raspi2 / BCM2836 / Cortex-A7:
# #
# export MTUNE=cortex-a7
# export MFPU=neon-vfpv4
# export MARCH=armv7-a

export OPTIONS_GCC_ALL=" \
    -mtune=$MTUNE \
    -mfpu=$MFPU \
    -march=$MARCH \
    -mfloat-abi=hard \
    -fpic \
    -ffreestanding"

#    -DNDEBUG \

export OPTIONS_GCC_C="$OPTIONS_GCC_ALL \
    -O2 \
    -std=gnu99 \
    -Wall \
    -Werror \
    -Wextra"

export MT_CC="arm-none-eabi-gcc $OPTIONS_GCC_C -c"

echo Assembling..

# Assemble:
#
arm-none-eabi-gcc $OPTIONS_GCC_ALL -c app/boot.S -o app/boot.o

echo Compiling..

# Compile:

# Hardware-independent library:
#
$MT_CC lib/mem/mem.c -o lib/mem/mem.o
$MT_CC lib/console/console.c -o lib/console/console.o
$MT_CC lib/str/str.c -o lib/str/str.o
$MT_CC lib/calc/calc.c -o lib/calc/calc.o
$MT_CC lib/alloc/alloc.c -o lib/alloc/alloc.o
$MT_CC lib/alloc/alloc_mem.c -o lib/alloc/alloc_mem.o
$MT_CC lib/alloc/nodemem.c -o lib/alloc/nodemem.o
$MT_CC lib/basic/basic.c -o lib/basic/basic.o
$MT_CC lib/video/video.c -o lib/video/video.o
$MT_CC lib/xmodem/xmodem.c -o lib/xmodem/xmodem.o
$MT_CC lib/ymodem/ymodem.c -o lib/ymodem/ymodem.o
$MT_CC lib/petasc/petasc.c -o lib/petasc/petasc.o
$MT_CC lib/sort/sort.c -o lib/sort/sort.o
$MT_CC lib/dir/dir.c -o lib/dir/dir.o
$MT_CC lib/filesys/filesys.c -o lib/filesys/filesys.o
$MT_CC lib/cfg/cfg.c -o lib/cfg/cfg.o
$MT_CC lib/ff14/source/diskio.c -o lib/ff14/source/diskio.o
$MT_CC lib/ff14/source/ff.c -o lib/ff14/source/ff.o

# Hardware-dependent drivers (may use library compiled above):
#
$MT_CC hardware/gpio/gpio.c -o hardware/gpio/gpio.o
$MT_CC hardware/mailbox/mailbox.c -o hardware/mailbox/mailbox.o
$MT_CC hardware/armtimer/armtimer.c -o hardware/armtimer/armtimer.o
$MT_CC hardware/systimer/systimer.c -o hardware/systimer/systimer.o
$MT_CC hardware/busywait/busywait.c -o hardware/busywait/busywait.o
$MT_CC hardware/miniuart/miniuart.c -o hardware/miniuart/miniuart.o
$MT_CC hardware/pl011uart/pl011uart.c -o hardware/pl011uart/pl011uart.o
$MT_CC hardware/framebuffer/framebuffer.c -o hardware/framebuffer/framebuffer.o
$MT_CC hardware/watchdog/watchdog.c -o hardware/watchdog/watchdog.o
$MT_CC hardware/irqcontroller/irqcontroller.c -o hardware/irqcontroller/irqcontroller.o
$MT_CC hardware/sdcard/sdcard.c -o hardware/sdcard/sdcard.o

# Application-specific files (may use library and driver compiled above):
#
$MT_CC app/statetoggle/statetoggle.c -o app/statetoggle/statetoggle.o
$MT_CC app/tape/tape_fill_buf.c -o app/tape/tape_fill_buf.o
$MT_CC app/tape/tape_send_buf.c -o app/tape/tape_send_buf.o
$MT_CC app/tape/tape_sample.c -o app/tape/tape_sample.o
$MT_CC app/tape/tape_send.c -o app/tape/tape_send.o
$MT_CC app/tape/tape_init.c -o app/tape/tape_init.o
$MT_CC app/tape/tape_input.c -o app/tape/tape_input.o
$MT_CC app/tape/tape_receive_buf.c -o app/tape/tape_receive_buf.o
$MT_CC app/tape/tape_receive.c -o app/tape/tape_receive.o
$MT_CC app/tape/tape_extract_buf.c -o app/tape/tape_extract_buf.o
$MT_CC app/ui/ui_send_sample.c -o app/ui/ui_send_sample.o
$MT_CC app/ui/ui_receive_test.c -o app/ui/ui_receive_test.o
$MT_CC app/ui/ui_send_test.c -o app/ui/ui_send_test.o
$MT_CC app/ui/ui_terminal_to_commodore.c -o app/ui/ui_terminal_to_commodore.o
$MT_CC app/ui/ui_commodore_to_terminal.c -o app/ui/ui_commodore_to_terminal.o
$MT_CC app/ui/ui.c -o app/ui/ui.o
$MT_CC app/cbm/cbm_receive.c -o app/cbm/cbm_receive.o
$MT_CC app/cbm/cbm_send.c -o app/cbm/cbm_send.o
$MT_CC app/cmd/cmd.c -o app/cmd/cmd.o
$MT_CC app/petload/petload.c -o app/petload/petload.o
$MT_CC app/kernel_main.c -o app/kernel_main.o

echo Linking..

# Link:
#
arm-none-eabi-ld \
    -T app/memmap.ld \
    \
    app/boot.o \
    app/kernel_main.o \
    app/statetoggle/statetoggle.o \
    app/tape/tape_fill_buf.o \
    app/tape/tape_send_buf.o \
    app/tape/tape_sample.o \
    app/tape/tape_send.o \
    app/tape/tape_init.o \
    app/tape/tape_input.o \
    app/tape/tape_receive_buf.o \
    app/tape/tape_receive.o \
    app/tape/tape_extract_buf.o \
    app/ui/ui_send_sample.o \
    app/ui/ui_receive_test.o \
    app/ui/ui_send_test.o \
    app/ui/ui_terminal_to_commodore.o \
    app/ui/ui_commodore_to_terminal.o \
    app/ui/ui.o \
    app/cbm/cbm_receive.o \
    app/cbm/cbm_send.o \
    app/cmd/cmd.o \
    app/petload/petload.o \
    \
    lib/mem/mem.o \
    lib/console/console.o \
    lib/str/str.o \
    lib/calc/calc.o \
    lib/alloc/alloc.o \
    lib/alloc/alloc_mem.o \
    lib/alloc/nodemem.o \
    lib/basic/basic.o \
    lib/video/video.o \
    lib/xmodem/xmodem.o \
    lib/ymodem/ymodem.o \
    lib/petasc/petasc.o \
    lib/sort/sort.o \
    lib/dir/dir.o \
    lib/filesys/filesys.o \
    lib/cfg/cfg.o \
    lib/ff14/source/diskio.o \
    lib/ff14/source/ff.o \
    \
    hardware/gpio/gpio.o \
    hardware/mailbox/mailbox.o \
    hardware/armtimer/armtimer.o \
    hardware/systimer/systimer.o \
    hardware/busywait/busywait.o \
    hardware/miniuart/miniuart.o \
    hardware/pl011uart/pl011uart.o \
    hardware/watchdog/watchdog.o \
    hardware/framebuffer/framebuffer.o \
    hardware/irqcontroller/irqcontroller.o \
    hardware/sdcard/sdcard.o \
    \
    -o kernel.elf

echo Extracting..

arm-none-eabi-objcopy kernel.elf -O binary kernel.img

echo Copying..

cp kernel.img kernel7.img # Append "7" for raspi2 [(still) necessary?].

echo Create Intel HEX output..

arm-none-eabi-objcopy -O ihex kernel.elf kernel.ihx

# QEMU will work (with a more or less reliable serial console..),
# if you use PL011UART without calling pl011uart_init():
#
# echo Starting emulator..
# #
# qemu-system-arm -m 256 -M raspi2 -kernel kernel.elf -serial stdio
