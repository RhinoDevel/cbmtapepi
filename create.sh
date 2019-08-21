#!/bin/sh

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

export OPTIONS_GCC_C="$OPTIONS_GCC_ALL \
    -O2 \
    -DNDEBUG \
    -std=gnu99 \
    -Wall \
    -Werror \
    -Wextra"

export MT_CC="arm-none-eabi-gcc $OPTIONS_GCC_C -c"

echo Assembling..

# Assemble:
#
arm-none-eabi-gcc \
    $OPTIONS_GCC_ALL \
    -c boot.S \
    -o boot.o

echo Compiling..

# Compile:

$MT_CC kernel_main.c -o kernel_main.o
$MT_CC hardware/baregpio/baregpio.c -o hardware/baregpio/baregpio.o
$MT_CC mailbox/mailbox.c -o mailbox/mailbox.o
$MT_CC lib/mem/mem.c -o lib/mem/mem.o
$MT_CC hardware/armtimer/armtimer.c -o hardware/armtimer/armtimer.o
$MT_CC hardware/miniuart/miniuart.c -o hardware/miniuart/miniuart.o
$MT_CC hardware/pl011uart/pl011uart.c -o hardware/pl011uart/pl011uart.o
$MT_CC statetoggle/statetoggle.c -o statetoggle/statetoggle.o
$MT_CC tape/tape_fill_buf.c -o tape/tape_fill_buf.o
$MT_CC tape/tape_send_buf.c -o tape/tape_send_buf.o
$MT_CC tape/tape_sample.c -o tape/tape_sample.o
$MT_CC tape/tape_send.c -o tape/tape_send.o
$MT_CC tape/tape_init.c -o tape/tape_init.o
$MT_CC tape/tape_receive_buf.c -o tape/tape_receive_buf.o
$MT_CC tape/tape_receive.c -o tape/tape_receive.o
$MT_CC tape/tape_extract_buf.c -o tape/tape_extract_buf.o
$MT_CC ui/ui_send_sample.c -o ui/ui_send_sample.o
$MT_CC ui/ui_receive_test.c -o ui/ui_receive_test.o
$MT_CC ui/ui_send_test.c -o ui/ui_send_test.o
$MT_CC ui/ui_terminal_to_commodore.c -o ui/ui_terminal_to_commodore.o
$MT_CC ui/ui_commodore_to_terminal.c -o ui/ui_commodore_to_terminal.o
$MT_CC ui/ui.c -o ui/ui.o
$MT_CC lib/console/console.c -o lib/console/console.o
$MT_CC lib/str/str.c -o lib/str/str.o
$MT_CC lib/calc/calc.c -o lib/calc/calc.o
$MT_CC lib/alloc/alloc.c -o lib/alloc/alloc.o
$MT_CC lib/alloc/alloc_mem.c -o lib/alloc/alloc_mem.o
$MT_CC lib/alloc/nodemem.c -o lib/alloc/nodemem.o
$MT_CC video/video.c -o video/video.o
$MT_CC hardware/watchdog/watchdog.c -o hardware/watchdog/watchdog.o
$MT_CC lib/xmodem/xmodem.c -o lib/xmodem/xmodem.o
$MT_CC lib/ymodem/ymodem.c -o lib/ymodem/ymodem.o

echo Linking..

# Link:
#
arm-none-eabi-ld \
    -T memmap.ld \
    boot.o \
    kernel_main.o \
    hardware/baregpio/baregpio.o \
    mailbox/mailbox.o \
    lib/mem/mem.o \
    hardware/armtimer/armtimer.o \
    hardware/miniuart/miniuart.o \
    hardware/pl011uart/pl011uart.o \
    statetoggle/statetoggle.o \
    tape/tape_fill_buf.o \
    tape/tape_send_buf.o \
    tape/tape_sample.o \
    tape/tape_send.o \
    tape/tape_init.o \
    tape/tape_receive_buf.o \
    tape/tape_receive.o \
    tape/tape_extract_buf.o \
    ui/ui_send_sample.o \
    ui/ui_receive_test.o \
    ui/ui_send_test.o \
    ui/ui_terminal_to_commodore.o \
    ui/ui_commodore_to_terminal.o \
    ui/ui.o \
    lib/console/console.o \
    lib/str/str.o \
    lib/calc/calc.o \
    lib/alloc/alloc.o \
    lib/alloc/alloc_mem.o \
    lib/alloc/nodemem.o \
    hardware/watchdog/watchdog.o \
    video/video.o \
    lib/xmodem/xmodem.o \
    lib/ymodem/ymodem.o \
    -o kernel.elf

echo Extracting..

arm-none-eabi-objcopy \
    kernel.elf \
    -O binary \
    kernel.img

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
