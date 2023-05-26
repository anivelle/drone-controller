#include <Arduino.h>
#line 1 "/home/alex/pid-drone/pid-drone.ino"
#include <stdio.h>
#include <stdint.h>
#include "mbed.h"

uint16_t command;

/**
 * Takes a command (currently in the form 0000TSSSSSSSSSSS), and calculates a
 * checksum based on the 12 set bits. Modifies the command in place.
 */
#line 11 "/home/alex/pid-drone/pid-drone.ino"
void CalcChecksum(uint16_t *command);
#line 21 "/home/alex/pid-drone/pid-drone.ino"
void setup();
#line 29 "/home/alex/pid-drone/pid-drone.ino"
void loop();
#line 11 "/home/alex/pid-drone/pid-drone.ino"
void CalcChecksum(uint16_t *command) {
    uint16_t temp = *command;
    temp = (temp ^ (temp >> 4) ^ (temp >> 8)) & 0x0F;
    *command |= temp << 12;
}

pwmout_t pwm;
unsigned long curTime;
int duty = 0;

void setup() {
    Serial.begin(9600);
    pwmout_init(&pwm, P1_11);
    pwmout_period_us(&pwm, 1);
    pwmout_write(&pwm, 0.650);
    pin_mode(P1_12, INPUT);
}

void loop() {
    curTime = micros();
    PinStatus pin = digitalRead(P1_12);
    if (!duty)
        pwmout_write(&pwm, 0.650);
    else
        pwmout_write(&pwm, 0.325);

    Serial.print(curTime);
    Serial.print(", ");
    Serial.print(pin);
    delayMicroseconds(3);
}

