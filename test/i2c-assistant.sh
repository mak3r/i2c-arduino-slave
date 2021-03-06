#!/bin/bash


function verify {
	# $1 is the i2c device addres
	# $2 is the register to read
	# $3 is the expected value
	shopt -s nocasematch
	outval=$(i2cget -y 1 $1 $2)
	if [[ "$outval" != "$3" ]]; then 
		echo "FAIL: Expected register $2 value to be $3" 1>&2
		echo "FAIL: Unexpected value: $outval" 1>&2
		return 1
	fi
	echo "OK: Register $2 value is $3 at device address $1"
	return 0
}

function detect {
	ADDRESS="$(echo $1 | cut -c3-4)"
	# $1 is the i2c bus slave address
	if i2cdetect -y 1 | grep -q "$ADDRESS"; then
		echo "i2c device discovered on $ADDRESS" 1>&2
		return 0
	fi
	echo "I2c device expected on $ADDRESS - NOT FOUND" 1>&2
	return 1
}

function verify_control_reg_defaults {
  DEVICE_ADDRESS="$1"
  # check the first 4 registers are as expected
  verify "$DEVICE_ADDRESS" "0x00" "0x14"
  verify "$DEVICE_ADDRESS" "0x01" "0x08"
  verify "$DEVICE_ADDRESS" "0x02" "0x00"
  verify "$DEVICE_ADDRESS" "0x03" "0x00"
}