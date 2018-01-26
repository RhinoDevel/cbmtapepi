#!/bin/sh

# Create a cross compiler as described at:
# http://wiki.osdev.org/GCC_Cross-Compiler

# Get bootcode.bin, start.elf and fixup.dat from:
# https://github.com/raspberrypi/firmware/tree/master/boot

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
$MT_CC tape/tape.c -o tape/tape.o

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
    tape/tape.o \
    -o kernel.elf

echo Extracting..

arm-none-eabi-objcopy \
    kernel.elf \
    -O binary \
    kernel.img

echo Copying..

cp kernel.img kernel7.img # Append "7" for raspi2.

#echo Starting emulator..
##
#qemu-system-arm -m 256 -M raspi2 -kernel kernel.elf -serial stdio
