  /*
   * If you want to allow the device to be reset programmatically by the I2C master
   * then you will need to include resetIfRequested() in loop()
   * AND wire the resetPin to a GPIO output (pin12 by default).
   * 
   * Programmatic reset is in no way required and a hard reset will have
   * the same result.
   * 
   * Include only if you know what your are doing.
   * 
   * Example Master:
   * i2cset -y 1 0x08 0x00 0x80
   * 
   * @see: https://github.com/mak3r/i2c-arduino-slave
   */
  #include <I2CSlaveMode.h>

I2CSlaveMode i2cSlaveMode(0x08);

void setup() {
}

void loop() {
  i2cSlaveMode.resetIfRequested();
}
