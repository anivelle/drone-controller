#include <cstdio>
#include <stdint.h>
#include "mbed.h"
#include "PWM.h"
#include "DShot.h"

#define TELEMETRY 0
#define POTENTIOMETER A0

uint16_t commands[4];
uint16_t throttle;

uint16_t seq[SEQ_SIZE];

uint8_t dataReady;
int status;
uint8_t counter;
int prevTime, curTime;
void setup() {
    // distanceSensor.VL53L4CX_WaitDeviceBooted();
    // distanceSensor.VL53L4CX_DataInit();
    Serial.begin(115200);
    // This causes the PWM output to wait until I open a serial monitor,
    // which may or may not be desirable
    while (!Serial) {
        ;
    }

    commands[0] = 48;
    commands[1] = 48;
    commands[2] = 48;
    commands[3] = 48;
    CreateSequence(commands, seq);
    PWM_AddPins(0, P1_12);
    PWM_AddPins(1, P1_15);
    PWM_AddPins(2, P1_13);
    PWM_AddPins(3, P1_14);
    pinMode(POTENTIOMETER, INPUT);
    PWM_Init((uint32_t)&seq);

    prevTime = micros();
    counter = 0;
}

void loop() {
    uint16_t command;
    if (counter == 17) 
    {
      delay(260);
      counter++;
    }
    if (counter > 17) {
        int temp = analogRead(POTENTIOMETER) * 2;
        if (abs(temp - command) > 50)
            command = temp;
        if (command < 48)
            command = 48;
        for (int i = 0; i < 4; i++) {
            commands[i] = command;
        }
    } else if (counter > 10 && counter < 17) {
        Serial.print("Updating after ");
        Serial.println(curTime - prevTime);
        commands[0] = 1;
        commands[1] = 2;
        commands[2] = 3;
        commands[3] = 4;
    }
    curTime = micros();
    if (curTime - prevTime > 50) {
        Serial.print("Sending after ");
        Serial.println(curTime - prevTime);
        CreateSequence(commands, seq);
        PWM_SendCommand((uint32_t)&seq); // Test this later to see if I have to
                                         // update the address
        prevTime = micros();
        if (counter < 255)
            counter++;
    }
}
