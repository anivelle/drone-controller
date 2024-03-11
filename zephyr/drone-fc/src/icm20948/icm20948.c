#include "icm20948.h"

// After writing these I notice that they're really just wrappers haha

int icm20948_userconfig(const struct device *dev, const uint8_t mode) {
    return icm20948_setregister(dev, ICM20948_USER_CTRL, mode);
}

int icm20948_set_int(const struct device *dev, const uint8_t intmode) {
    return icm20948_setregister(dev, ICM20948_INT_ENABLE, intmode);
}

int icm20948_setregister(const struct device *dev, const uint8_t reg,
                         const uint8_t data) {
    return i2c_reg_write_byte(dev, ICM20948_ADDRESS, reg, data);
}

int icm20948_readregister(const struct device *dev, const uint8_t reg,
                          uint8_t *buf) {
    return i2c_reg_read_byte(dev, ICM20948_ADDRESS, reg, buf);
}
int icm20948_read_fifo(const struct device *dev, uint8_t *buf, uint8_t count) {
    return i2c_burst_read(dev, ICM20948_ADDRESS, ICM20948_FIFO_R_W, buf, count);
}

int icm20948_get_fifo_count(const struct device *dev, uint16_t *count) {
    return i2c_burst_read(dev, ICM20948_ADDRESS, ICM20948_FIFO_COUNTH,
                          (uint8_t *)count, 2);
}
