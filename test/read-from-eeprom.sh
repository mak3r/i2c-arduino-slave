#!/bin/bash

i2cdetect -y 1 | grep 08
if [ "$?" = "0" ]; then
  i2cset -y 1 0x08 0x00 0x20
  i2cset -y 1 0x08 0x00 0x10
  if [ $(i2cget -y 1 0x08 0x00) != "0x04" ]; then 
	echo "Expected control register value to be 0x04" 1>&2
	exit 1;
  fi
  if [ "$(i2cget -y 1 0x08 0x01)" != "0x08" ]; then 
	echo "Expected slave alt value to be 0x08" 1>&2
	exit 1;
  fi
  i2cset -y 1 0x08 0x00 0x00
  i2cset -y 1 0x08 0x00 0x10
  if [ $(i2cget -y 1 0x08 0x00) != "0x00" ]; then 
	echo "Expected control register value to be 0x00" 1>&2
	exit 1;
  fi
  if [ "$(i2cget -y 1 0x08 0x01)" != "0x00" ]; then 
	echo "Expected slave alt value to be 0x00" 1>&2
	exit 1;
  fi
else 
	echo "Did not find expected device at addres 0x08." 1>&2
	exit 1
fi
