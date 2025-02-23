#include <zephyr/kernel.h>
// #include <stdio.h>
// #include <zephyr/usb/usb_device.h>
// #include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include "icm20948/icm20948.h"

#define DEFAULT_STACK 512

/*
 *  ICM20948 Necessary Components
 */
BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
             "Console device is not ACM CDC UART device");
// Data buffer
uint8_t icm20948_fifo_buffer[FIFO_SIZE];

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
        icm20948_readregister(i2c_dev, ICM20948_DATA_RDY_STATUS, &data_ready);
        // if (!k_sem_take(&icm20948_ready, K_FOREVER)) {
        if (data_ready) {
            uint16_t bytes_ready = 0;
            icm20948_get_fifo_count(i2c_dev, &bytes_ready);
            printk("FIFO count: %d\n", bytes_ready);
            icm20948_read_fifo(i2c_dev, icm20948_fifo_buffer, bytes_ready);
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

    // This was just to check that I was interfacing properly
    uint8_t test;
    int err = icm20948_readregister(i2c_dev, ICM20948_WHO_AM_I, &test);
    printk("Error %d: %X\n", err, test);

    // k_sem_give(&icm20948_ready);
    // Set up the semaphore to block the thread until the interrupt occurs
    // printk("Semaphore status: %d\n", k_sem_take(&icm20948_ready, K_FOREVER));
    // Wake up the ICM20948 from sleep mode
    uint8_t pwr_mode;
    icm20948_readregister(i2c_dev, ICM20948_PWR_MGMT_1, &pwr_mode);
    pwr_mode &= ~SLEEP;
    icm20948_setregister(i2c_dev, ICM20948_PWR_MGMT_1, pwr_mode);

    icm20948_readregister(i2c_dev, ICM20948_PWR_MGMT_1, &pwr_mode);
    printk("Power mode: %X\n", pwr_mode);

    // Enable the DMP and FIFO on the IMU
    icm20948_userconfig(i2c_dev, DMP_EN | FIFO_EN);
    icm20948_readregister(i2c_dev, ICM20948_USER_CTRL, &test);
    printk("User config: %X\n", test);

    // printk("User config done\n");
    while (1) {
        // printk("Semaphore reset\n");
        uint8_t data_ready = 0;
        icm20948_readregister(i2c_dev, ICM20948_DATA_RDY_STATUS, &data_ready);
        // if (!k_sem_take(&icm20948_ready, K_FOREVER)) {
        if (data_ready) {
            uint16_t bytes_ready = 0;
            icm20948_get_fifo_count(i2c_dev, &bytes_ready);
            printk("FIFO count: %d\n", bytes_ready);
            icm20948_read_fifo(i2c_dev, icm20948_fifo_buffer, bytes_ready);
            // uint8_t dummy;
            // icm20948_readregister(i2c_dev, ICM20948_INT_STATUS, &dummy);
        }
        k_sleep(K_MSEC(10));
    }

    // Enables DMP interrupts
    // icm20948_set_int(i2c_dev, DMP_INT1_EN);
    // char buf;
    // icm20948_readregister(i2c_dev, ICM20948_INT_STATUS, &buf);

    // printk("Interrupt registers: %X\n", buf);

    // Create and start a thread that constantly tries to read from the IMU
    // k_tid_t icm20948_get_gyro = k_thread_create(
    //     &gyro_thread_data, gyro_stack_area,
    //     K_THREAD_STACK_SIZEOF(gyro_stack_area), &read_icm20948_data,
    //     (void *)&i2c_dev, NULL, NULL, 7, 0, K_NO_WAIT);
    // k_thread_join(icm20948_get_gyro, K_FOREVER);
    return 0;
}
