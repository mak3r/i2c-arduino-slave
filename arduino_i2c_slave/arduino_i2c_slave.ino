/*
  I2C Pinouts
  SDA -> A4
  SCL -> A5
*/

/*
 0x00 - EEPROM Control Register
    - Controls the reading and writing to EEPROM via I2C
    - Default 0x80
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


 Author: Mark Abrams
 github: @mak3r
 
 */

//Import the library required
#include <Wire.h>
#include <EEPROM.h>

//Slave Address for the Communication
#define I2C_SLAVE_ADDRESS 0x08

//Register keys
#define CONTROL_REG       0x00
#define I2C_ADDR_REG      0x01
#define DEFAULT_VAL_REG   0x02 //Register holding the default value for program control registers
#define PC_START_REG      0x03

//Control Register Bit Masks
#define EEPROM_STORE            0x01
#define EEPROM_READ             0x02
#define EEPROM_RESET_PC         0x04
#define EEPROM_LOAD_TO_LOCAL    0x08
#define EEPROM_LOAD_FROM_LOCAL  0x10
#define EEPROM_RESET_ALL        0x20
#define EEPR0M_SLAVE_ALT        0x40
#define EEPROM_UNUSED           0x80 


//Default content when filling Program Control Registers
#define REGISTERS_DEFAULT_VAL 0x00

//Default bit mask of the control register
#define CONTROL_DEFAULT_VAL (EEPROM_READ & EEPROM_LOAD_TO_LOCAL)

static const int NUM_REGISTERS=256;
int reg = 0;
byte regbuffer[NUM_REGISTERS];
bool read_eeprom = false;
bool write_eeprom = false;

//Code Initialization
void setup() {
  // Initialize Serial communication
  Serial.begin(9600);

  // initialize EEPROM control mode
  byte cr = EEPROM.read(CONTROL_REG);  
  controlUpdated(cr);

  regbuffer[NUM_REGISTERS] = cr;
  
  // initialize i2c as slave
  byte i2c_slave_addr = I2C_SLAVE_ADDRESS;
  if (cr & EEPR0M_SLAVE_ALT) {
    i2c_slave_addr = EEPROM.read(I2C_ADDR_REG);
    if (i2c_slave_addr >= 0x03 && i2c_slave_addr <= 0x77)
      i2c_slave_addr = I2C_SLAVE_ADDRESS;
  }    
  Wire.begin(i2c_slave_addr);
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);  

}

void loop() {
  delay(100);
} // end loop

void resetControls() {
  EEPROM.write(CONTROL_REG, CONTROL_DEFAULT_VAL);
  EEPROM.write(I2C_ADDR_REG, I2C_SLAVE_ADDRESS); 
  EEPROM.write(DEFAULT_VAL_REG, REGISTERS_DEFAULT_VAL);
}

void resetProgramControl() {
  byte c = EEPROM.read(DEFAULT_VAL_REG);
  for (int i = PC_START_REG; i < NUM_REGISTERS; i++) {
    EEPROM.write(i, c);
  }
}

void localExchange(byte cr) {
  //If both bits are set then EEPROM is loaded to local first
  if ( cr & EEPROM_LOAD_TO_LOCAL ) {
    //copy EEPROM data to local registers
    for (int i = 0; i < NUM_REGISTERS; i++) {
      EEPROM.write(i, regbuffer[i]);
    }
  }
  if ( cr & EEPROM_LOAD_FROM_LOCAL ) {
    //copy local registers to EEPROM
    for (int i = 0; i < NUM_REGISTERS; i++) {
      regbuffer[i] = EEPROM.read(i);
    }
  }
}


void controlUpdated(byte cr) {
  EEPROM.write(CONTROL_REG, cr);
  if ( cr & EEPROM_RESET_ALL ) {
    resetControls();
    resetProgramControl();
  }
  //Now look at the remaining bits for possible immediate change
  if ( cr & EEPROM_RESET_PC )
    resetProgramControl();
  write_eeprom = ( cr & EEPROM_STORE ? true : false );    
  read_eeprom = ( cr & EEPROM_READ ? true : false );      
  if ( cr & EEPROM_RESET_PC )
    resetProgramControl();   

  //Now handle any EEPROM to / from exchange with the local store
  localExchange(cr);
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
      regbuffer[rx] = Wire.read();  //always assign register content to the local buffer
      if (rx == CONTROL_REG) {
        controlUpdated(regbuffer[rx]);
      }
      if (write_eeprom) {                   //NOTE: This value can change under controlUpdated()
        if (rx < EEPROM.length())           //NOTE: The EEPROM size varies depending on the arduino board
          EEPROM.write(rx, regbuffer[rx]);  // be sure not to index past the end of the EEPROM                                       
      }
      rx++;
    }
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
