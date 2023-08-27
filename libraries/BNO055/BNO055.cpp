#include "BNO055.h"

BNO055::BNO055(TwoWire *i2c) {
    address = 0x28;
    page = 0;
    wire = i2c;
}

uint8_t BNO055::readRegister(Register reg) {
    uint8_t data;
    wire->beginTransmission(address);
    wire->write(reg);
    wire->requestFrom(reg, 1);
    data = wire->read();
    wire->endTransmission();
    return data;
}

size_t BNO055::writeRegister(Register reg, uint8_t data) {
    size_t written;
    wire->beginTransmission(address);
    wire->write(reg);
    written = wire->write(data);
    wire->endTransmission();
    return written;
}

uint16_t BNO055::readAcc(Axis axis) {
    char *buf;
    wire->beginTransmission(address);
    wire->requestFrom(8 + 2 * axis, 2);
    wire->readBytes(buf, 2);
    wire->endTransmission();
    return (buf[1] << 8) + buf[0];
}

uint16_t BNO055::readMag(Axis axis) {}

uint16_t BNO055::readGyr(Axis axis) {}

uint16_t BNO055::readQua(Axis axis) {}

uint16_t BNO055::readEul(Axis axis) {}

uint16_t BNO055::readLia(Axis axis) {}
