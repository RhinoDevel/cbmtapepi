#!/bin/sh

# RhinoDevel, Marcel Timm, 2019aug12


# This script does the following:
# -------------------------------
#
# - Runs creation script to generate kernel file.
# - Configures serial device.
# - Sends kernel file to serial device via XMODEM.
# - Sends start-kernel command to serial device.

# Setup some constants:
# ---------------------

SERIAL_DEVICE=/dev/ttyUSB0 # Serial device to communicate with Raspi.

KERNEL_FILENAME=kernel.img # Name of kernel file to send to Raspi.

# Execute script commands:
# ------------------------

echo "*********************************"
echo "*** Running creation script.. ***"
echo "*********************************"
echo
#
./create.sh
#
echo

echo "***********************************"
echo "*** Configuring serial device.. ***"
echo "***********************************"
echo
#
sudo bash -c "stty -F ${SERIAL_DEVICE} 115200"

echo "******************************"
echo "*** Sending created file.. ***"
echo "******************************"
echo
#
sudo bash -c "sx ${KERNEL_FILENAME} < ${SERIAL_DEVICE} > ${SERIAL_DEVICE}"
#
echo

echo "*******************************"
echo "*** Sending start command.. ***"
echo "*******************************"
echo
#
sudo bash -c "echo -n g > ${SERIAL_DEVICE}"
