#!/bin/bash

source ./i2c-assistant.sh

DEVICE_ADDRESS="0x08"

detect $DEVICE_ADDRESS # make sure the slave is at the expected address
if [ "$?" = "0" ]; then
  # Reset EEPROM and restart the device to run this test
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0xA0
  sleep 2 # give a moment for restart
  verify "$DEVICE_ADDRESS" "0x00" "0x04"

  # All registers in EEPROM should be value 0x00
  # Set some registers in-memory 
  i2cset -y 1 "$DEVICE_ADDRESS" 0x16 0xF2
  verify "$DEVICE_ADDRESS" "0x16" "0xF2"

  # Now set the read from EEPROM flag 
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x08
  verify "$DEVICE_ADDRESS" "0x00" "0x14"

  # and check the same register
  verify "$DEVICE_ADDRESS" "0x16" "0x00"

  # cleanup
  # reset the device is all we need to do since
  # eeprom hasn't changed since we reset it.
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x80
else 
	echo "Did not find expected device at address "$DEVICE_ADDRESS"." 1>&2
	exit 1
fi
