#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include "icm20948/icm20948.h"

#define DEFAULT_STACK 512

/*
 *  ICM20948 Necessary Components
 */

// Data buffer
uint8_t icm20948_fifo_buffer[FIFO_SIZE];

void icm20948_isr(const void *unused) {
    // TODO: Check if right pin was activated here
    k_sem_give(&icm20948_ready);
}

K_THREAD_STACK_DEFINE(gyro_stack_area, DEFAULT_STACK);
struct k_thread gyro_thread_data;

extern void read_icm20948_data(void *dev, void *unused2, void *unused3) {
    const struct device *const i2c_dev = (const struct device *const)dev;
    while (1) {
        printk("Semaphore reset\n");
        if (!k_sem_take(&icm20948_ready, K_FOREVER)) {
            uint16_t bytes_ready = 0;
            icm20948_get_fifo_count(i2c_dev, &bytes_ready);
            printk("FIFO count: %d\n", bytes_ready);
            icm20948_read_fifo(i2c_dev, icm20948_fifo_buffer, bytes_ready);
            uint8_t dummy;
            icm20948_readregister(i2c_dev, ICM20948_INT_STATUS, &dummy);
        }
    }
}

int main(void) {

    if (usb_enable(NULL))
        return 0;

    uint32_t dtr = 0;
    const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    while (!dtr) {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        /* Give CPU resources to low priority threads. */
        k_sleep(K_MSEC(100));
    }

    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (i2c_dev == NULL || !device_is_ready(i2c_dev))
        printf("Could not get I2C device");

    uint32_t dev_config = I2C_SPEED_FAST | I2C_MODE_CONTROLLER;
    i2c_configure(i2c_dev, dev_config);

    // This was just to check that I was interfacing properly
    uint8_t test;
    int err = icm20948_readregister(i2c_dev, ICM20948_WHO_AM_I, &test);
    printf("Error %d: %X\n", err, test);

    // Doesn't actually configure the IMU although maybe it should. Sets the ISR
    // to use and enables GPIO interrupts on the microcontroller
    icm20948_use_interrupts(&icm20948_isr);

    // Set up the semaphore to block the thread until the interrupt occurs
    printf("Semaphore status: %d\n", k_sem_take(&icm20948_ready, K_FOREVER));
    printf("Here\n");
    // Enable the DMP and FIFO on the IMU
    icm20948_userconfig(i2c_dev, DMP_EN | FIFO_EN);

    // Enables DMP interrupts
    icm20948_set_int(i2c_dev, DMP_INT1_EN);
    char buf;
    icm20948_readregister(i2c_dev, ICM20948_INT_STATUS, &buf);
    
    printf("Interrupt registers: %X\n", buf);

    // Create and start a thread that constantly tries to read from the IMU
    k_tid_t icm20948_get_gyro = k_thread_create(
        &gyro_thread_data, gyro_stack_area,
        K_THREAD_STACK_SIZEOF(gyro_stack_area), &read_icm20948_data,
        (void *)&i2c_dev, NULL, NULL, 7, 0, K_NO_WAIT);
    k_thread_join(icm20948_get_gyro, K_FOREVER);
    return 0;
}
