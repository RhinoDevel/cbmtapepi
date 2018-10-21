#!/bin/sh

export CARDPATH=/run/media/marc/BOOT/

set -e

echo Copying..
cp kernel7.img $CARDPATH
cp kernel.img $CARDPATH

echo Unmounting..
umount $CARDPATH

echo Syncing..
sync
