SHELL = /bin/bash

target: local-preserve i2c-slave-alt eeprom-to-local read-from-eeprom read-location eeprom-reset local-to-eeprom device-reset

local-preserve:
	cd test; \
	./0x01_local-preserve.sh
	sleep 2

i2c-slave-alt:
	cd test; \
	./0x02_i2c-slave-alt.sh
	sleep 2

eeprom-to-local:
	cd test; \
	./0x04_eeprom-to-local.sh
	sleep 2

read-from-eeprom:
	cd test; \
	./0x08_read-from-eeprom.sh
	sleep 2

read-location:
	cd test; \
	./0x10_read-location.sh
	sleep 2

eeprom-reset:
	cd test; \
	./0x20_eeprom-reset.sh
	sleep 2

local-to-eeprom:
	cd test; \
	./0x40_local-to-eeprom.sh
	sleep 2

device-reset:
	cd test; \
	./0x80_device-reset.sh
	sleep 2
