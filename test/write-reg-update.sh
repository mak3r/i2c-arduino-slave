#!/bin/bash

i2cdetect -y 1 | grep 08
if [ "$?" != "0" ]; then
	# reset eeprom and then reset the device
	# sets the i2c slave address back to the default 0x08
	# sets the control mask to load eeprom to local
	# restarts the device
	i2cset -y 1 0x08 0x00 0xA0 #reset eeprom
fi

i2cdetect -y 1 | grep 08
if [ "$?" != "0" ]; then
	echo "Failed to reset to default slave as expected."
	exit 1;
fi

# The write value of the incoming control bits
# dictates whether the write is done to eeprom
# or to the local-memory register.
# initially we expect reads and writes to be from 
# the in-memory store and the in-memory store should have been 
# loaded from eeprom at restart
# expected values are
# register 0x00 - 0x04
# register 0x01 - 0x08
if [ $(i2cget -y 1 0x08 0x00) != "0x04" ]; then 
	echo "Expected control register value to be 0x04" 1>&2
	exit 1;
fi
if [ "$(i2cget -y 1 0x08 0x01)" != "0x08" ]; then 
	echo "Expected slave alt value to be 0x08" 1>&2
	exit 1;
fi

# OK, everything looks as expected so far. 
# Lets use the WRITE_REG_UPDATE flag and see 
# that it works as expected

# Write to a program register in local-memory then
# Change the control register to read from eeprom
# write update to eeprom
i2cset -y 1 0x08 0x42 0x17 # written to in-memory
i2cset -y 1 0x08 0x00 0x11 

# read the control register from eeprom
# it should match what was just written

# write update to in-memory
# read the control register from in-memory
# it should match what was just written

# test complete

i2cdetect -y 1 | grep 08
if [ "$?" = "0" ]; then
	i2cset -y 1 0x08 0x00 0x20 #reset eeprom 
	i2cset -y 1 0x08 0x00 0x84 #load eeprom to local then restart
	if [ $(i2cget -y 1 0x08 0x00) != "0x04" ]; then 
	echo "Expected control register value to be 0x04" 1>&2
	exit 1;
	fi
	if [ "$(i2cget -y 1 0x08 0x01)" != "0x08" ]; then 
		echo "Expected slave alt value to be 0x08" 1>&2
		exit 1;
	fi
	## Setting up a program
	#in local, set the slave address to 0x33
	i2cset -y 1 0x08 0x01 0x33 
	#set the local buffer control register to:
	# read from eeprom
	# and use the slave alt address
	i2cset -y 1 0x08 0x00 0x13
	# write local registers to eeprom
	# and reset the device 
	i2cset -y 1 0x08 0x00 0xC2 
else 
	echo "Did not find expected device at addres 0x08." 1>&2
	exit 1
fi

i2cdetect -y 1 | grep 33
if [ "$?" = "0" ]; then
	exit 0;
else
	echo "slave-alt test failed." 1>&2
	exit 1;
fi