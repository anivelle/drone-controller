#include "CustomI2C.h"
#include "mbed.h"

void CustomI2C::begin() {
    if (NRF_TWIM0->ENABLE)
        return;
    // Page 468
    // Sets up the I2C (Two-Wire Interface)
    NRF_P0->DIRCLR |= (1 << 31) + (1 << 2); // Just to ensure they are inputs
    NRF_TWIM0->PSEL.SCL = P0_2;
    NRF_TWIM0->PSEL.SDA = P0_31;
    // Page 247
    // PPI Shortcut between the EVENTS_LASTTX event and TASKS_STOP
    // Should stop transmission after the last bit without the need for an
    // interrupt
    NRF_PPI->CHEN = 0x01;
    NRF_PPI->CH[0].EEP = (uint32_t)&NRF_TWIM0->EVENTS_LASTTX;
    NRF_PPI->CH[0].TEP = (uint32_t)&NRF_TWIM0->TASKS_STOP;
    NRF_TWIM0->ENABLE = 1;
}

void CustomI2C::end() {
    if (!NRF_TWIM0->ENABLE)
        return;
    NRF_TWIM0->ENABLE = 0;
}

void CustomI2C::beginTransmission(uint8_t addr) {
    NRF_TWIM0->ADDRESS = (uint32_t)addr;
    NRF_TWIM0->TASKS_STARTTX = 1;
}

int CustomI2C::write(uint8_t data) { NRF_TWIM0->TXD.PTR = (uint32_t)&data; }

int CustomI2C::write(uint8_t *data, uint8_t len) {
    NRF_TWIM0->TXD.PTR = (uint32_t)data;
}

void CustomI2C::endTransmission() {
    NRF_TWIM0->TASKS_STOP = 1;
    NRF_TWIM0->INTENCLR = 1 << 24;
}

uint8_t CustomI2C::endTransmission(bool stopBit) { endTransmission(); }
