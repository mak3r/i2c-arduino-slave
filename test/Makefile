.PHONY: i2c-test local-preserve i2c-slave-alt eeprom-to-local read-from-eeprom read-location eeprom-reset local-to-eeprom device-reset

SHELL = /bin/bash

i2c-test: local-preserve i2c-slave-alt eeprom-to-local read-from-eeprom read-location eeprom-reset local-to-eeprom device-reset

local-preserve:
	./0x01_local-preserve.sh
	sleep 2

i2c-slave-alt:
	./0x02_i2c-slave-alt.sh
	sleep 2

eeprom-to-local:
	./0x04_eeprom-to-local.sh
	sleep 2

read-from-eeprom:
	./0x08_read-from-eeprom.sh
	sleep 2

read-location:
	./0x10_read-location.sh
	sleep 2

eeprom-reset:
	./0x20_eeprom-reset.sh
	sleep 2

local-to-eeprom:
	./0x40_local-to-eeprom.sh
	sleep 2

device-reset:
	./0x80_device-reset.sh
	sleep 2
