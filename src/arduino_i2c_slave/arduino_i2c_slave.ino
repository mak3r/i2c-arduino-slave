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


 Author: Mark Abrams
 github: @mak3r
 
 */

#include <Wire.h>
#include <EEPROM.h>

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

static const int NUM_REGISTERS=256;
int reg = 0;
byte regbuffer[NUM_REGISTERS];
bool read_eeprom = false;
bool use_slave_alt = false;
int reset_pin = 12;
bool device_reset = false;


//Code Initialization
void setup() {
  digitalWrite(reset_pin, HIGH);
  pinMode(reset_pin, OUTPUT);
  // Initialize Serial communication
  Serial.begin(9600);
  //Serial.println("initializing ...");

  // initialize control mode based on EEPROM 
  byte cr = EEPROM.read(CONTROL_REG);  
  //Serial.print("Updating control .. ");
  //Serial.println(cr, BIN);
  controlUpdated(cr);

  //Serial.println("Setting slave address");
  //Serial.print("Alt address: ");
  //Serial.println(EEPROM.read(I2C_ADDR_REG), HEX);
  // initialize i2c as slave
  byte i2c_slave_addr = I2C_SLAVE_ADDRESS;
  if (use_slave_alt) {
    i2c_slave_addr = EEPROM.read(I2C_ADDR_REG);
    if (i2c_slave_addr < 0x03 || i2c_slave_addr > 0x77) { 
      //I2C standard address range 0x03-0x77
      // Out of range
      // Revert to the stored addres
      i2c_slave_addr = I2C_SLAVE_ADDRESS;
    }
  }
  //Serial.println(i2c_slave_addr, HEX);
  Wire.begin(i2c_slave_addr);
  // define callbacks for i2c communication
  Wire.onReceive(receiveEvent);
  Wire.onRequest(sendEvent);
}

void loop() {
  delay(100);
  if (device_reset)
    digitalWrite(reset_pin, LOW);

} // end loop

void controlUpdated(byte cr) {
  byte cntrl_reg_val = cr;
  byte control_mask = 0x01;
  byte perma_mask = B00001110; // only the respected bits should be stored
                               // AND this with the cntrl_reg_val
  // Serial.print("cntrl_reg_val: ");      
  // Serial.println(cntrl_reg_val, BIN);        

  read_eeprom = false;
  use_slave_alt = false;
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
        use_slave_alt = true;
        break;
      }
      case LOAD_EEPROM_TO_LOCAL: 
      {
        // Serial.println("Case LOAD_EEPROM_TO_LOCAL");
        if (!local_preserve) {
          //copy EEPROM data to local registers
          for (int i = CONTROL_REG; i < NUM_REGISTERS; i++) {
            regbuffer[i] = EEPROM.read(i);
          }
        }
        break;
      }
      case READ_FROM_EEPROM: 
      {
        // Serial.println("Case READ_FROM_EEPROM");        
        read_eeprom = true;
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
            EEPROM.update(i, regbuffer[i]);
          else
            EEPROM.update(i, cntrl_reg_val & perma_mask);
        }
        break;
      }
      case DEVICE_RESET: 
      {
        // Serial.println("Case DEVICE_RESET");
        device_reset = true;
        break;
      }
    }
    control_mask <<= 1;
    cur_control = control_mask & cntrl_reg_val;
    //Serial.print("control_mask :");
    //Serial.println(control_mask, BIN);
  }
  // Store the incoming control register value
  regbuffer[CONTROL_REG] = cntrl_reg_val & perma_mask;

}

void receiveEvent(int len){
  if(len == 1){ // One Byte Data received -> Read Request Address
    reg = Wire.read();
  } else {
    reg = 0;
    byte rx = Wire.read();
    delayMicroseconds(20);
    while(Wire.available() > 0){
      rx %= sizeof(regbuffer);
      regbuffer[rx] = Wire.read();  //pull in the latest byte of data and process it
      if (rx == CONTROL_REG)
        controlUpdated(regbuffer[rx]);
      rx++;
    }
  }
}

byte readData(int p, bool from_eeprom) {
  byte c;
  if (from_eeprom) {
    // read from eeprom
    if (p == CONTROL_REG)
      //add the READ_LOCATION bit flag
      c = (EEPROM.read(p) | B00010000);   
    else
      c = (EEPROM.read(p));
  } else {
    c = regbuffer[p];    //read from local buffer    
  }
  return c;
}

void sendEvent(){
  int p = reg % sizeof(regbuffer); 
  byte c;
  delayMicroseconds(20);
  c = readData(p, read_eeprom);
  Wire.write(c);
}

//End of the program
