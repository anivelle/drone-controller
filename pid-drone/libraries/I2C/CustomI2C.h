#ifndef I2C_H
#define I2C_H

#include <stdint.h>

class CustomI2C {
  private:
    void begin();
    void end();

    void beginTransmission(uint8_t addr);
    static void endTransmission();
    int write(uint8_t data);
    int write(uint8_t *data, uint8_t len);

  public:
    static void endTransmit() { endTransmission(); }
    volatile uint8_t tester; // I just want this to be accessible!!!!!!
};

#endif
