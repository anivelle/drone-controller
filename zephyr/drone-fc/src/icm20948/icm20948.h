#ifndef ICM20948_H
#define ICM20948_H

#include <zephyr/drivers/i2c.h>

#define ICM20948_ADDRESS 0x69

// USER_CTRL register settings (p. 36)
#define DMP_EN (1 << 7)      // Enable DMP (Digital Motion Processor)
#define FIFO_EN (1 << 6)     // Enable FIFO
#define I2C_MST_EN (1 << 5)  // Enable I2C controller for exteral sensors
#define I2C_IF_DIS (1 << 4)  // Reset I2C peripheral and enable SPI only
#define DMP_RST (1 << 3)     // Reset DMP (bit auto-clears)
#define SRAM_RST (1 << 2)    // Reset SRAM (bit auto-clears)
#define I2C_MST_RST (1 << 1) // Reset I2C controller module (bit auto-clears)

// INT_PIN_CFG settings
#define INT1_ACTL (1 << 7)          // Interrupt pin is active low
#define INT1_OPEN (1 << 6)          // Interrupt pin is open drain (not push-pull)
#define INT1_LATCH_EN (1 << 5)      // Interrupt is held until cleared
#define INT_ANYRD_2CLEAR (1 << 4)   // Interrupt is cleared by any read operation
#define ACTL_FSYNC (1 << 3)         // FSYNC (p. 25) is active low
#define FSYNC_INT_MODE_EN (1 << 2)  // FSYNC can be used as an interrupt
#define BYPASS_EN (1 << 1)          // I2C_MST interface is put into bypass mode

// INT_ENABLE settings
#define REG_WOF_EN (1 << 7)     // Enable wake on FSYNC interrupt
#define WOM_INT_EN (1 << 3)     // Enable Wake-On-Motion interrupt
#define PLL_RDY_EN (1 << 2)     // Enable PLL RDY interrupt (from gyro?)
#define DMP_INT1_EN (1 << 1)    // Enable DMP interrupt
#define I2C_MST_INT_EN (1 << 0) // Enable I2C interrupt

// FIFO_EN_1 settings (for reading external I2C devices)
#define SLV_3_FIFO_EN (1 << 3)
#define SLV_2_FIFO_EN (1 << 2)
#define SLV_1_FIFO_EN (1 << 1)
#define SLV_0_FIFO_EN (1 << 0)

// FIFO_EN_2 settings
#define ACCEL_FIFO_EN   (1 << 4)  // Writes all acceleration data to FIFO
#define GYRO_Z_FIFO_EN  (1 << 3)  // Writes gyro Z-axis data to FIFO
#define GYRO_Y_FIFO_EN  (1 << 2)  // Writes gyro Y-axis data to FIFO
#define GYRO_X_FIFO_EN  (1 << 1)  // Writes gyro X-axis data to FIFO
#define TEMP_FIFO_EN    (1 << 0)  // Writes temperature data to FIFO 

// FIFO_RST and FIFO_MODE settings 
#define FIFO_RESET (1 << 0)   // Resets the FIFO. Hold to set FIFO size to 0
#define FIFO_MODE (1 << 0)    // Stream - Additional writes override FIFO data

// Ignoring most of the other register settings for now

// I probably did not need all of these registers written out
enum icm20948_register_0 {
    ICM20948_WHO_AM_I,
    ICM20948_USER_CTRL = 3,
    ICM20948_LP_CONFIG = 5,
    ICM20948_PWR_MGMT_1,
    ICM20948_PWR_MGMT_2,
    ICM20948_INT_PIN_CFG = 15,
    ICM20948_INT_ENABLE,
    ICM20948_INT_ENABLE_1,
    ICM20948_INT_ENABLE_2,
    ICM20948_INT_ENABLE_3,
    ICM20948_I2C_MST_STATUS = 23,
    ICM20948_INT_STATUS = 25,
    ICM20948_INT_STATUS_1,
    ICM20948_INT_STATUS_2,
    ICM20948_INT_STATUS_3,
    ICM20948_DELAY_TIMEH = 40,
    ICM20948_DELAY_TIMEL,
    ICM20948_ACCEL_XOUT_H = 45,
    ICM20948_ACCEL_XOUT_L,
    ICM20948_ACCEL_YOUT_H,
    ICM20948_ACCEL_YOUT_L,
    ICM20948_ACCEL_ZOUT_H,
    ICM20948_ACCEL_ZOUT_L,
    ICM20948_GYRO_XOUT_H,
    ICM20948_GYRO_XOUT_L,
    ICM20948_GYRO_YOUT_H,
    ICM20948_GYRO_YOUT_L,
    ICM20948_GYRO_ZOUT_H,
    ICM20948_GYRO_ZOUT_L,
    ICM20948_TEMP_OUT_H,
    ICM20948_TEMP_OUT_L,
    ICM20948_EXT_SLV_SENS_DATA_00,
    ICM20948_EXT_SLV_SENS_DATA_01,
    ICM20948_EXT_SLV_SENS_DATA_02,
    ICM20948_EXT_SLV_SENS_DATA_03,
    ICM20948_EXT_SLV_SENS_DATA_04,
    ICM20948_EXT_SLV_SENS_DATA_05,
    ICM20948_EXT_SLV_SENS_DATA_06,
    ICM20948_EXT_SLV_SENS_DATA_07,
    ICM20948_EXT_SLV_SENS_DATA_08,
    ICM20948_EXT_SLV_SENS_DATA_09,
    ICM20948_EXT_SLV_SENS_DATA_10,
    ICM20948_EXT_SLV_SENS_DATA_11,
    ICM20948_EXT_SLV_SENS_DATA_12,
    ICM20948_EXT_SLV_SENS_DATA_13,
    ICM20948_EXT_SLV_SENS_DATA_14,
    ICM20948_EXT_SLV_SENS_DATA_15,
    ICM20948_EXT_SLV_SENS_DATA_16,
    ICM20948_EXT_SLV_SENS_DATA_17,
    ICM20948_EXT_SLV_SENS_DATA_18,
    ICM20948_EXT_SLV_SENS_DATA_19,
    ICM20948_EXT_SLV_SENS_DATA_20,
    ICM20948_EXT_SLV_SENS_DATA_21,
    ICM20948_EXT_SLV_SENS_DATA_22,
    ICM20948_EXT_SLV_SENS_DATA_23,
    ICM20948_FIFO_EN_1 = 102,
    ICM20948_FIFO_EN_2,
    ICM20948_FIFO_RST,
    ICM20948_FIFO_MODE,
    ICM20948_FIFO_COUNTH = 112,
    ICM20948_FIFO_COUNTL,
    ICM20948_FIFO_R_W,
    ICM20948_DATA_RDY_STATUS = 116,
    ICM20948_FIFO_CFG = 118,
    ICM20948_REG_BANK_SEL = 127
};


int icm20948_userconfig(const struct device *dev, const uint8_t mode);
int icm20948_set_int(const struct device *dev, const uint8_t intmode);

int icm20948_setregister(const struct device *dev, const uint8_t reg, const uint8_t data);
int icm20948_readregister(const struct device *dev, const uint8_t reg, uint8_t *buf);

int icm20948_get_fifo_count(const struct device *dev, uint16_t *buf);
int icm20948_read_fifo(const struct device *dev, uint8_t *buf, const uint8_t count);

#endif
