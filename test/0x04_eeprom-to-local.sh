#!/bin/bash

source ./i2c-assistant.sh

DEVICE_ADDRESS="0x08"

detect $DEVICE_ADDRESS # make sure the slave is at the expected address
if [ "$?" = "0" ]; then
  # Reset EEPROM and restart the device to run this test
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0xA0
  sleep 2 # give a moment for restart
  verify "$DEVICE_ADDRESS" "0x00" "0x04"

  # All registers in EEPROM and in-memory should be value 0x00
  # So lets set a bunch of registers in-memory 
  register=(16 26 32 9B F7)
  regval=(10 20 30 40 50)
  for i in ${!register[@]}; do
    verify "$DEVICE_ADDRESS" "0x${register[$i]}" "0x00" # registers start at 0
    i2cset -y 1 "$DEVICE_ADDRESS" "0x${register[$i]}" "0x${regval[$i]}" # set new value
    verify "$DEVICE_ADDRESS" "0x${register[$i]}" "0x${regval[$i]}" # verify
  done

  # Setup in-memory to load EEPROM to local on restart
  # and read from eeprom
  # and restart the device
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0xC8
  verify "$DEVICE_ADDRESS" "0x00" "0x18" 
  sleep 2
  # check that the program registers are set 
  for i in ${!register[@]}; do
    verify "$DEVICE_ADDRESS" "0x${register[$i]}" "0x${regval[$i]}" # verify
  done

  # Now unset the read from EEPROM flag 
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x00
  verify "$DEVICE_ADDRESS" "0x00" "0x00" # note the READ_LOCATION bit is not set

  # check the same registers in-memory for value 0
  # this confirms the restart since it overwrote the program registers
  for i in ${!register[@]}; do
    verify "$DEVICE_ADDRESS" "0x${register[$i]}" "0x00" # registers start at 0
  done

  # cleanup
  # reset the device is all we need to do since
  # eeprom hasn't changed since we reset it.
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x80
else 
	echo "Did not find expected device at address "$DEVICE_ADDRESS"." 1>&2
	exit 1
fi
