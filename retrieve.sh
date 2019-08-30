#!/bin/bash

# Marcel Timm, RhinoDevel, 2019aug23

# Setup some constants:
# ---------------------

SERIAL_DEVICE=/dev/ttyUSB0 # Serial device to communicate with Raspi.
BAUD_RATE=115200

# Execute script commands:
# ------------------------

echo "********************************************************************"
echo "*** CBM Tape Pi - Retrieve file from Commodore machine via Raspi ***"
echo "********************************************************************"
echo "(2019 by Marcel Timm, RhinoDevel)"
echo

echo "- Make sure that Raspi is in SAVE move (LED on)."
echo

read -s -p "Press enter key (here) to start waiting for file, then enter SAVE and press return on Commodore."
echo
echo

echo "Configuring serial device \"${SERIAL_DEVICE}\".."
echo
#
sudo bash -c "stty -F ${SERIAL_DEVICE} ${BAUD_RATE}"

echo "Starting to wait for file to be retrieved.."
echo
#
sudo bash -c "rx --ymodem < ${SERIAL_DEVICE} > ${SERIAL_DEVICE}"
