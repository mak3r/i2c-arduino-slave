/*
 * Using the Blink without delay example as a basis
 * Blink the builtin led based on register 0x22 and 0x23 values
 * register 0x22 indicates whether to enable or disable the led
 * register 0x23 indicates the blink duration.
 * 
 * On the I2C master: 
 * set the duration to 1 second
 * i2cset -y 1 0x08 0x23 0x0A
 * enable blinking
 * i2cset -y 1 0x08 0x22 0x01
 * change the duration to 200 ms
 * i2cset -y 1 0x08 0x23 0x02
 */

#include <I2CSlaveMode.h>

I2CSlaveMode i2cSlaveMode(0x08);

// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
long interval;           // interval at which to blink (milliseconds)

void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);//initialize the led to off
}


void loop() {

  unsigned long currentMillis = millis();
  //Check for updates to how long to blink
  interval = i2cSlaveMode.getRegister(0x23)*100;

  //Check if we should blink
  if (i2cSlaveMode.getRegister(0x22) > 0) {
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
  
      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }
  
      // set the LED with the ledState of the variable:
      digitalWrite(ledPin, ledState);
    }
  } else {
    //Turn off the LED if not specified to blink
    digitalWrite(ledPin,LOW);
  }
  
}
