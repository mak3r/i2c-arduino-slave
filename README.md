# i2c-arduino-slave
An I2C slave project built on wire.h. Designed to easily drop into a variety of projects which can then be managed by an I2C master.

This project uses an in memory array as the I2C registers. The in-memory array solution is obviously volatile and unsustainable across device restarts. To mitigate this, it is possible to write the contents of the in-memory array to the device EEPROM. Of course EEPROM on Arduino devices is said to have a lifetime of about 100K writes so it is not ideal as the permanent read/write registers for many applications. The following section detailing the *I2C Slave Configuration Registers* provides capabilities to change the mode to use EEPROM storage as the default, or to read from EEPROM if one were to preset the instructions necessary for a variety of applications. Please read that section for more details and look a the examples section for some ideas about how it might be used in practice.

## I2C Slave Configuration Registers
```
 0x00 - EEPROM Control Register
    - Controls the reading and writing to EEPROM via I2C
    - Default 0x0A
    Mask values 
    - 0x01 Store all received values in EEPROM registers 
        - if this bit is not set, values are stored in local memory
    - 0x02 Always read registers from EEPROM
        - if this bit is not set, values are read from local memory
    - 0x04 Reset all registers except the control register to default values 
    - 0x08 Load EEPROM to local memory
    - 0x10 Load local memory into EEPROM - including control register 0x00
    - 0x20 Reset all registers including the control register to defaults 
        - If this is set than all other bits in this register will also be reset
        - regardless of their current value
    - 0x40 Load slave address from register 0x01 - only on power recycle
    - 0x80 UNUSED

 0x01 - Slave address
    - The slave address is set to 0x08 programmatically
    - If an alternate slave address is desired, it can be set in this register

 0x02 - Default value for program control registers
    - 0x00
     
 0x03-0xFF Program Control
    - use as needed based on I2C communication via the device
```
## Example wiring between RPi (master) and Arduino (slave)

## Example Controls using a linux master running i2c-tools

## Known challenges with I2C
* Master / Slave voltages matter
* In order for devices with RTC to do I2C, they must use a software methodology called *clock pulse stretching*
    * sometimes this results in cycle delays an the appearance of lost bits
    * this seems to be able to be mitigaged in code by adding a slight delay at certain points
        `delayMicroseconds(20);` 



