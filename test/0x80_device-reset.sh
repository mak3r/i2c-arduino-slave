#!/bin/bash

#!/bin/bash

source ./i2c-assistant.sh

echo "Testing $0 .."

ORIG_ADDRESS="0x08"
ALT_ADDRESS="0x33"

detect $ORIG_ADDRESS # make sure the slave is at the expected address
if [ "$?" = "0" ]; then
  # Reset EEPROM and restart the device to run this test
  i2cset -y 1 "$ORIG_ADDRESS" 0x00 0xA0
  sleep 2 # give a moment for restart
  verify "$ORIG_ADDRESS" "0x00" "0x04"

  # prepare the control register to LOAD_EEPROM_TO_LOCAL
  # without taking action by keeping LOCAL_PRESERVE set
  # also set it up to use the slave alt address
  i2cset -y 1 "$ORIG_ADDRESS" 0x00 0x07
  verify "$ORIG_ADDRESS" "0x00" "0x06"

  # Set the alt device address register in-memory
  i2cset -y 1 "$ORIG_ADDRESS" 0x01 "$ALT_ADDRESS"
  verify "$ORIG_ADDRESS" "0x01" "$ALT_ADDRESS"

  # write it all to eeprom
  i2cset -y 1 "$ORIG_ADDRESS" 0x00 0x4F
  verify "$ORIG_ADDRESS" "0x00" "0x16" # reading from in-memory
  verify "$ORIG_ADDRESS" "0x01" "$ALT_ADDRESS"
    
  # reset the device
  i2cset -y 1 "$ORIG_ADDRESS" 0x00 0x80
  sleep 2

  # our device should be at a different location now
  detect "$ALT_ADDRESS"
  if [ "$?" = "1" ]; then  
    echo "$0 Test failed" 1>&2
    exit 1
  fi

  # Make sure the defaults are as expected
  verify_control_reg_defaults "$ALT_ADDRESS"

  # cleanup
  i2cset -y 1 "$ALT_ADDRESS" 0x00 0xA0
  sleep 4
  detect "$ORIG_ADDRESS"
else 
	echo "Did not find expected device at address "$ORIG_ADDRESS"." 1>&2
	exit 1
fi
