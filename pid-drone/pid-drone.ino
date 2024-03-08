#include <cstdio>
#include <stdint.h>
#include "mbed.h"
#include "VL53L4CX.h"
#include "PWM.h"
#include "DShot.h"
#include "Wire.h"
// Not using the BNO055 anymore
// #include "BNO055.h" 

#define TELEMETRY 0
#define POTENTIOMETER A0

uint16_t commands[4];
uint16_t throttle;

uint16_t seq[SEQ_SIZE];

VL53L4CX distanceSensor(&Wire, A1);
//BNO055 gyro(&Wire);

uint8_t dataReady;
int status;
uint16_t prevTime, curTime;

void setup() {
    // distanceSensor.VL53L4CX_WaitDeviceBooted();
    // distanceSensor.VL53L4CX_DataInit();
    Serial.begin(115200);
    // This causes the PWM output to wait until I open a serial monitor, 
    // which may or may not be desirable
    while (!Serial) {
        ;
    }
    Wire.begin();
    distanceSensor.begin();
    distanceSensor.VL53L4CX_Off();
    distanceSensor.InitSensor(0x12);

    distanceSensor.VL53L4CX_StartMeasurement();
    //gyro.writeRegister(OPR_MODE, AMG);
  
    commands[0] = 0;
    commands[1] = 0;
    commands[2] = 0;
    commands[3] = 0;
    CreateSequence(commands, seq);
    PWM_AddPins(0, P1_12);
    PWM_AddPins(1, P1_15);
    PWM_AddPins(2, P1_13);
    PWM_AddPins(3, P1_14);
    pinMode(POTENTIOMETER, INPUT);
    PWM_Init((uint32_t)&seq);
    
    Serial.println("Done initializing");
    prevTime = millis();
}

void loop() {
    curTime = micros();
    uint16_t command;
    if (counter > 10) {
        command = analogRead(POTENTIOMETER) * 2;
        if (command < 48)
            command = 48;
        for (int i = 0; i < 4; i++) {
            commands[i] = command;
        }
        // commands[0] = (commands[0] + 1) % 2048;
    }
    if (curTime - prevTime > 200) {
      CreateSequence(commands, seq);
      PWM_SendCommand((uint32_t)&seq); // Test this later to see if I have to update the address
      prevTime = curTime;
        if (counter < 255)
            counter++;
    }
  
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
        // This is an important line (for some reason)!
        status = distanceSensor.VL53L4CX_ClearInterruptAndStartMeasurement();
    }
}
