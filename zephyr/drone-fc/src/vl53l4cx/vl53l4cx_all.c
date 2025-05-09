#include "vl53l4cx_class.h"

void VL53L4CX_init_device(VL53L4CX_Dev_t *myDevice, serif_t *i2c,
                          const struct device *port, int xshut_pin) {
    memset((void *)myDevice, 0x0, sizeof(VL53L4CX_Dev_t));
    myDevice->I2cHandle = i2c;
    myDevice->I2cDevAddr = VL53L4CX_DEFAULT_DEVICE_ADDRESS;
    myDevice->port = port;
    myDevice->xshut = xshut_pin;
}

int begin(VL53L4CX_Dev_t *pdev) {
    if (pdev->xshut >= 0) {
        gpio_pin_configure(pdev->port, pdev->xshut, GPIO_OUTPUT_ACTIVE);
        // gpio_pin_set(pdev->port, pdev->xshut, 0);
    }
    return 0;
}

int end(VL53L4CX_Dev_t *pdev) {
    if (pdev->xshut >= 0) {
        gpio_pin_configure(pdev->port, pdev->xshut, GPIO_INPUT);
    }
    return 0;
}

VL53L4CX_Error InitSensor(VL53L4CX_Dev_t *pdev, uint8_t address) {
    VL53L4CX_Error status = VL53L4CX_ERROR_NONE;
    VL53L4CX_Off(pdev);
    VL53L4CX_On(pdev);

    status = VL53L4CX_SetDeviceAddress(pdev, address);

    if (status == VL53L4CX_ERROR_NONE) {
        status = VL53L4CX_WaitDeviceBooted(pdev);
    }

    if (status == VL53L4CX_ERROR_NONE) {
        status = VL53L4CX_DataInit(pdev);
    }

    return status;
}

void setI2cDevice(VL53L4CX_Dev_t *myDevice, serif_t *i2c) {
    myDevice->I2cHandle = i2c;
}

void VL53L4CX_On(VL53L4CX_Dev_t *pdev) {
    if (pdev->xshut >= 0) {
        gpio_pin_set(pdev->port, pdev->xshut, 1);
    }
    k_sleep(K_MSEC(10));
}

void VL53L4CX_Off(VL53L4CX_Dev_t *pdev) {
    if (pdev->xshut >= 0) {
        gpio_pin_set(pdev->port, pdev->xshut, 0);
    }
    k_sleep(K_MSEC(10));
}



