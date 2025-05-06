#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/mem_manage.h>
#include "sparkfun_icm20948/ICM_20948_C.h" // Thank god for the Sparkfun library
#include "sparkfun_icm20948/AK09916_REGISTERS.h"
// #include <nrfx_gpiote.h>
#include "vl53l4cx/vl53l4cx_class.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define DEFAULT_STACK 512
#define ICM_ADDR ICM_20948_I2C_ADDR_AD1

const uint8_t MAX_MAGNETOMETER_STARTS = 10;
struct k_sem icm20948_ready;

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
             "Console device is not ACM CDC UART device");
// Data buffer
// uint8_t icm20948_fifo_buffer[FIFO_SIZE];

// void icm20948_isr(const void *unused) {
//     // TODO: Check if right pin was activated here
//     k_sem_give(&icm20948_ready);
// }

int write(uint8_t addr, uint8_t reg, uint8_t *data, uint32_t len, void *user);
int read(uint8_t addr, uint8_t *reg, uint8_t regLen, uint8_t *buff,
         uint32_t len, void *user);

ICM_20948_Status_e initializeDMP(ICM_20948_Device_t *pdev);
ICM_20948_Status_e setup_IMU(ICM_20948_Device_t *pdev,
                             ICM_20948_Serif_t *serif);
ICM_20948_Status_e startupDefault(ICM_20948_Device_t *pdev, bool minimal);
ICM_20948_Status_e startupMagnetometer(ICM_20948_Device_t *pdev, bool minimal);
ICM_20948_Status_e readMagnetometer(ICM_20948_Device_t *pdev,
                                    AK09916_Reg_Addr_e reg, uint8_t *data);

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
    // k_sem_init(&icm20948_ready, 0, 1);
    // IRQ_CONNECT(6, 1, icm20948_isr, NULL, 0)
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

    uint32_t dev_config;
    if (!i2c_get_config(i2c_dev, &dev_config))
        dev_config |= I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_CONTROLLER;
    else
        dev_config = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_CONTROLLER;
    i2c_configure(i2c_dev, dev_config);

    ICM_20948_Device_t icm20948;
    ICM_20948_init_struct(&icm20948);

    serif_t vl53l4cx_serif = {.i2c_dev = i2c_dev, .write = write, .read = read};
    
    VL53L4CX_Dev_t vl53l4cx; 
    
    const struct device *const gpio_port = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    VL53L4CX_init_device(&vl53l4cx, &vl53l4cx_serif, gpio_port, 14);

    begin(&vl53l4cx);
    
    ICM_20948_Serif_t icm20948_serif = {
        .read = read, .write = write, .user = (void *)i2c_dev};
    bool init = false;
    int err;
    do {
        err = setup_IMU(&icm20948, &icm20948_serif);
        if (err != ICM_20948_Stat_Ok)
            k_sleep(K_MSEC(500));
        else
            init = true;

    } while (!init);

    printk("Setup error %d\n", err);

    // This was just to check that I was interfacing properly
    // uint8_t test;
    // err = ICM_20948_get_who_am_i(&pdev, &test);
    // printk("Error %d: %X\n", err, test);

    // ICM_20948_sw_reset(&pdev);
    // k_sleep(K_MSEC(500));
    err = initializeDMP(&icm20948);
    // printk("Initialized DMP %d\n", err);

    err = inv_icm20948_enable_dmp_sensor(&icm20948, INV_ICM20948_SENSOR_ORIENTATION,
                                         true);
    // printk("DMP Sens %d\n", err);
    err = inv_icm20948_set_dmp_sensor_period(&icm20948, DMP_ODR_Reg_Quat9, 0);
    // printk("DMP Sens period %d\n", err);

    ICM_20948_enable_FIFO(&icm20948, true);
    err = ICM_20948_enable_DMP(&icm20948, true);
    // printk("Enabling DMP %d\n", err);
    ICM_20948_reset_DMP(&icm20948);
    ICM_20948_reset_FIFO(&icm20948);
    uint16_t count;
    icm_20948_DMP_data_t data;
    // k_sem_give(&icm20948_ready);

    // irq_enable(6);
    // Set up the semaphore to block the thread until the interrupt occurs
    // printk("Semaphore status: %d\n", k_sem_take(&icm20948_ready, K_FOREVER));

    // printk("User config done\n");
    ICM_20948_Status_e data_ready = ICM_20948_Stat_Err;
    while (1) {

        data_ready = inv_icm20948_read_dmp_data(&icm20948, &data);
        if ((data_ready == ICM_20948_Stat_Ok ||
             data_ready == ICM_20948_Stat_FIFOMoreDataAvail)) {
            if ((data.header & DMP_header_bitmap_Quat9) > 0) {
                double q1 = ((double)data.Quat9.Data.Q1) /
                            1073741824.0; // Convert to double. Divide by 2^30
                double q2 = ((double)data.Quat9.Data.Q2) /
                            1073741824.0; // Convert to double. Divide by 2^30
                double q3 = ((double)data.Quat9.Data.Q3) /
                            1073741824.0; // Convert to double. Divide by 2^30
                double q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
                printf(
                    "{\"quat_w\":%.3f,\"quat_x\":%.3f,\"quat_y\":%.3f,\"quat_"
                    "z\":%.3f}\n",
                    q0, q1, q2, q3);
            }
        }
        if (data_ready != ICM_20948_Stat_FIFOMoreDataAvail)
            k_sleep(K_MSEC(1));
    }

    // Create and start a thread that constantly tries to read from the IMU
    /* k_tid_t icm20948_get_gyro = k_thread_create(
           &gyro_thread_data, gyro_stack_area,
           K_THREAD_STACK_SIZEOF(gyro_stack_area), &read_icm20948_data,
           (void *)&i2c_dev, NULL, NULL, 7, 0, K_NO_WAIT);
       k_thread_join(icm20948_get_gyro, K_FOREVER); */
    return 0;
}

// int write(uint8_t addr, uint8_t *buf, int numWrite, void *user) {
//   const struct device *i2c_dev = (const struct device *)user;
//   return i2c_write(i2c_dev, buf, numWrite, addr);
// }
//
// int read(uint8_t addr, uint8_t *writeBuf, uint8_t numWrite, uint8_t *readBuf,
//          int numRead, void *user) {
//   const struct device *i2c_dev = (const struct device *)user;
//   return i2c_write_read(i2c_dev, addr, writeBuf, numWrite, readBuf, numRead);
// }

int write(uint8_t addr, uint8_t reg, uint8_t *data, uint32_t len, void *user) {
    const struct device *const i2c_dev = (const struct device *const)user;
    uint8_t reg_data[len + 1];
    reg_data[0] = reg;
    memcpy(&reg_data[1], data, len);
    if (!i2c_write(i2c_dev, reg_data, len + 1, addr))
        return ICM_20948_Stat_Ok;
    return ICM_20948_Stat_Err;
}

int read(uint8_t addr, uint8_t *reg, uint8_t regLen, uint8_t *buff,
         uint32_t len, void *user) {
    const struct device *const i2c_dev = (const struct device *const)user;

    if (!i2c_write_read(i2c_dev, addr, &reg, regLen, buff, len))
        return ICM_20948_Stat_Ok;
    return ICM_20948_Stat_Err;
}

ICM_20948_Status_e readMagnetometer(ICM_20948_Device_t *pdev,
                                    AK09916_Reg_Addr_e reg, uint8_t *data) {

    return ICM_20948_i2c_master_single_r(pdev, MAG_AK09916_I2C_ADDR, reg, data);
}

ICM_20948_Status_e magWhoAmI(ICM_20948_Device_t *pdev) {
    uint8_t whoami1, whoami2;
    ICM_20948_Status_e retval;

    retval = readMagnetometer(pdev, AK09916_REG_WIA1, &whoami1);
    if (retval != ICM_20948_Stat_Ok)
        return retval;

    retval = readMagnetometer(pdev, AK09916_REG_WIA2, &whoami2);
    if (retval != ICM_20948_Stat_Ok)
        return retval;

    if ((whoami1 == (MAG_AK09916_WHO_AM_I >> 8)) &&
        (whoami2 == (MAG_AK09916_WHO_AM_I & 0xFF)))
        return ICM_20948_Stat_Ok;
    return ICM_20948_Stat_WrongID;
}

ICM_20948_Status_e startupMagnetometer(ICM_20948_Device_t *pdev, bool minimal) {
    ICM_20948_Status_e retval;
    ICM_20948_i2c_master_passthrough(pdev, false);
    ICM_20948_i2c_master_enable(pdev, true);

    // Sparkfun puts this in a resetMag() function, may be used later
    uint8_t SRST = 1;
    ICM_20948_i2c_master_single_w(pdev, MAG_AK09916_I2C_ADDR, AK09916_REG_CNTL3,
                                  &SRST);

    uint8_t tries = 0;
    while (tries < MAX_MAGNETOMETER_STARTS) {
        tries++;
        retval = magWhoAmI(pdev);
        if (retval == ICM_20948_Stat_Ok)
            break;
        ICM_20948_i2c_master_reset(pdev);
        k_sleep(K_MSEC(10));
    }
    if (tries == MAX_MAGNETOMETER_STARTS) {
        return ICM_20948_Stat_WrongID;
    }
    if (minimal)
        return ICM_20948_Stat_Ok;
    // Same as startupDefault, there is more but I am doing minimal startup
    return ICM_20948_Stat_Ok;
}

ICM_20948_Status_e startupDefault(ICM_20948_Device_t *pdev, bool minimal) {
    ICM_20948_Status_e retval;
    retval = ICM_20948_check_id(pdev);
    if (retval != ICM_20948_Stat_Ok)
        return retval;

    retval = ICM_20948_sw_reset(pdev);
    if (retval != ICM_20948_Stat_Ok)
        return retval;

    k_sleep(K_MSEC(50));

    retval = ICM_20948_sleep(pdev, false);
    if (retval != ICM_20948_Stat_Ok)
        return retval;

    retval = ICM_20948_low_power(pdev, false);
    if (retval != ICM_20948_Stat_Ok)
        return retval;

    retval = startupMagnetometer(pdev, minimal);
    if (minimal)
        return retval;
    return retval;
    // There is more to the startup but I'm doing the minimal startup only for
    // now
}
// Copying the Sparkfun begin() function
ICM_20948_Status_e setup_IMU(ICM_20948_Device_t *pdev,
                             ICM_20948_Serif_t *serif) {
#if defined(ICM_20948_USE_DMP)
    pdev->_dmp_firmware_available = true;
#else
    pdev->_dmp_firmware_available = false;
#endif
    ICM_20948_link_serif(pdev, serif);
    pdev->_firmware_loaded = false;
    pdev->_last_bank = 255;
    pdev->_last_mems_bank = 255;
    pdev->_gyroSF = 0;
    pdev->_gyroSFpll = 0;
    pdev->_enabled_Android_0 = 0;
    pdev->_enabled_Android_1 = 0;
    pdev->_enabled_Android_intr_0 = 0;
    pdev->_enabled_Android_intr_1 = 0;

    return startupDefault(pdev, pdev->_dmp_firmware_available);
}

/* DMP has a whole initialization. Sparkfun doesn't provide this in the C files.
 * This is copied from ICM_20948.cpp
 */
ICM_20948_Status_e initializeDMP(ICM_20948_Device_t *pdev) {
    ICM_20948_Status_e result = ICM_20948_Stat_Ok;
    ICM_20948_Status_e worstResult = ICM_20948_Stat_Ok;
    result = ICM_20948_i2c_controller_configure_peripheral(
        pdev, 0, MAG_AK09916_I2C_ADDR, AK09916_REG_RSV2, 10, true, true, false,
        true, true, 0);
    if (result > worstResult)
        worstResult = result;

    result = ICM_20948_i2c_controller_configure_peripheral(
        pdev, 1, MAG_AK09916_I2C_ADDR, AK09916_REG_CNTL2, 1, false, true, false,
        false, false, AK09916_mode_single);
    if (result > worstResult)
        worstResult = result;

    result = ICM_20948_set_bank(pdev, 3);
    if (result > worstResult)
        worstResult = result;
    uint8_t config = 0x04;
    result = pdev->_serif->write(ICM_ADDR, AGB3_REG_I2C_MST_ODR_CONFIG, &config,
                                 1, pdev->_serif->user);
    if (result > worstResult)
        worstResult = result;

    result = ICM_20948_set_clock_source(pdev, ICM_20948_Clock_Auto);
    if (result > worstResult)
        worstResult = result;

    result = ICM_20948_set_bank(pdev, 0);
    if (result > worstResult)
        worstResult = result;
    config = 0x40;
    result = ICM_20948_execute_w(pdev, AGB0_REG_PWR_MGMT_2, &config, 1);
    if (result > worstResult)
        worstResult = result;

    result = ICM_20948_set_sample_mode(pdev, ICM_20948_Internal_Mst,
                                       ICM_20948_Sample_Mode_Cycled);
    k_sleep(K_MSEC(1));
    if (result > worstResult)
        worstResult = result;

    result = ICM_20948_enable_FIFO(pdev, false);
    if (result > worstResult)
        worstResult = result;
    result = ICM_20948_enable_DMP(pdev, false);
    if (result > worstResult)
        worstResult = result;

    ICM_20948_fss_t fss;
    fss.a = gpm4;
    fss.g = dps2000;
    result = ICM_20948_set_full_scale(
        pdev,
        (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc |
                                        ICM_20948_Internal_Gyr),
        fss);
    if (result > worstResult)
        worstResult = result;
    result = ICM_20948_enable_dlpf(pdev, ICM_20948_Internal_Gyr, true);
    if (result > worstResult)
        worstResult = result;

    // Enable interrupt for FIFO overflow from FIFOs through INT_ENABLE_2
    // If we see this interrupt, we'll need to reset the FIFO
    // result = intEnableOverflowFIFO( 0x1F ); if (result > worstResult)
    // worstResult = result; // Enable the interrupt on all FIFOs

    // Turn off what goes into the FIFO through FIFO_EN_1, FIFO_EN_2
    // Stop the peripheral data from being written to the FIFO by writing zero
    // to FIFO_EN_1
    result = ICM_20948_set_bank(pdev, 0);
    if (result > worstResult)
        worstResult = result;
    uint8_t zero = 0;
    result = ICM_20948_execute_w(pdev, AGB0_REG_FIFO_EN_1, &zero, 1);
    if (result > worstResult)
        worstResult = result;
    // Stop the accelerometer, gyro and temperature data from being written to
    // the FIFO by writing zero to FIFO_EN_2
    result = ICM_20948_execute_w(pdev, AGB0_REG_FIFO_EN_2, &zero, 1);
    if (result > worstResult)
        worstResult = result;

    // Turn off data ready interrupt through INT_ENABLE_1
    // This is technically its own function but I only use it once
    ICM_20948_INT_enable_t en;
    result = ICM_20948_int_enable(pdev, NULL, &en);
    if (result > worstResult)
        worstResult = result;
    en.RAW_DATA_0_RDY_EN = false;
    result = ICM_20948_int_enable(pdev, &en, &en);
    if (en.RAW_DATA_0_RDY_EN != false)
        result = ICM_20948_Stat_Err;
    if (result > worstResult)
        worstResult = result;

    // Reset FIFO through FIFO_RST
    result = ICM_20948_reset_FIFO(pdev);
    if (result > worstResult)
        worstResult = result;

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
    result = ICM_20948_set_sample_rate(
        pdev, (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), mySmplrt);
    if (result > worstResult)
        worstResult = result;

    // Setup DMP start address through PRGM_STRT_ADDRH/PRGM_STRT_ADDRL
    result = ICM_20948_set_dmp_start_address(pdev, DMP_START_ADDRESS);
    if (result > worstResult)
        worstResult = result;

    // Now load the DMP firmware
    result = ICM_20948_firmware_load(pdev);
    if (result > worstResult)
        worstResult = result;

    // Write the 2 byte Firmware Start Value to ICM
    // PRGM_STRT_ADDRH/PRGM_STRT_ADDRL
    result = ICM_20948_set_dmp_start_address(pdev, DMP_START_ADDRESS);
    if (result > worstResult)
        worstResult = result;

    // Set the Hardware Fix Disable register to 0x48
    result = ICM_20948_set_bank(pdev, 0);
    if (result > worstResult)
        worstResult = result;
    uint8_t fix = 0x48;
    result = ICM_20948_execute_w(pdev, AGB0_REG_HW_FIX_DISABLE, &fix, 1);
    if (result > worstResult)
        worstResult = result;

    // Set the Single FIFO Priority Select register to 0xE4
    result = ICM_20948_set_bank(pdev, 0);
    if (result > worstResult)
        worstResult = result;
    uint8_t fifoPrio = 0xE4;
    result = ICM_20948_execute_w(pdev, AGB0_REG_SINGLE_FIFO_PRIORITY_SEL,
                                 &fifoPrio, 1);
    if (result > worstResult)
        worstResult = result;

    // Configure Accel scaling to DMP
    // The DMP scales accel raw data internally to align 1g as 2^25
    // In order to align internal accel raw data 2^25 = 1g write 0x04000000 when
    // FSR is 4g
    const unsigned char accScale[4] = {0x04, 0x00, 0x00, 0x00};
    result = inv_icm20948_write_mems(pdev, ACC_SCALE, 4, &accScale[0]);
    if (result > worstResult)
        worstResult = result;
    // Write accScale to ACC_SCALE DMP register
    // In order to output hardware unit data as configured FSR write 0x00040000
    // when FSR is 4g
    const unsigned char accScale2[4] = {0x00, 0x04, 0x00, 0x00};
    result = inv_icm20948_write_mems(pdev, ACC_SCALE2, 4, &accScale2[0]);
    if (result > worstResult)
        worstResult = result;
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
    result =
        inv_icm20948_write_mems(pdev, CPASS_MTX_00, 4, &mountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;
    result =
        inv_icm20948_write_mems(pdev, CPASS_MTX_01, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result =
        inv_icm20948_write_mems(pdev, CPASS_MTX_02, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result =
        inv_icm20948_write_mems(pdev, CPASS_MTX_10, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, CPASS_MTX_11, 4,
                                     &mountMultiplierMinus[0]);
    if (result > worstResult)
        worstResult = result;
    result =
        inv_icm20948_write_mems(pdev, CPASS_MTX_12, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result =
        inv_icm20948_write_mems(pdev, CPASS_MTX_20, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result =
        inv_icm20948_write_mems(pdev, CPASS_MTX_21, 4, &mountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, CPASS_MTX_22, 4,
                                     &mountMultiplierMinus[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the B2S Mounting Matrix
    const unsigned char b2sMountMultiplierZero[4] = {0x00, 0x00, 0x00, 0x00};
    const unsigned char b2sMountMultiplierPlus[4] = {
        0x40, 0x00, 0x00, 0x00}; // Value taken from InvenSense Nucleo example
    result = inv_icm20948_write_mems(pdev, B2S_MTX_00, 4,
                                     &b2sMountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_01, 4,
                                     &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_02, 4,
                                     &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_10, 4,
                                     &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_11, 4,
                                     &b2sMountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_12, 4,
                                     &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_20, 4,
                                     &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_21, 4,
                                     &b2sMountMultiplierZero[0]);
    if (result > worstResult)
        worstResult = result;
    result = inv_icm20948_write_mems(pdev, B2S_MTX_22, 4,
                                     &b2sMountMultiplierPlus[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the DMP Gyro Scaling Factor
    // @param[in] gyro_div Value written to GYRO_SMPLRT_DIV register, where
    //            0=1125Hz sample rate, 1=562.5Hz sample rate, ... 4=225Hz
    //            sample rate, ... 10=102.2727Hz sample rate, ... etc.
    // @param[in] gyro_level 0=250 dps, 1=500 dps, 2=1000 dps, 3=2000 dps
    result = inv_icm20948_set_gyro_sf(pdev, 4, 3);
    if (result > worstResult)
        worstResult = result;
    // 19 = 55Hz (see above), 3 = 2000dps (see above)

    // Configure the Gyro full scale
    // 2000dps : 2^28
    // 1000dps : 2^27
    //  500dps : 2^26
    //  250dps : 2^25
    const unsigned char gyroFullScale[4] = {0x10, 0x00, 0x00,
                                            0x00}; // 2000dps : 2^28
    result =
        inv_icm20948_write_mems(pdev, GYRO_FULLSCALE, 4, &gyroFullScale[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel Only Gain: 15252014 (225Hz) 30504029 (112Hz) 61117001
    // (56Hz)
    // const unsigned char accelOnlyGain[4] = {0x03, 0xA4, 0x92, 0x49}; // 56Hz
    const unsigned char accelOnlyGain[4] = {0x00, 0xE8, 0xBA, 0x2E}; // 225Hz
    // const unsigned char accelOnlyGain[4] = {0x01, 0xD1, 0x74, 0x5D}; // 112Hz
    result =
        inv_icm20948_write_mems(pdev, ACCEL_ONLY_GAIN, 4, &accelOnlyGain[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel Alpha Var: 1026019965 (225Hz) 977872018 (112Hz)
    // 882002213 (56Hz)
    // const unsigned char accelAlphaVar[4] = {0x34, 0x92, 0x49, 0x25}; // 56Hz
    const unsigned char accelAlphaVar[4] = {0x3D, 0x27, 0xD2, 0x7D}; // 225Hz
    // const unsigned char accelAlphaVar[4] = {0x3A, 0x49, 0x24, 0x92}; // 112Hz
    result =
        inv_icm20948_write_mems(pdev, ACCEL_ALPHA_VAR, 4, &accelAlphaVar[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel A Var: 47721859 (225Hz) 95869806 (112Hz) 191739611
    // (56Hz)
    // const unsigned char accelAVar[4] = {0x0B, 0x6D, 0xB6, 0xDB}; // 56Hz
    const unsigned char accelAVar[4] = {0x02, 0xD8, 0x2D, 0x83}; // 225Hz
    // const unsigned char accelAVar[4] = {0x05, 0xB6, 0xDB, 0x6E}; // 112Hz
    result = inv_icm20948_write_mems(pdev, ACCEL_A_VAR, 4, &accelAVar[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Accel Cal Rate
    const unsigned char accelCalRate[4] = {
        0x00, 0x00}; // Value taken from InvenSense Nucleo example
    result = inv_icm20948_write_mems(pdev, ACCEL_CAL_RATE, 2, &accelCalRate[0]);
    if (result > worstResult)
        worstResult = result;

    // Configure the Compass Time Buffer. The I2C Master ODR Configuration (see
    // above) sets the magnetometer read rate to 68.75Hz. Let's set the Compass
    // Time Buffer to 69 (Hz).
    const unsigned char compassRate[2] = {0x00, 0x45}; // 69Hz
    result =
        inv_icm20948_write_mems(pdev, CPASS_TIME_BUFFER, 2, &compassRate[0]);
    if (result > worstResult)
        worstResult = result;

    // Enable DMP interrupt
    // This would be the most efficient way of getting the DMP data, instead of
    // polling the FIFO

    // result |= ICM_20948_int_enable(pdev, NULL, &en);
    // en.DMP_INT1_EN = true;
    // result |= ICM_20948_int_enable(pdev, &en, &en);

    return worstResult;
}
