# i2c-arduino-slave
An I2C slave project built on wire.h. Designed to easily drop into a variety of projects which can then be managed by an I2C master.

This project uses an in memory array as the I2C registers. The in-memory array solution is obviously volatile and unsustainable across device restarts. To mitigate this, it is possible to write the contents of the in-memory array to the device EEPROM. Of course EEPROM on Arduino devices is said to have a lifetime of about 100K writes so it is not ideal as the permanent read/write registers for many applications. The following section detailing the *I2C Slave Configuration Registers* provides capabilities to change the mode to use EEPROM storage as the default, or to read from EEPROM if one were to preset the instructions necessary for a variety of applications. Please read that section for more details and look a the examples section for some ideas about how it might be used in practice.

## I2C Slave Configuration Registers
<<<<<<< HEAD
* This is the set of registers used to manipulate and manage the slave. 
* Whenever the Control Register is modified, the changes are made by reading the values of the least significant bit first.
* The lower 4 bits are always retained when updated.
* The upper 4 bits are acted on but never stored in the control register 
=======
* Registers 0x00, 0x01 and 0x02 are reserved for device management
* Registers 0x03 - 0xFF are available for program control

>>>>>>> d1f58f56aa36c00951af96c5e2e3f0024e99bfe8
```
0x00 - EEPROM Control Register
    - Controls the reading and writing to EEPROM via I2C
    - Default 0x02
    Mask values 
    - 0x01 Load slave address from register (control register + 0x01) - only on power recycle
    - 0x02 Load EEPROM to local memory
    - 0x04 Store all received values into EEPROM registers 
        - if this bit is not set, values are stored in local memory
    - 0x08 Always read registers from EEPROM
        - if this bit is not set, values are read from local memory
    - 0x10 Reset all registers except the control register to default values
		- this bit will always be 0 in the saved register 
    - 0x20 Reset control register to default value
		- this bit will always be 0 in the saved register 
    - 0x40 Load local memory into EEPROM - including control register 0x00 
		- this bit will always be 0 in the saved register
    - 0x80 Force device reset 
		- this bit will always be 0 in the saved register

 0x01 - Slave address
    - The slave address is set to 0x08 programmatically
    - If an alternate slave address is desired, it can be set in this register

 0x02 - Default value for program control registers
    - 0x00
     
 0x03 - Program control offset. 
	- This can be used to extend where the program is stored
	- This can only work if if the device has more EEPROM available. It's not magic
	- The default offset is this register+1
	- CURRENTLY UNIMPLEMENTED

 0x04-0xFF Program Control
    - use as needed based on I2C communication via the device
```
## Example wiring between RPi (master) and Arduino (slave)

## Example Controls using a linux master running i2c-tools
### Restart the device with a new slave address
Note that the device is on 0x08 by default so we use that address to send the configuration changes to restart the device on 0x33
```
i2cset -y 1 0x08 0x01 0x33 #set the slave address to 0x33
i2cset -y 1 0x08 0x00 0x81 #set the control register to use the new slave address and reset the device 
i2cset -y 1 0x33 0x00 0x03 #using the new slave address. Retain the use of the slave address and load EEPROM content to the local memory registers
```
### Feed a program into the local memory and then store it in EEPROM
This is useful to configure a program in memory and then store it for long term use to run from EEPROM
```
i2ctransfer -y 1 w17@0x08 0x42 0xff- #write 17 bytes of data in descending order starting at offset 0x42
i2cset -y 1 0x08 0x00 0xC8	# Set the system to read EEPROM registers on restart, 
							# Store the local memory in EEPROM and 
							# reset the device
```

## Known challenges with I2C
* Master / Slave voltages should be the same. 
	* If they are not, it is acceptable for the master to drive the slave as long as the master runs at a lower voltage
	* If they master has a higher voltage, a logic converter is required 
* In order for devices with RTC to do I2C, they must use a software methodology called *clock pulse stretching*
    * sometimes this results in cycle delays an the appearance of lost bits
    * this seems to be able to be mitigated in code by adding a slight delay at certain points
        `delayMicroseconds(20);` 



