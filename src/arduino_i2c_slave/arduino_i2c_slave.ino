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
    - 0x01 Load slave address from register (control register + 0x01) - only on power recycle
    - 0x02 Load EEPROM to local memory
    - 0x04 Store all received values into EEPROM registers 
        - if this bit is not set, values are stored in local memory
    - 0x08 Always read registers from EEPROM
        - if this bit is not set, values are read from local memory
    - 0x10 Reset all registers except the control register to default values 
    - 0x20 Reset control register to default value 
    - 0x40 Load local memory into EEPROM - including control register 0x00
    - 0x80 Developer use - force device reset 

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


 Author: Mark Abrams
 github: @mak3r
 
 */

//Import the library required
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
#define EEPR0M_SLAVE_ALT        0x01
#define EEPROM_LOAD_TO_LOCAL    0x02
#define EEPROM_STORE            0x04
#define EEPROM_READ             0x08
#define EEPROM_RESET_PC         0x10
#define EEPROM_RESET_CNTRL_REG 0x20
#define EEPROM_LOAD_FROM_LOCAL  0x40
#define EEPROM_DEVICE_RESET     0x80 


//Default content when filling Program Control Registers
#define REGISTERS_DEFAULT_VAL 0x00

//Default bit mask of the control register
#define CONTROL_DEFAULT_VAL EEPROM_LOAD_TO_LOCAL

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
  byte control_reg = cr;
  byte control_mask = 0x01;
  byte perma_mask = 0x0F; // only the last 4 bits should be stored
                          // AND this with the control_reg
  Serial.print("control_reg: ");      
  Serial.println(control_reg, BIN);        

  //If the read or write bits are set, these will go true accordingly
  write_eeprom = false;
  read_eeprom = false;
  use_slave_alt = false;
  byte cur_control = control_mask & control_reg;
  while (control_mask > 0) {
    Serial.print("cur_control: ");      
    Serial.println(cur_control, BIN);        
    switch (cur_control) {
      case 0x00:
      {
        Serial.println("Case 0");        
        break;
      }
      case EEPR0M_SLAVE_ALT: 
      {
        Serial.println("Case EEPR0M_SLAVE_ALT");    
        use_slave_alt = true;    
        break;
      }
      case EEPROM_LOAD_TO_LOCAL: 
      {
        Serial.println("Case EEPROM_LOAD_TO_LOCAL");        
        //copy EEPROM data to local registers
        for (int i = 0; i < NUM_REGISTERS; i++) {
          regbuffer[i] = EEPROM.read(i);
        }
        break;
      }
      case EEPROM_STORE: 
      {
        Serial.println("Case EEPROM_STORE");        
        write_eeprom = true;
        break;
      }
      case EEPROM_READ: 
      {
        Serial.println("Case EEPROM_READ");        
        read_eeprom = true;
        break;
      }
      case EEPROM_RESET_PC: 
      {
        Serial.println("Case EEPROM_RESET_PC");
        /*
        safeWriteEEPROM(I2C_ADDR_REG, I2C_SLAVE_ADDRESS); 
        safeWriteEEPROM(DEFAULT_VAL_REG, REGISTERS_DEFAULT_VAL);
        byte c = EEPROM.read(DEFAULT_VAL_REG);
        for (int i = PC_START_REG; i < NUM_REGISTERS; i++) {
          safeWriteEEPROM(i, c);
        }
        */
        break;
      }
      case EEPROM_RESET_CNTRL_REG: 
      {
        Serial.println("Case EEPROM_RESET_CNTRL_REG");        
        control_reg = CONTROL_DEFAULT_VAL;
        break;
      }
      case EEPROM_LOAD_FROM_LOCAL: 
      {
        Serial.println("Case EEPROM_LOAD_FROM_LOCAL");        
        //copy local registers to EEPROM
        for (int i = 0; i < NUM_REGISTERS; i++) {
          safeWriteEEPROM(i, regbuffer[i]);
        }
        break;
      }
      case EEPROM_DEVICE_RESET: 
      {
        Serial.println("Case EEPROM_DEVICE_RESET");        
        //resetFunc();
        break;
      }
    }
    control_mask <<= 1;
    cur_control = control_mask & control_reg;
    Serial.print("control_mask :");
    Serial.println(control_mask, BIN);
  }
  //store the lower 4 bits of the control register value
    Serial.print("control_updated :");
    Serial.println(control_reg & perma_mask, BIN);
  safeWriteEEPROM(CONTROL_REG, control_reg & perma_mask);

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
      if (CONTROL_REG < rx && rx < EEPROM.length()) { //The control register is a special case write 
                                                      //NOTE: The EEPROM size varies depending on the arduino board
          safeWriteEEPROM(rx, regbuffer[rx]);         // be sure not to index past the end of the EEPROM                                       
      }
      rx++;
    }
  }
}

void safeWriteEEPROM(byte reg, byte content) {
  //Only write to EEPROM if the value is not the same as the existing value
  Serial.print("current reg: 0x" );
  Serial.println(EEPROM.read(reg), HEX );
  Serial.print("content: 0x" );
  Serial.println(content, HEX );
  if ( write_eeprom && (EEPROM.read(reg) != content) ) {
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
