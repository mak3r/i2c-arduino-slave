/*
	Library for accessing Arduino in I2C slave mode.
	Assumes 256 registers similar to hardware I2C slave devices
	Created by: Mark Abrams, November 22, 2020
	github: @mak3r
	Licence: Apache2

  /*
  I2C Pinouts
  SDA -> A4
  SCL -> A5
*/

/*
0x00 - EEPROM Control Register
    - Controls the reading and writing to EEPROM via I2C
    - Default 0x02
    Mask values 
    - 0x01 Write the bits to the control register but do not
      - overwrite in-memory store even if the bit is set
    - 0x04 Load EEPROM to local memory
    - 0x08 Always read registers from EEPROM
        - if this bit is not set, values are read from local memory
    - 0x10 This bit is ignored on the incoming control value
      - This bit is stored and returned based on the EEPROM read value
      - 1 when reading from EEPROM
      - 0 when reading from in-memory
    - 0x20 Reset all registers including the control register to default values
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
	- This functionality is CURRENTLY UNIMPLEMENTED
 
 0x04-0xFF Program Control
    - use as needed based on I2C communication via the device


*/
#include "Arduino.h"
#include "Wire.h"
#include "EEPROM.h"
#include "I2CSlaveMode.h"

//Slave Address for the Communication
#define I2C_SLAVE_ADDRESS 0x08

//Register keys
#define CONTROL_REG       0x00 // Control how eeprom is used
#define I2C_ADDR_REG      0x01 // Value of the I2C slave if not the default
#define DEFAULT_VAL_REG   0x02 // Register holding the default value for program control registers
#define PC_OFFSET         0x03 // Register indicating the offset for program control start index
#define PC_START_REG      0x04 // Default program control start index

//Control Register Bit Masks
#define LOCAL_PRESERVE          0x01
#define I2C_SLAVE_ALT           0x02
#define LOAD_EEPROM_TO_LOCAL    0x04
#define READ_FROM_EEPROM        0x08
#define READ_LOCATION           0x10
#define EEPROM_RESET            0X20
#define LOAD_LOCAL_TO_EEPROM    0x40
#define DEVICE_RESET            0x80


//Default content when filling Program Control Registers
#define REGISTERS_DEFAULT_VAL 0x00

//Default bit mask of the control register
#define CONTROL_DEFAULT_VAL LOAD_EEPROM_TO_LOCAL

static int I2CSlaveMode::_reg = 0;
static byte I2CSlaveMode::_regbuffer[NUM_REGISTERS];
static bool I2CSlaveMode::_read_eeprom = false;
static bool I2CSlaveMode::_use_slave_alt = false;
static int I2CSlaveMode::_reset_pin = 12;
static bool I2CSlaveMode::_device_reset = false;

I2CSlaveMode::I2CSlaveMode()
{
  I2CSlaveMode(I2C_SLAVE_ADDRESS);
}

I2CSlaveMode::I2CSlaveMode(byte address)
{
  I2CSlaveMode(address, _reset_pin);
}

I2CSlaveMode::I2CSlaveMode(byte address, int pin)
{
  _reset_pin = pin;
  digitalWrite(_reset_pin, HIGH);
  pinMode(_reset_pin, OUTPUT);
  // Initialize Serial communication
  Serial.begin(9600);
  Serial.write("Starting I2CSlaveMode");
  // initialize control mode based on EEPROM 
  byte cr = EEPROM.read(CONTROL_REG);  
  controlUpdated(cr);

  // initialize i2c as slave
  byte i2c_slave_addr = address;
  if (_use_slave_alt) {
    i2c_slave_addr = EEPROM.read(I2C_ADDR_REG);
    if (i2c_slave_addr < 0x03 || i2c_slave_addr > 0x77) { 
      //I2C standard address range 0x03-0x77
      // Out of range
      // Revert to the default addres
      i2c_slave_addr = I2C_SLAVE_ADDRESS;
    }
  }
  Wire.begin(i2c_slave_addr);
  // define callbacks for i2c communication
  Wire.onReceive(receiveEvent);
  Wire.onRequest(sendEvent);
}

void I2CSlaveMode::resetIfRequested() 
{
  if (_device_reset)
    digitalWrite(_reset_pin, LOW);
}

const byte I2CSlaveMode::getRegister(byte address)
{
  return _regbuffer[address];
}

const byte* I2CSlaveMode::getRange(byte start, byte end)
{
  byte buffer[end-start];
  for (byte i = start; i++ ; i<end) {
    buffer[i] = _regbuffer[i];
  }
  return buffer;
}

const byte* I2CSlaveMode::getBuffer()
{
  return _regbuffer;
}

void* I2CSlaveMode::bufferChanged(int *ptr)
{

}

/*
  private methods
*/
static void I2CSlaveMode::controlUpdated(byte cr) {
  byte cntrl_reg_val = cr;
  byte control_mask = 0x01;
  byte perma_mask = B00001110; // only the respected bits should be stored
                               // AND this with the cntrl_reg_val
  // Serial.print("cntrl_reg_val: ");      
  // Serial.println(cntrl_reg_val, BIN);        

  _read_eeprom = false;
  _use_slave_alt = false;
  bool local_preserve = false;

  byte cur_control = control_mask & cntrl_reg_val;
  while (control_mask > 0) {
    // Serial.print("cur_control: ");      
    // Serial.println(cur_control, BIN);        
    switch (cur_control) {
      case 0x00:
      {
        break;
      }
      case LOCAL_PRESERVE:
      {
        // Serial.println("Case LOCAL_PRESERVE");
        local_preserve = true;
        break;
      }
      case I2C_SLAVE_ALT: 
      {
        // Serial.println("Case I2C_SLAVE_ALT");
        _use_slave_alt = true;
        break;
      }
      case LOAD_EEPROM_TO_LOCAL: 
      {
        // Serial.println("Case LOAD_EEPROM_TO_LOCAL");
        if (!local_preserve) {
          //copy EEPROM data to local registers
          for (int i = CONTROL_REG; i < NUM_REGISTERS; i++) {
            _regbuffer[i] = EEPROM.read(i);
          }
        }
        break;
      }
      case READ_FROM_EEPROM: 
      {
        // Serial.println("Case READ_FROM_EEPROM");        
        _read_eeprom = true;
        break;
      }
      case READ_LOCATION: 
      {
        // Serial.println("Case READ_LOCATION");        
        break;
      }
      case EEPROM_RESET: 
      {
        // Serial.println("Case EEPROM_RESET");
        //cntrl_reg_val = CONTROL_DEFAULT_VAL; //This must be set because the loop exit option will try again
        EEPROM.update(CONTROL_REG, CONTROL_DEFAULT_VAL & perma_mask);
        EEPROM.update(I2C_ADDR_REG, I2C_SLAVE_ADDRESS); 
        EEPROM.update(DEFAULT_VAL_REG, REGISTERS_DEFAULT_VAL);
        byte c = EEPROM.read(DEFAULT_VAL_REG);
        for (int i = PC_START_REG; i < NUM_REGISTERS; i++) {
          EEPROM.update(i, c);
        }
        break;
      }
      case LOAD_LOCAL_TO_EEPROM: 
      {
        // Serial.println("Case LOAD_LOCAL_TO_EEPROM");        
        //copy local registers to EEPROM
        for (int i = CONTROL_REG; i < NUM_REGISTERS; i++) {
          if (i != CONTROL_REG) 
            EEPROM.update(i, _regbuffer[i]);
          else
            EEPROM.update(i, cntrl_reg_val & perma_mask);
        }
        break;
      }
      case DEVICE_RESET: 
      {
        // Serial.println("Case DEVICE_RESET");
        _device_reset = true;
        break;
      }
    }
    control_mask <<= 1;
    cur_control = control_mask & cntrl_reg_val;
    //Serial.print("control_mask :");
    //Serial.println(control_mask, BIN);
  }
  // Store the incoming control register value
  _regbuffer[CONTROL_REG] = cntrl_reg_val & perma_mask;

}

static void I2CSlaveMode::receiveEvent(int len){
  if(len == 1){ // One Byte Data received -> Read Request Address
    _reg = Wire.read();
  } else {
    _reg = 0;
    byte rx = Wire.read();
    delayMicroseconds(20);
    while(Wire.available() > 0){
      rx %= sizeof(_regbuffer);
      _regbuffer[rx] = Wire.read();  //pull in the latest byte of data and process it
      if (rx == CONTROL_REG)
        controlUpdated(_regbuffer[rx]);
      rx++;
    }
  }
}

static byte I2CSlaveMode::readData(int p, bool from_eeprom) {
  byte c;
  if (from_eeprom) {
    // read from eeprom
    if (p == CONTROL_REG)
      //add the READ_LOCATION bit flag
      c = (EEPROM.read(p) | B00010000);   
    else
      c = (EEPROM.read(p));
  } else {
    c = _regbuffer[p];    //read from local buffer    
  }
  return c;
}

static void I2CSlaveMode::sendEvent(){
  int p = _reg % sizeof(_regbuffer); 
  byte c;
  delayMicroseconds(20);
  c = readData(p, _read_eeprom);
  Wire.write(c);
}
