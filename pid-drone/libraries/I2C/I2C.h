#ifndef I2C_H
#define I2C_H

#include "mbed.h"

class I2C {
    void begin();
    void end();
    void beginTransmission(uint8_t addr);
    int write(uint8_t data);
    int write(uint8_t *data, uint8_t len);
};
#endif
