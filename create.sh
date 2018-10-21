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

# Get prograspi from:
# https://github.com/dwelch67/raspberrypi/tree/master/bootloader01

# Get bootloader for Raspberry Pi 2 from:
# https://github.com/dwelch67/raspberrypi/tree/master/boards/pi2/bootloader07
#
# OR
#
# Get bootloader for Raspberry Pi 1 from:
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
$MT_CC tape/tape_fill_buf.c -o tape/tape_fill_buf.o
$MT_CC tape/tape_transfer_buf.c -o tape/tape_transfer_buf.o

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
    tape/tape_fill_buf.o \
    tape/tape_transfer_buf.o \
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

#echo Sending Intel HEX file..
##
#../prograspi/prograspi kernel.ihx /dev/ttyUSB0

#echo Starting emulator..
##
#qemu-system-arm -m 256 -M raspi2 -kernel kernel.elf -serial stdio
