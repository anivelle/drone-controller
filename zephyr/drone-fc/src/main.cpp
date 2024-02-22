#include <Arduino.h>
#include <Wire.h>
#include "vl53lx_class.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "nrf52840.h"

VL53LX sensor(&Wire, D15);
uint8_t dataReady, status;
uint16_t prevTime;
VL53LX_MultiRangingData_t rangeData;

int main(void) {

    Serial.begin(115200);
    // This causes the PWM output to wait until I open a serial monitor,
    // which may or may not be desirable
    while (!Serial) {
        ;
    }
    Wire.begin();
    sensor.begin();
    sensor.VL53LX_Off();
    sensor.InitSensor(0x12);

    sensor.VL53LX_StartMeasurement();
    Serial.println("Done initializing");
    prevTime = millis();
    while (1) {
    status = sensor.VL53LX_GetMeasurementDataReady(&dataReady);
    if (!status && (dataReady != 0)) {
        sensor.VL53LX_GetMultiRangingData(&rangeData);
        for (int i = 0; i < rangeData.NumberOfObjectsFound; i++) {
            char text[25];
            sprintf(text, "Found %1d: Distance %4d", i,
                    rangeData.RangeData[i].RangeMilliMeter);
            Serial.println(text);
        }
        Serial.println();
        // This is an important line (for some reason)!
        status = sensor.VL53LX_ClearInterruptAndStartMeasurement();
    }

    }

    return 0;
}
