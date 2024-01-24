#include "ICM20948.h"

ICM20948::ICM20948(TwoWire *i2c) {
    address = 0x28;
    page = 0;
    wire = i2c;
    mode = CONFIG;
}

int16_t ICM20948::readRegister(Register reg) {
    wire->beginTransmission(address);
    size_t written = wire->write(reg);
    written = wire->endTransmission();
    written = wire->requestFrom(address, 1);
    return wire->read();
}

size_t ICM20948::writeRegister(Register reg, uint8_t data) {
    size_t written;
    wire->beginTransmission(address);
    written = wire->write(reg);
    written = wire->write(data);
    return wire->endTransmission(true);
}

int ICM20948::changePage(uint8_t page) {
    if (this->page == page)
        return -1;
    writeRegister(PAGE_ID, page);
    this->page = page;
    return 0;
}

int ICM20948::changeMode(Mode mode) {
    if (this->page)
        return -1;
    writeRegister(OPR_MODE, mode);
    this->mode = mode;
    return 0;
}

// Returns 0xFFFF if nothing was read
int16_t ICM20948::readAcc(Axis axis) {
    int16_t val = 0xFFFF;
    wire->beginTransmission(address);
    // Offset the register address by two to go to the next axis
    wire->write(ACC_DATA_X_LSB + 2 * axis);
    wire->endTransmission();
    wire->requestFrom(address, 2);
    if (wire->available()) {
        val = 0;
        for (int i = 0; i < 2; i++) {
            val |= wire->read() << (8 * i);
        }
    }
    return val;
}

int16_t ICM20948::readMag(Axis axis) {
    int16_t val = 0xFFFF;
    wire->beginTransmission(address);
    // Offset the register address by two to go to the next axis
    wire->write(MAG_DATA_X_LSB + 2 * axis);
    wire->endTransmission();
    wire->requestFrom(address, 2);
    if (wire->available()) {
        val = 0;
        for (int i = 0; i < 2; i++) {
            val |= wire->read() << (8 * i);
        }
    }
    return val;
}

int16_t ICM20948::readGyr(Axis axis) {
    int16_t val = 0xFFFF;
    wire->beginTransmission(address);
    // Offset the register address by two to go to the next axis
    wire->write(GYR_DATA_X_LSB + 2 * axis);
    wire->endTransmission();
    wire->requestFrom(address, 2);
    if (wire->available()) {
        val = 0;
        for (int i = 0; i < 2; i++) {
            val |= wire->read() << (8 * i);
        }
    }
    return val;
}

int16_t ICM20948::readQua(Axis axis) {
    int16_t val = 0xFFFF;
    wire->beginTransmission(address);
    // Offset the register address by two to go to the next axis
    wire->write(QUA_DATA_W_LSB + 2 * axis);
    wire->endTransmission();
    wire->requestFrom(address, 2);
    if (wire->available()) {
        val = 0;
        for (int i = 0; i < 2; i++) {
            val |= wire->read() << (8 * i);
        }
    }
    return val;
}

int16_t ICM20948::readEul(Axis axis) {
    int16_t val = 0xFFFF;
    wire->beginTransmission(address);
    // Offset the register address by two to go to the next axis
    wire->write(EUL_HEADING_LSB + 2 * axis);
    wire->endTransmission();
    wire->requestFrom(address, 2);
    if (wire->available()) {
        val = 0;
        for (int i = 0; i < 2; i++) {
            val |= wire->read() << (8 * i);
        }
    }
    return val;
}

int16_t ICM20948::readLia(Axis axis) {
    int16_t val = 0xFFFF;
    wire->beginTransmission(address);
    // Offset the register address by two to go to the next axis
    wire->write(LIA_DATA_X_LSB + 2 * axis);
    wire->endTransmission();
    wire->requestFrom(address, 2);
    if (wire->available()) {
        val = 0;
        for (int i = 0; i < 2; i++) {
            val |= wire->read() << (8 * i);
        }
    }
    return val;
}
