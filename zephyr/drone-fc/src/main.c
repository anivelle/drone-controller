#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/mem_manage.h>
#include "sparkfun_icm20948/ICM_20948_C.h" // Thank god for the Sparkfun library
#include "sparkfun_icm20948/AK09916_REGISTERS.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define DEFAULT_STACK 512
#define ICM_ADDR ICM_20948_I2C_ADDR_AD1

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
             "Console device is not ACM CDC UART device");
// Data buffer
// uint8_t icm20948_fifo_buffer[FIFO_SIZE];

// void icm20948_isr(const void *unused) {
//     // TODO: Check if right pin was activated here
//     k_sem_give(&icm20948_ready);
// }

ICM_20948_Status_e write(uint8_t reg, uint8_t *data, uint32_t len, void *user);
ICM_20948_Status_e read(uint8_t reg, uint8_t *buff, uint32_t len, void *user);
ICM_20948_Status_e initializeDMP(ICM_20948_Device_t *pdev);

K_THREAD_STACK_DEFINE(gyro_stack_area, DEFAULT_STACK);
struct k_thread gyro_thread_data;
//
extern void read_icm20948_data(void *dev, void *unused2, void *unused3) {
    const struct device *const i2c_dev = (const struct device *const)dev;
    while (1) {
        printk("Semaphore reset\n");
        uint8_t data_ready = 0;
        // icm20948_readregister(i2c_dev, ICM20948_DATA_RDY_STATUS,
        // &data_ready); if (!k_sem_take(&icm20948_ready, K_FOREVER)) {
        if (data_ready) {
            uint16_t bytes_ready = 0;
            // icm20948_get_fifo_count(i2c_dev, &bytes_ready);
            printk("FIFO count: %d\n", bytes_ready);
            // icm20948_read_fifo(i2c_dev, icm20948_fifo_buffer, bytes_ready);
            // uint8_t dummy;
            // icm20948_readregister(i2c_dev, ICM20948_INT_STATUS, &dummy);
        }
    }
}

int main(void) {

    // if (usb_enable(NULL))
    //     return 0;

    uint32_t dtr = 0;
    const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    while (!dtr) {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        /* Give CPU resources to low priority threads. */
        k_sleep(K_MSEC(100));
    }

    // Initialize I2C and set it to fast mode
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (i2c_dev == NULL || !device_is_ready(i2c_dev))
        printk("Could not get I2C device");

    uint32_t dev_config = I2C_SPEED_FAST | I2C_MODE_CONTROLLER;
    i2c_configure(i2c_dev, dev_config);

    ICM_20948_Device_t pdev;
    ICM_20948_init_struct(&pdev);

    pdev._dmp_firmware_available = true;
    ICM_20948_Serif_t serif = {
        .read = read, .write = write, .user = (void *)i2c_dev};
    ICM_20948_link_serif(&pdev, &serif);

    // This was just to check that I was interfacing properly
    uint8_t test;
    int err = ICM_20948_get_who_am_i(&pdev, &test);
    // printk("Error %d: %X\n", err, test);

    ICM_20948_sw_reset(&pdev);
    k_sleep(K_MSEC(250));
    err = initializeDMP(&pdev);
    // printk("Initialized DMP %d\n", err);

    err = inv_icm20948_enable_dmp_sensor(&pdev, INV_ICM20948_SENSOR_ORIENTATION,
                                   true);
    // printk("DMP Sens %d\n", err);
    err = inv_icm20948_set_dmp_sensor_period(&pdev, DMP_ODR_Reg_Quat9, 0);
    // err = ICM_20948_low_power(&pdev, false);
    // printk("DMP ODR %d\n", err);
    // printk("Enabling DMP %d\n", err);
    ICM_20948_enable_FIFO(&pdev, true);
    err = ICM_20948_enable_DMP(&pdev, true);
    // printk("Enabling DMP %d\n", err);
    ICM_20948_reset_DMP(&pdev);
    ICM_20948_reset_FIFO(&pdev);
    ICM_20948_low_power(&pdev, false);
    uint16_t count;
    icm_20948_DMP_data_t data;
    // k_sem_give(&icm20948_ready);
    // Set up the semaphore to block the thread until the interrupt occurs
    // printk("Semaphore status: %d\n", k_sem_take(&icm20948_ready, K_FOREVER));
    // Wake up the ICM20948 from sleep mode
    // uint8_t pwr_mode;
    // icm20948_readregister(i2c_dev, ICM20948_PWR_MGMT_1, &pwr_mode);
    // pwr_mode &= ~SLEEP;
    // icm20948_setregister(i2c_dev, ICM20948_PWR_MGMT_1, pwr_mode);

    // icm20948_readregister(i2c_dev, ICM20948_PWR_MGMT_1, &pwr_mode);
    // printk("Power mode: %X\n", pwr_mode);

    // Enable the DMP and FIFO on the IMU
    // printk("User config: %X\n", test);

    // printk("User config done\n");
    while (1) {
        ICM_20948_get_FIFO_count(&pdev, &count);
        ICM_20948_Status_e data_ready = ICM_20948_Stat_Err;
        if (count != 0)
            data_ready = inv_icm20948_read_dmp_data(&pdev, &data);
        // printf("FIFO count: %d\n", count);
        // printf("Data ready? %d\n", data_ready);
        if ((data_ready == ICM_20948_Stat_Ok ||
             data_ready == ICM_20948_Stat_FIFOMoreDataAvail) &&
            (data.header & DMP_header_bitmap_Quat9) > 0) {
            double q1 = ((double)data.Quat9.Data.Q1) /
                        1073741824.0; // Convert to double. Divide by 2^30
            double q2 = ((double)data.Quat9.Data.Q2) /
                        1073741824.0; // Convert to double. Divide by 2^30
            double q3 = ((double)data.Quat9.Data.Q3) /
                        1073741824.0; // Convert to double. Divide by 2^30
            double q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
            printf("{\"quat_w\":%f,\"quat_x\":%f,\"quat_y\":%f,\"quat_z\":%f}\n", q0, q1, q2, q3);
            // printf("Accuracy: %u\n", data.Quat9.Data.Accuracy);
        }
        k_sleep(K_MSEC(50));
    }

    // Enables DMP interrupts
    // icm20948_set_int(i2c_dev, DMP_INT1_EN);
    // char buf;
    // icm20948_readregister(i2c_dev, ICM20948_INT_STATUS, &buf);

    // printk("Interrupt registers: %X\n", buf);

    // Create and start a thread that constantly tries to read from the IMU
    /* k_tid_t icm20948_get_gyro = k_thread_create(
           &gyro_thread_data, gyro_stack_area,
           K_THREAD_STACK_SIZEOF(gyro_stack_area), &read_icm20948_data,
           (void *)&i2c_dev, NULL, NULL, 7, 0, K_NO_WAIT);
       k_thread_join(icm20948_get_gyro, K_FOREVER); */
    return 0;
}

ICM_20948_Status_e write(uint8_t reg, uint8_t *data, uint32_t len, void *user) {
    const struct device *const i2c_dev = (const struct device *const)user;
    uint8_t reg_data[len + 1];
    reg_data[0] = reg;
    memcpy(&reg_data[1], data, len);
    if (!i2c_write(i2c_dev, reg_data, len + 1, ICM_ADDR))
        return ICM_20948_Stat_Ok;
    return ICM_20948_Stat_Err;
}

ICM_20948_Status_e read(uint8_t reg, uint8_t *buff, uint32_t len, void *user) {
    const struct device *const i2c_dev = (const struct device *const)user;
    if (!i2c_burst_read(i2c_dev, ICM_ADDR, reg, buff, len))
        return ICM_20948_Stat_Ok;
    return ICM_20948_Stat_Err;
}

/* DMP has a whole initialization. Sparkfun doesn't provide this in the C files.
 * This is copied from ICM_20948.cpp
 */
ICM_20948_Status_e initializeDMP(ICM_20948_Device_t *pdev) {
    ICM_20948_Status_e result = ICM_20948_Stat_Ok;
    result |= ICM_20948_i2c_controller_configure_peripheral(
        pdev, 0, MAG_AK09916_I2C_ADDR, AK09916_REG_RSV2, 10, true, true, false,
        true, true, 0);

    result |= ICM_20948_i2c_controller_configure_peripheral(
        pdev, 1, MAG_AK09916_I2C_ADDR, AK09916_REG_CNTL2, 1, false, true, false,
        false, false, AK09916_mode_single);

    result |= ICM_20948_set_bank(pdev, 3);
    uint8_t config = 0x04;
    result |= pdev->_serif->write(AGB3_REG_I2C_MST_ODR_CONFIG, &config, 1,
                                  pdev->_serif->user);

    result |= ICM_20948_set_clock_source(pdev, ICM_20948_Clock_Auto);

    result |= ICM_20948_set_bank(pdev, 1);
    config = 0x40;
    result |= pdev->_serif->write(AGB0_REG_PWR_MGMT_2, &config, 1,
                                  pdev->_serif->user);

    result |= ICM_20948_set_sample_mode(pdev, ICM_20948_Internal_Mst,
                                        ICM_20948_Sample_Mode_Cycled);

    result |= ICM_20948_enable_FIFO(pdev, false);
    result |= ICM_20948_enable_DMP(pdev, false);

    ICM_20948_fss_t fss;
    fss.a = gpm4;
    fss.g = dps2000;
    result |= ICM_20948_set_full_scale(
        pdev, (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), fss);
    result |= ICM_20948_enable_dlpf(pdev, ICM_20948_Internal_Gyr, true);

    // Enable interrupt for FIFO overflow from FIFOs through INT_ENABLE_2
    // If we see this interrupt, we'll need to reset the FIFO
    // result = intEnableOverflowFIFO( 0x1F ); if (result > worstResult)
    // worstResult = result; // Enable the interrupt on all FIFOs

    // Turn off what goes into the FIFO through FIFO_EN_1, FIFO_EN_2
    // Stop the peripheral data from being written to the FIFO by writing zero
    // to FIFO_EN_1
    result |= ICM_20948_set_bank(pdev, 0);
    uint8_t zero = 0;
    result |=
        pdev->_serif->write(AGB0_REG_FIFO_EN_1, &zero, 1, pdev->_serif->user);
    // Stop the accelerometer, gyro and temperature data from being written to
    // the FIFO by writing zero to FIFO_EN_2
    result |=
        pdev->_serif->write(AGB0_REG_FIFO_EN_2, &zero, 1, pdev->_serif->user);

    // Turn off data ready interrupt through INT_ENABLE_1
    ICM_20948_INT_enable_t en;
    result |= ICM_20948_int_enable(pdev, NULL, &en);
    en.RAW_DATA_0_RDY_EN = false;
    result |= ICM_20948_int_enable(pdev, &en, &en);

    // Reset FIFO through FIFO_RST
    result |= ICM_20948_reset_FIFO(pdev);

    // Set gyro sample rate divider with GYRO_SMPLRT_DIV
    // Set accel sample rate divider with ACCEL_SMPLRT_DIV_2
    ICM_20948_smplrt_t mySmplrt;
    // mySmplrt.g =
        // 19; // ODR is computed as follows: 1.1 kHz/(1+GYRO_SMPLRT_DIV[7:0]). 19
            // = 55Hz. InvenSense Nucleo example uses 19 (0x13).
    // mySmplrt.a =
        // 19; // ODR is computed as follows: 1.125 kHz/(1+ACCEL_SMPLRT_DIV[11:0]).
            // 19 = 56.25Hz. InvenSense Nucleo example uses 19 (0x13).
    mySmplrt.g = 4; // 225Hz
    mySmplrt.a = 4; // 225Hz
    // mySmplrt.g = 8; // 112Hz
    // mySmplrt.a = 8; // 112Hz
    result |= ICM_20948_set_sample_rate(
        pdev, (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), mySmplrt);

    // Setup DMP start address through PRGM_STRT_ADDRH/PRGM_STRT_ADDRL
    result |= ICM_20948_set_dmp_start_address(pdev, DMP_START_ADDRESS);

    // Now load the DMP firmware
    result |= ICM_20948_firmware_load(pdev);

    // Write the 2 byte Firmware Start Value to ICM
    // PRGM_STRT_ADDRH/PRGM_STRT_ADDRL
    result |= ICM_20948_set_dmp_start_address(pdev, DMP_START_ADDRESS);

    // Set the Hardware Fix Disable register to 0x48
    result |= ICM_20948_set_bank(pdev, 0);
    uint8_t fix = 0x48;
    result |= pdev->_serif->write(AGB0_REG_HW_FIX_DISABLE, &fix, 1,
                                  pdev->_serif->user);

    // Set the Single FIFO Priority Select register to 0xE4
    result |= ICM_20948_set_bank(pdev, 0);
    uint8_t fifoPrio = 0xE4;
    result |= pdev->_serif->write(AGB0_REG_SINGLE_FIFO_PRIORITY_SEL, &fifoPrio,
                                  1, pdev->_serif->user);

    // Configure Accel scaling to DMP
    // The DMP scales accel raw data internally to align 1g as 2^25
    // In order to align internal accel raw data 2^25 = 1g write 0x04000000 when
    // FSR is 4g
    const unsigned char accScale[4] = {0x04, 0x00, 0x00, 0x00};
    result |= inv_icm20948_write_mems(pdev, ACC_SCALE, 4, &accScale[0]);
    // Write accScale to ACC_SCALE DMP register
    // In order to output hardware unit data as configured FSR write 0x00040000
    // when FSR is 4g
    const unsigned char accScale2[4] = {0x00, 0x04, 0x00, 0x00};
    result |= inv_icm20948_write_mems(pdev, ACC_SCALE2, 4, &accScale2[0]);
    // Write accScale2 to ACC_SCALE2 DMP register

    // Configure Compass mount matrix and scale to DMP
    // The mount matrix write to DMP register is used to align the compass axes
    // with accel/gyro. This mechanism is also used to convert hardware unit to
    // uT. The value is expressed as 1uT = 2^30. Each compass axis will be
    // converted as below: X = raw_x * CPASS_MTX_00 + raw_y * CPASS_MTX_01 +
    // raw_z * CPASS_MTX_02 Y = raw_x * CPASS_MTX_10 + raw_y * CPASS_MTX_11 +
    // raw_z * CPASS_MTX_12 Z = raw_x * CPASS_MTX_20 + raw_y * CPASS_MTX_21 +
    // raw_z * CPASS_MTX_22 The AK09916 produces a 16-bit signed output in the
    // range +/-32752 corresponding to +/-4912uT. 1uT = 6.66 ADU. 2^30 / 6.66666
    // = 161061273 = 0x9999999
    const unsigned char mountMultiplierZero[4] = {0x00, 0x00, 0x00, 0x00};
    const unsigned char mountMultiplierPlus[4] = {
        0x09, 0x99, 0x99, 0x99}; // Value taken from InvenSense Nucleo example
    const unsigned char mountMultiplierMinus[4] = {
        0xF6, 0x66, 0x66, 0x67}; // Value taken from InvenSense Nucleo example
    result |=
        inv_icm20948_write_mems(pdev, CPASS_MTX_00, 4, &mountMultiplierPlus[0]);
    result |=
        inv_icm20948_write_mems(pdev, CPASS_MTX_01, 4, &mountMultiplierZero[0]);
    result |=
        inv_icm20948_write_mems(pdev, CPASS_MTX_02, 4, &mountMultiplierZero[0]);
    result |=
        inv_icm20948_write_mems(pdev, CPASS_MTX_10, 4, &mountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, CPASS_MTX_11, 4,
                                      &mountMultiplierMinus[0]);
    result |=
        inv_icm20948_write_mems(pdev, CPASS_MTX_12, 4, &mountMultiplierZero[0]);
    result |=
        inv_icm20948_write_mems(pdev, CPASS_MTX_20, 4, &mountMultiplierZero[0]);
    result |=
        inv_icm20948_write_mems(pdev, CPASS_MTX_21, 4, &mountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, CPASS_MTX_22, 4,
                                      &mountMultiplierMinus[0]);

    // Configure the B2S Mounting Matrix
    const unsigned char b2sMountMultiplierZero[4] = {0x00, 0x00, 0x00, 0x00};
    const unsigned char b2sMountMultiplierPlus[4] = {
        0x40, 0x00, 0x00, 0x00}; // Value taken from InvenSense Nucleo example
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_00, 4,
                                      &b2sMountMultiplierPlus[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_01, 4,
                                      &b2sMountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_02, 4,
                                      &b2sMountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_10, 4,
                                      &b2sMountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_11, 4,
                                      &b2sMountMultiplierPlus[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_12, 4,
                                      &b2sMountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_20, 4,
                                      &b2sMountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_21, 4,
                                      &b2sMountMultiplierZero[0]);
    result |= inv_icm20948_write_mems(pdev, B2S_MTX_22, 4,
                                      &b2sMountMultiplierPlus[0]);

    // Configure the DMP Gyro Scaling Factor
    // @param[in] gyro_div Value written to GYRO_SMPLRT_DIV register, where
    //            0=1125Hz sample rate, 1=562.5Hz sample rate, ... 4=225Hz
    //            sample rate, ... 10=102.2727Hz sample rate, ... etc.
    // @param[in] gyro_level 0=250 dps, 1=500 dps, 2=1000 dps, 3=2000 dps
    result |= inv_icm20948_set_gyro_sf(pdev, 4, 3);
    // 19 = 55Hz (see above), 3 = 2000dps (see above)

    // Configure the Gyro full scale
    // 2000dps : 2^28
    // 1000dps : 2^27
    //  500dps : 2^26
    //  250dps : 2^25
    const unsigned char gyroFullScale[4] = {0x10, 0x00, 0x00,
                                            0x00}; // 2000dps : 2^28
    result |=
        inv_icm20948_write_mems(pdev, GYRO_FULLSCALE, 4, &gyroFullScale[0]);

    // Configure the Accel Only Gain: 15252014 (225Hz) 30504029 (112Hz) 61117001
    // (56Hz)
    // const unsigned char accelOnlyGain[4] = {0x03, 0xA4, 0x92, 0x49}; // 56Hz
    const unsigned char accelOnlyGain[4] = {0x00, 0xE8, 0xBA, 0x2E}; // 225Hz
    // const unsigned char accelOnlyGain[4] = {0x01, 0xD1, 0x74, 0x5D}; // 112Hz
    result |=
        inv_icm20948_write_mems(pdev, ACCEL_ONLY_GAIN, 4, &accelOnlyGain[0]);

    // Configure the Accel Alpha Var: 1026019965 (225Hz) 977872018 (112Hz)
    // 882002213 (56Hz)
    // const unsigned char accelAlphaVar[4] = {0x34, 0x92, 0x49, 0x25}; // 56Hz
    const unsigned char accelAlphaVar[4] = {0x3D, 0x27, 0xD2, 0x7D}; // 225Hz
    // const unsigned char accelAlphaVar[4] = {0x3A, 0x49, 0x24, 0x92}; // 112Hz
    result |=
        inv_icm20948_write_mems(pdev, ACCEL_ALPHA_VAR, 4, &accelAlphaVar[0]);

    // Configure the Accel A Var: 47721859 (225Hz) 95869806 (112Hz) 191739611
    // (56Hz)
    // const unsigned char accelAVar[4] = {0x0B, 0x6D, 0xB6, 0xDB}; // 56Hz
    const unsigned char accelAVar[4] = {0x02, 0xD8, 0x2D, 0x83}; // 225Hz
    // const unsigned char accelAVar[4] = {0x05, 0xB6, 0xDB, 0x6E}; // 112Hz
    result |= inv_icm20948_write_mems(pdev, ACCEL_A_VAR, 4, &accelAVar[0]);

    // Configure the Accel Cal Rate
    const unsigned char accelCalRate[4] = {
        0x00, 0x00}; // Value taken from InvenSense Nucleo example
    result |=
        inv_icm20948_write_mems(pdev, ACCEL_CAL_RATE, 2, &accelCalRate[0]);

    // Configure the Compass Time Buffer. The I2C Master ODR Configuration (see
    // above) sets the magnetometer read rate to 68.75Hz. Let's set the Compass
    // Time Buffer to 69 (Hz).
    const unsigned char compassRate[2] = {0x00, 0x45}; // 69Hz
    result |=
        inv_icm20948_write_mems(pdev, CPASS_TIME_BUFFER, 2, &compassRate[0]);

    // Enable DMP interrupt
    // This would be the most efficient way of getting the DMP data, instead of
    // polling the FIFO

    // result |= ICM_20948_int_enable(pdev, NULL, &en);
    // en.DMP_INT1_EN = true;
    // result |= ICM_20948_int_enable(pdev, &en, &en);

    return result;
}
