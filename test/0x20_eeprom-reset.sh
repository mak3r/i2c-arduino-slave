#!/bin/bash

source ./i2c-assistant.sh

DEVICE_ADDRESS="0x08"

function reset_eeprom {
  # Reset EEPROM 
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x20
  # in-memory control register is now 0x00
  verify "$DEVICE_ADDRESS" "0x00" "0x00"

  # setup READ_FROM_EEPROM 
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x08
  verify_control_reg_defaults "$DEVICE_ADDRESS"

  # All registers in EEPROM should be value 0x00
  # Let's check
  for i in {4..255}; do
    hexi=$(printf "%x" "$i")
    verify "$DEVICE_ADDRESS" "0x$hexi" "0x00"
  done
}

detect $DEVICE_ADDRESS # make sure the slave is at the expected address
if [ "$?" = "0" ]; then
  reset_eeprom

  # Now write some things to in-memory 
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x00
  register=(16 26 32 9B F7)
  regval=(10 20 30 40 50)
  for i in ${!register[@]}; do
    verify "$DEVICE_ADDRESS" "0x${register[$i]}" "0x00" # registers start at 0
    i2cset -y 1 "$DEVICE_ADDRESS" "0x${register[$i]}" "0x${regval[$i]}" # set new value
    verify "$DEVICE_ADDRESS" "0x${register[$i]}" "0x${regval[$i]}" # verify
  done

  # now write those things to eeprom
  # and preparet to read eeprom
  i2cset -y 1 "$DEVICE_ADDRESS" "0x00" "0x48"
  verify "$DEVICE_ADDRESS" "0x00" "0x18"
  for i in ${!register[@]}; do
    verify "$DEVICE_ADDRESS" "0x${register[$i]}" "0x${regval[$i]}" # verify
  done

  # Test EEPROM_RESET again
  reset_eeprom
  
  # cleanup
  # reset the device is all we need to do since
  # eeprom hasn't changed since we reset it.
  i2cset -y 1 "$DEVICE_ADDRESS" 0x00 0x80
else 
	echo "Did not find expected device at address "$DEVICE_ADDRESS"." 1>&2
	exit 1
fi
