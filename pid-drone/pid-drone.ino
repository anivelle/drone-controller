#include <cstdio>
#include <stdint.h>
#include "mbed.h"
#include "VL53L4CX.h"
#include "PWM.h"
#include "DShot.h"
#include "Wire.h"
#include "BNO055.h"

uint16_t seq[SEQ_SIZE * 4];
uint16_t commands[4];

VL53L4CX distanceSensor(&Wire, A1);
BNO055 gyro(&Wire);

uint8_t dataReady;
int status;
void setup() {
    // distanceSensor.VL53L4CX_WaitDeviceBooted();
    // distanceSensor.VL53L4CX_DataInit();
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Wire.begin();
    distanceSensor.begin();
    distanceSensor.VL53L4CX_Off();
    distanceSensor.InitSensor(0x12);
    commands[0] = 1046;
    commands[1] = 500;
    commands[2] = 1500;
    commands[3] = 2000;
    CreateSequence(commands, seq);

    Serial.println("Initializing");
    PWM_AddPins(0, P1_11);
    PWM_AddPins(1, P1_12);
    PWM_AddPins(2, P1_13);
    PWM_AddPins(3, P1_14);
    PWM_Init(SEQ_SIZE, seq);
    PWM_SendCommand();
    distanceSensor.VL53L4CX_StartMeasurement();
    gyro.writeRegister(OPR_MODE, AMG);
    Serial.println("Done initializing");
}

void loop() {
    VL53L4CX_MultiRangingData_t rangeData;
    dataReady = 0;
    status = distanceSensor.VL53L4CX_GetMeasurementDataReady(&dataReady);
    if (!status && (dataReady != 0)) {
        distanceSensor.VL53L4CX_GetMultiRangingData(&rangeData);
        for (int i = 0; i < rangeData.NumberOfObjectsFound; i++) {
            char text[23];
            sprintf(text, "Found %1d: Distance %4d", i,
                    rangeData.RangeData[i].RangeMilliMeter);
            Serial.println(text);
        }
        Serial.println();
        // This is an important line!
        status = distanceSensor.VL53L4CX_ClearInterruptAndStartMeasurement();
    }
}
