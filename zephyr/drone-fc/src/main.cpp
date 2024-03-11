#include <zephyr/kernel.h>
#include <Arduino.h>
#include <Wire.h>
#include "icm20948/icm20948.h"
#include "vl53lx/vl53lx_class.h"
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/i2c.h>

VL53LX sensor_vl53lx_sat(&Wire, D15);

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

    Wire.begin();
    // Configure VL53LX satellite component.
    sensor_vl53lx_sat.begin();

    // Switch off VL53LX satellite component.
    sensor_vl53lx_sat.VL53LX_Off();

    // Initialize VL53LX satellite component.
    sensor_vl53lx_sat.InitSensor(0x12);

    // Start Measurements
    sensor_vl53lx_sat.VL53LX_StartMeasurement();

    while (1) {
        VL53LX_MultiRangingData_t MultiRangingData;
        VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
        uint8_t NewDataReady = 0;
        int no_of_object_found = 0, j;
        char report[64];
        int status;

        do {
            status =
                sensor_vl53lx_sat.VL53LX_GetMeasurementDataReady(&NewDataReady);
        } while (!NewDataReady);

        if ((!status) && (NewDataReady != 0)) {
            status =
                sensor_vl53lx_sat.VL53LX_GetMultiRangingData(pMultiRangingData);
            no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
            snprintf(report, sizeof(report),
                     "VL53LX Satellite: Count=%d, #Objs=%1d ",
                     pMultiRangingData->StreamCount, no_of_object_found);
            printk("%s", report);
            for (j = 0; j < no_of_object_found; j++) {
                if (j != 0)
                    printk("\r\n                               ");
                printk("status=");
                printk("%u", pMultiRangingData->RangeData[j].RangeStatus);
                printk(", D=");
                printk("%u", pMultiRangingData->RangeData[j].RangeMilliMeter);
                printk("mm");
                printk(", Signal=");
                printk("%f",
                       pMultiRangingData->RangeData[j].SignalRateRtnMegaCps /
                           65536.0);
                printk(" Mcps, Ambient=");
                printk("%f",
                       pMultiRangingData->RangeData[j].AmbientRateRtnMegaCps /
                           65536.0);
                printk(" Mcps");
            }
            printk("\n");
            if (status == 0) {
                status = sensor_vl53lx_sat
                             .VL53LX_ClearInterruptAndStartMeasurement();
            }
        }
    }
    return 0;
}
