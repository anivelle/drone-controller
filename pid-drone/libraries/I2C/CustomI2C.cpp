#include "CustomI2C.h"
#include "mbed.h"

#define TWI_IRQn SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn
#define TWIM0_IRQHandler SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler

void TWIM0_IRQHandler(void) {
    if (NRF_TWIM0->EVENTS_LASTTX) {
        NRF_TWIM0->EVENTS_LASTTX = 0;
        CustomI2C::endTransmit(); // This is probably bad but I don't really
                                  // care
        uint32_t read = NRF_TWIM0->EVENTS_LASTTX;
        CustomI2C::tester = 1;
    }
}

void CustomI2C::begin() {
    if (NRF_TWIM0->ENABLE)
        return;
    tester = 0;
    NRF_P0->DIRCLR |= (1 << 31) + (1 << 2); // Just to ensure they are inputs
    NRF_TWIM0->PSEL.SCL = P0_2;
    NRF_TWIM0->PSEL.SDA = P0_31;
    NRF_TWIM0->TXD.AMOUNT = 256;
    NRF_TWIM0->INTENSET = 1 << 24;
    NVIC_EnableIRQ(TWI_IRQn);
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
