#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include "icm20948/icm20948.h"

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

    ;
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (i2c_dev == NULL || !device_is_ready(i2c_dev))
        printf("Could not get I2C device");

    uint32_t dev_config = I2C_SPEED_FAST | I2C_MODE_CONTROLLER;
    i2c_configure(i2c_dev, dev_config);
    struct i2c_msg msgs;

    uint8_t test;
    int err = icm20948_readregister(i2c_dev, ICM20948_WHO_AM_I, &test);
    printf("Error %d: %X\n", err, test);
    return 0;
}
