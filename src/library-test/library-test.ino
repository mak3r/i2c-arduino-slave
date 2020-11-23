#include <I2CSlaveMode.h>

I2CSlaveMode i2cSlaveMode(0x08);

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(300);
  i2cSlaveMode.resetIfRequested();
}
