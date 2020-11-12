#!/bin/bash

echo 

i2cset -y 1 0x08 0x01 0x33 #set the slave address to 0x33
i2cset -y 1 0x08 0x00 0x81 #set the control register to use the new slave address and reset the device 
i2cset -y 1 0x33 0x00 0x03 #using the new slave address. Retain the use of the slave address and load EEPROM content to the local memory registers

if ( "$(i2cget -y 1 0x33 0x00)" == "0x33" )
	exit 0;
else
	exit 1;
fi