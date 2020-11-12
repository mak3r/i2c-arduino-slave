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
    - 0x01 NOACTION 
		- don't make any changes based on the incoming mask bits
		- store the incoming mask according to the current read location
		- this bit will always be 0 in the saved register 
    - 0x02 Load slave address from register (control register + 0x01) - only on power recycle
    - 0x04 Load EEPROM to local memory
    - 0x08 Store all received values into EEPROM registers 
        - if this bit is not set, values are stored in local memory
    - 0x10 Always read registers from EEPROM
        - if this bit is not set, values are read from local memory
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
#define NOACTION                0x01
#define I2C_SLAVE_ALT           0x02
#define LOAD_EEPROM_TO_LOCAL    0x04
#define WRITE_TO_EEPROM         0x08
#define READ_FROM_EEPROM        0x10
#define EEPROM_RESET            0X20
#define LOAD_EEPROM_FROM_LOCAL  0x40
#define DEVICE_RESET            0x80


//Default content when filling Program Control Registers
#define REGISTERS_DEFAULT_VAL 0x00

//Default bit mask of the control register
#define CONTROL_DEFAULT_VAL LOAD_EEPROM_TO_LOCAL

static const int NUM_REGISTERS=256;
int reg = 0;
byte regbuffer[NUM_REGISTERS];
bool read_eeprom = false;
bool write_eeprom = false;
bool use_slave_alt = false;

//Reset function
void(* resetFunc) (void) = 0;

//Code Initialization
void setup() {
  // Initialize Serial communication
  Serial.begin(9600);
  Serial.println("initializing ...");

  // initialize EEPROM control mode
  byte cr = EEPROM.read(CONTROL_REG);  
  Serial.print("Updating control .. ");
  Serial.println(cr, BIN);
  controlUpdated(cr);

  Serial.println("Setting slave address");
  Serial.print("Alt address: ");
  Serial.println(EEPROM.read(I2C_ADDR_REG), HEX);
  // initialize i2c as slave
  byte i2c_slave_addr = I2C_SLAVE_ADDRESS;
  if (use_slave_alt) {
    Serial.println("using slave alt address");
    if (i2c_slave_addr >= 0x03 && i2c_slave_addr <= 0x77) { //I2C standard address range 0x03-0x77
      Serial.println("slave alt address in range");
      i2c_slave_addr = EEPROM.read(I2C_ADDR_REG);
    }
  }
  Serial.println(i2c_slave_addr, HEX);
  Wire.begin(i2c_slave_addr);
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);  

}

void loop() {
  delay(100);
} // end loop

void controlUpdated(byte cr) {
  byte cntrl_reg_val = cr;
  byte control_mask = 0x01;
  byte perma_mask = B00011110; // only the respected bits should be stored
                               // AND this with the cntrl_reg_val
  Serial.print("cntrl_reg_val: ");      
  Serial.println(cntrl_reg_val, BIN);        

  //If the read or write bits are set, these will go true accordingly
  write_eeprom = false;
  read_eeprom = false;
  use_slave_alt = false;

  byte cur_control = control_mask & cntrl_reg_val;
  while (control_mask > 0) {
    Serial.print("cur_control: ");      
    Serial.println(cur_control, BIN);        
    switch (cur_control) {
      case 0x00:
      {
        Serial.println("Case 0");        
        break;
      }
      case NOACTION:
      {
        //Register is properly stored on exit
        // Ignore all other operations
        control_mask = 0x00; //force exit by tripping the while condition
        break;
      }
      case I2C_SLAVE_ALT: 
      {
        Serial.println("Case I2C_SLAVE_ALT");    
        use_slave_alt = true;    
        break;
      }
      case LOAD_EEPROM_TO_LOCAL: 
      {
        Serial.println("Case LOAD_EEPROM_TO_LOCAL");        
        //copy EEPROM data to local registers
        for (int i = 0; i < NUM_REGISTERS; i++) {
          regbuffer[i] = EEPROM.read(i);
        }
        break;
      }
      case WRITE_TO_EEPROM: 
      {
        Serial.println("Case WRITE_TO_EEPROM");        
        write_eeprom = true;
        break;
      }
      case READ_FROM_EEPROM: 
      {
        Serial.println("Case READ_FROM_EEPROM");        
        read_eeprom = true;
        break;
      }
      case EEPROM_RESET: 
      {
        Serial.println("Case EEPROM_RESET");
        cntrl_reg_val = CONTROL_DEFAULT_VAL; //This must be set because the loop exit option will try again
        safeWriteEEPROM(CONTROL_REG, cntrl_reg_val & perma_mask);
        safeWriteEEPROM(I2C_ADDR_REG, I2C_SLAVE_ADDRESS); 
        safeWriteEEPROM(DEFAULT_VAL_REG, REGISTERS_DEFAULT_VAL);
        byte c = EEPROM.read(DEFAULT_VAL_REG);
        for (int i = PC_START_REG; i < NUM_REGISTERS; i++) {
          safeWriteEEPROM(i, c);
        }
        break;
      }
      case LOAD_EEPROM_FROM_LOCAL: 
      {
        Serial.println("Case LOAD_EEPROM_FROM_LOCAL");        
        //copy local registers to EEPROM
        for (int i = 0x00; i < NUM_REGISTERS; i++) {
          safeWriteEEPROM(i, regbuffer[i]);
        }
        break;
      }
      case DEVICE_RESET: 
      {
        Serial.println("Case DEVICE_RESET");        
        //resetFunc();
        break;
      }
    }
    control_mask <<= 1;
    cur_control = control_mask & cntrl_reg_val;
    Serial.print("control_mask :");
    Serial.println(control_mask, BIN);
  }
  //store the respected bits of the control register value
  Serial.print("control_updated :");
  Serial.println(cntrl_reg_val & perma_mask, BIN);
  storeData(CONTROL_REG, cntrl_reg_val & perma_mask);

}

void receiveData(int len){
  if(len == 1){ // One Byte Data received -> Read Request Address
    reg = Wire.read();
  } else {
    reg = 0;
    byte rx = Wire.read();
    delayMicroseconds(20);
    while(Wire.available() > 0){
      rx %= sizeof(regbuffer);
      byte data_in = Wire.read();  //pull in the latest byte of data and process it
      if (rx == CONTROL_REG) {
        controlUpdated(data_in); //Control register data is a special case
      } else {
        storeData(rx, data_in);
      }
      rx++;
    }
  }
}

void storeData(byte reg, byte content) {
  if ( write_eeprom && (reg < EEPROM.length()) ) //NOTE: The EEPROM size varies depending on the arduino board
    safeWriteEEPROM(reg, content); //Assign the byte to eeprom
  else
    regbuffer[reg] = content; //Assign the byte to the in-memory array
}

void safeWriteEEPROM(byte reg, byte content) {
  //Only write to EEPROM if the value is not the same as the existing value
  Serial.print("current reg: 0x" );
  Serial.println(EEPROM.read(reg), HEX );
  Serial.print("content: 0x" );
  Serial.println(content, HEX );
  if ( EEPROM.read(reg) != content ) {
    Serial.println(" ********* writing eeprom ********" );
    EEPROM.write(reg, content);
  }
}

void sendData(){
  int p = reg % sizeof(regbuffer); 
  byte c;
  delayMicroseconds(20);
  if (read_eeprom) {
    c = EEPROM.read(p);   // read from eeprom
  } else {
    c = regbuffer[p];    //read from local buffer    
  }
  Wire.write(c);
}

//End of the program
