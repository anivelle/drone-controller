#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/mem_manage.h>
// #include "icm20948/icm20948.h"
#include "sparkfun_icm20948/ICM_20948_C.h" // Thank god for the Sparkfun library
#include <string.h>

#define DEFAULT_STACK 512
#define ICM_ADDR ICM_20948_I2C_ADDR_AD1

/*
 *  ICM20948 Necessary Components
 */
BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
             "Console device is not ACM CDC UART device");
// Data buffer
// uint8_t icm20948_fifo_buffer[FIFO_SIZE];

// void icm20948_isr(const void *unused) {
//     // TODO: Check if right pin was activated here
//     k_sem_give(&icm20948_ready);
// }

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

ICM_20948_Status_e write(uint8_t reg, uint8_t *data, uint32_t len, void *user) {
    const struct device *const i2c_dev = (const struct device *const)user;
    uint8_t reg_data[len + 1];
    reg_data[0] = reg;
    memcpy(&reg_data[1], data, len);
    if (!i2c_write(i2c_dev,  reg_data, len + 1, ICM_ADDR))
        return ICM_20948_Stat_Ok;
    return ICM_20948_Stat_Err;
}

ICM_20948_Status_e read(uint8_t reg, uint8_t *buff, uint32_t len, void *user) {
    const struct device *const i2c_dev = (const struct device *const)user;
    if (!i2c_burst_read(i2c_dev, ICM_ADDR, reg, buff, len))
        return ICM_20948_Stat_Ok;
    return ICM_20948_Stat_Err;
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
    printk("Error %d: %X\n", err, test);

    ICM_20948_sw_reset(&pdev);
    k_sleep(K_MSEC(250));
    printf("Loading firmware %d\n", ICM_20948_firmware_load(&pdev));
    
    inv_icm20948_enable_dmp_sensor(&pdev, INV_ICM20948_SENSOR_ORIENTATION, true);
    inv_icm20948_set_dmp_sensor_period(&pdev, DMP_ODR_Reg_Quat9, 0);
    printf("Enabling DMP %d\n", ICM_20948_enable_DMP(&pdev, true));
    
    
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
        k_sleep(K_MSEC(10));
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
