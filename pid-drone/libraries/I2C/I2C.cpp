#include "I2C.h"

void I2C::begin() {

    NRF_P0->DIRCLR |= (1 << 31) + (1 << 2); // Just to ensure they are inputs
    NRF_TWIM0->PSEL.SCL = P0_2;
    NRF_TWIM0->PSEL.SDA = P0_31;
    NRF_TWIM0->ENABLE = 1;
}

void I2C::end() { NRF_TWIM0->ENABLE = 0; }

void I2C::beginTransmission(uint8_t addr) {
    NRF_TWIM0->ADDRESS = (uint32_t)addr;
    NRF_TWIM0->TASKS_STARTTX = 1;
}
int I2C::write(uint8_t data) {}
