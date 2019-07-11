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

#   -DNDEBUG \
export OPTIONS_GCC_C="$OPTIONS_GCC_ALL \
    -DNDEBUG \
    -O2 \
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
$MT_CC baregpio/baregpio.c -o baregpio/baregpio.o
$MT_CC mem/mem.c -o mem/mem.o
$MT_CC armtimer/armtimer.c -o armtimer/armtimer.o
$MT_CC busywait/busywait.c -o busywait/busywait.o
$MT_CC miniuart/miniuart.c -o miniuart/miniuart.o
$MT_CC pl011uart/pl011uart.c -o pl011uart/pl011uart.o
$MT_CC tape/tape_fill_buf.c -o tape/tape_fill_buf.o
$MT_CC tape/tape_send_buf.c -o tape/tape_send_buf.o
$MT_CC tape/tape_sample.c -o tape/tape_sample.o
$MT_CC tape/tape_send.c -o tape/tape_send.o
$MT_CC tape/tape_init.c -o tape/tape_init.o
$MT_CC ui/ui_send_sample.c -o ui/ui_send_sample.o
$MT_CC ui/ui_receive_test.c -o ui/ui_receive_test.o
$MT_CC ui/ui_terminal_to_commodore.c -o ui/ui_terminal_to_commodore.o
$MT_CC ui/ui.c -o ui/ui.o
$MT_CC console/console.c -o console/console.o
$MT_CC str/str.c -o str/str.o
$MT_CC calc/calc.c -o calc/calc.o
$MT_CC alloc/alloc.c -o alloc/alloc.o
$MT_CC alloc/alloc_mem.c -o alloc/alloc_mem.o
$MT_CC alloc/nodemem.c -o alloc/nodemem.o
$MT_CC watchdog/watchdog.c -o watchdog/watchdog.o
$MT_CC xmodem/xmodem.c -o xmodem/xmodem.o
$MT_CC ymodem/ymodem.c -o ymodem/ymodem.o

echo Linking..

# Link:
#
arm-none-eabi-ld \
    -T memmap.ld \
    boot.o \
    kernel_main.o \
    baregpio/baregpio.o \
    mem/mem.o \
    armtimer/armtimer.o \
    busywait/busywait.o \
    miniuart/miniuart.o \
    pl011uart/pl011uart.o \
    tape/tape_fill_buf.o \
    tape/tape_send_buf.o \
    tape/tape_sample.o \
    tape/tape_send.o \
    tape/tape_init.o \
    ui/ui_send_sample.o \
    ui/ui_receive_test.o \
    ui/ui_terminal_to_commodore.o \
    ui/ui.o \
    console/console.o \
    str/str.o \
    calc/calc.o \
    alloc/alloc.o \
    alloc/alloc_mem.o \
    alloc/nodemem.o \
    watchdog/watchdog.o \
    xmodem/xmodem.o \
    ymodem/ymodem.o \
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
