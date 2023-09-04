#include <stdint.h>
#include <Wire.h>
#include "BNO055.h"

BNO055 gyro(&Wire);
int err;

void setup() {
    Serial.begin(115200);
    // Very important for serial to work properly (inside setup, at least)
    while (!Serial) {
    }
    Wire.begin();
    DEVICE_I2C_ASYNCH;
    // Configuration stuff here
    delay(850);
    err = gyro.writeRegister(OPR_MODE, AMG);
    delay(30);
    if (err != 0) {
        Serial.println("ERROR: Did not write properly");
        Serial.println(err);
    }
    err = gyro.readRegister(OPR_MODE);
    if (err != AMG) {
        Serial.println("ERROR: Operation mode not set");
        Serial.println(err);
    } else {
        Serial.println("We're good");
    }
}

void loop() {
    // uint8_t ready = gyro.readRegister(INT_STA);
    // uint16_t acc[3];
    // if (ready & 1) {
    //     for (int i = 0; i < 3; i++) {
    //         //acc[i] = gyro.readAcc((Axis)i);
    //         // Serial.println(acc[i]);
    //     }
    // }
}
