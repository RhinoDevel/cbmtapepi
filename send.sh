#!/bin/bash

# Marcel Timm, RhinoDevel, 2019aug23

# Setup some constants:
# ---------------------

SERIAL_DEVICE=/dev/ttyUSB0 # Serial device to communicate with Raspi.
BAUD_RATE=115200

# Execute script commands:
# ------------------------

echo "**************************************************************"
echo "*** CBM Tape Pi - Send file via Raspi to Commodore machine ***"
echo "**************************************************************"
echo "(2019 by Marcel Timm, RhinoDevel)"
echo

echo "- Make sure that Raspi is in LOAD move (LED off)."
echo

read -s -p "Enter LOAD and press return on Commodore, then press enter key (here) to send."
echo
echo

echo "Configuring serial device \"${SERIAL_DEVICE}\".."
echo
#
sudo bash -c "stty -F ${SERIAL_DEVICE} ${BAUD_RATE}"

echo "Sending file \"${1}\".."
echo
#
sudo bash -c "sx --ymodem '${1}' < ${SERIAL_DEVICE} > ${SERIAL_DEVICE}"
echo

echo "Raspberry Pi is now forwarding file to Commodore machine. See Commodore screen."
