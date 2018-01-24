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

echo Assembling..

# Assemble:
#
arm-none-eabi-gcc \
    $OPTIONS_GCC_ALL \
    -c boot.S \
    -o boot.o

echo Compiling..

# Compile:

arm-none-eabi-gcc \
    $OPTIONS_GCC_ALL \
    -O2 \
    -std=gnu99 \
    -Wall \
    -Werror \
    -Wextra \
\
    -c kernel_main.c \
    -o kernel_main.o

arm-none-eabi-gcc \
    $OPTIONS_GCC_ALL \
    -O2 \
    -std=gnu99 \
    -Wall \
    -Werror \
    -Wextra \
\
    -c baregpio/baregpio.c \
    -o baregpio/baregpio.o

echo Linking..

# Link:
#
arm-none-eabi-ld \
    -T memmap.ld \
    boot.o \
    kernel_main.o \
    baregpio/baregpio.o \
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
