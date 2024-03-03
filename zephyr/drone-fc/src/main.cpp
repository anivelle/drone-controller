#include <Arduino.h>
#include <Wire.h>
#include "vl53lx_class.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include "nrf52840.h"

VL53LX sensor(&Wire, D15);
uint8_t dataReady, status;
uint16_t prevTime;
VL53LX_MultiRangingData_t rangeData;

int main(void) {

    if (usb_enable(NULL))
      return 0;

    // Serial.begin(115200);

    // This causes the PWM output to wait until I open a serial monitor,
    // which may or may not be desirable
    
    uint32_t dtr = 0;
    const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    while (!dtr) {
      uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
      /* Give CPU resources to low priority threads. */
      k_sleep(K_MSEC(100));
    }

    Wire.begin();
    sensor.begin();
    sensor.VL53LX_Off();
    sensor.InitSensor(0x12);

    sensor.VL53LX_StartMeasurement();
    printk("Done initializing\n");
    prevTime = millis();
    while (1) {
    status = sensor.VL53LX_GetMeasurementDataReady(&dataReady);
    if (!status && (dataReady != 0)) {
        sensor.VL53LX_GetMultiRangingData(&rangeData);
        for (int i = 0; i < rangeData.NumberOfObjectsFound; i++) {
            char text[25];
            sprintf(text, "Found %1d: Distance %4d", i,
                    rangeData.RangeData[i].RangeMilliMeter);
            printk("%s\n", text);
        }
        printk("\n");
        // This is an important line (for some reason)!
        status = sensor.VL53LX_ClearInterruptAndStartMeasurement();
    }

    }

    return 0;
}
