#!/bin/bash

# Marcel Timm, RhinoDevel, 2019aug23

# Setup some constants:
# ---------------------

SERIAL_DEVICE=/dev/ttyUSB0 # Serial device to communicate with Raspi.
BAUD_RATE=115200

# Execute script commands:
# ------------------------

echo "********************************"
echo "*** Retrieve file from Raspi ***"
echo "********************************"
echo "(2019 by Marcel Timm, RhinoDevel)"
echo

read -s -p "Press enter key (here) to start waiting for file, then trigger sending from Raspi."
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

echo "Done."