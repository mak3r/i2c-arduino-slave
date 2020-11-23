/*
	Library for accessing Arduino in I2C slave mode.
	Assumes 256 registers similar to hardware I2C slave devices
	Created by: Mark Abrams, November 22, 2020
	github: @mak3r
	Licence: Apache2
*/
#ifndef I2CSlaveMode_h
#define I2CSlaveMode_h

#include "Arduino.h"


class I2CSlaveMode {
	
	public:
		/*
			Handle I2C interactions as an I2C slave on 
			the default bus address 
		*/
		I2CSlaveMode();
		/*
			Handle I2C interactions as an I2C slave on 
			the address provided.
			I2C addressing limits the acceptable range 
			to be between 0x03 and 0x77
		*/
		I2CSlaveMode(byte address);
		/*
			Handle I2C interactions as an I2C slave on 
			the address provided.
			I2C addressing limits the acceptable range 
			to be between 0x03 and 0x77
			Set the pin value connected to the reset pin if 
			reset functionality is desired. 
			The default if not set is
			to use pin 12
		*/
		I2CSlaveMode(byte address, int pin);
		/*
			Get a value from a register.
			Registers range between 0x00 and 0xFF
		*/
		const byte getRegister(byte address);
		/*
			Get the range of values from start to end
		*/
		const byte* getRange(byte start, byte end);
		/*
			Get the entire buffer of registers.
			This is the equivalent of using getRange(0x00,0xFF)
		*/
		const byte* getBuffer();
		/*
			The ptr argument should be a method that will be called 
			at the end of the operation
			which stores changes to the buffer of registers.
			Use this to get notified of changes to the registers.
		*/
		void* bufferChanged(int *ptr);
		/*
			This method should be in the loop().
			It will check to see if the device reset has been triggered.
			If so, it will use the pin specified in the contructor 
			to reset the device. The pin specified must be connected
			to the Arduino resetPin
		*/
		void resetIfRequested();
	private:
		static const int NUM_REGISTERS = 256;
		static int _reg;
		static byte _regbuffer[NUM_REGISTERS];
		static bool _read_eeprom;
		static bool _use_slave_alt;
		static int _reset_pin;
		static bool _device_reset;

		static void controlUpdated(byte cr);
		static void receiveEvent(int len);
		static byte readData(int p, bool from_eeprom);
		static void sendEvent();
};

#endif
