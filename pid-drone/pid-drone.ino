#include <stdio.h>
#include <stdint.h>
#include "mbed.h"

#define ONESHOT
#define SEQ_SIZE 17
#define TELEMETRY 0
#define POTENTIOMETER A0

uint16_t command;
uint16_t throttle;
uint16_t oneshot[2];

uint16_t seq[SEQ_SIZE];
unsigned int prevTime;
unsigned int curTime;

/**
 * Takes a command (currently in the form 0000TSSSSSSSSSSS), and calculates a
 * checksum based on the 12 set bits. Modifies the command in place.
 */
void CalcChecksum(uint16_t *command) {
    uint16_t temp = *command;
    temp = ((temp ^ (temp >> 4)) ^ (temp >> 8)) & 0x0F;
    *command = (*command << 4) | temp;
}

void CreateSequence(uint16_t command, uint16_t sequence[17]) {
    command = (command << 1) | TELEMETRY;
    CalcChecksum(&command);
    int j;
    // Send LSB last
    for (int i = 0; i < 16; i++) {
        sequence[i] = (5 * (1 << ((command >> (15 - i)) & 1))) | (1 << 15);
    }
    sequence[16] = 0;
}

void setup() {
    Serial.begin(9600);
    command = 48;
    CreateSequence(command, seq);
    oneshot[0] = 2000;
    oneshot[1] = 0;
    pinMode(P1_11, OUTPUT);
    pinMode(A0, INPUT);
    digitalWrite(P1_11, LOW);
    NRF_PWM0->PSEL.OUT[0] = P1_11;
    NRF_PWM0->ENABLE = 1;
    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up;
#ifndef ONESHOT
    NRF_PWM0->COUNTERTOP = 13;
    NRF_PWM0->SEQ[0].CNT = SEQ_SIZE;
    NRF_PWM0->SEQ[0].PTR = (uint32_t)&seq;
#else
    NRF_PWM0->COUNTERTOP = 5000;
    NRF_PWM0->SEQ[0].CNT = 2;
    NRF_PWM0->SEQ[0].PTR = (uint32_t)&oneshot;
#endif
    NRF_PWM0->LOOP = 0;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
    NRF_PWM0->DECODER = PWM_DECODER_LOAD_Common |
                        (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
    NRF_PWM0->TASKS_SEQSTART[0] = 1;
    prevTime = millis();
}

void loop() {
    curTime = millis();
    if (curTime - prevTime > 100) {
        command = analogRead(A0) * 2;
        if (command < 48)
            command = 48;
            // Serial.println(command);
#ifndef ONESHOT
        CreateSequence(command, seq);
        NRF_PWM0->SEQ[0].PTR = (uint32_t)&seq;
#else
        oneshot[0] = command / 1023 * 2000 + 2000;
        NRF_PWM0->SEQ[0].PTR = (uint32_t)&oneshot;
#endif
        // for (int i = 0; i < 16; i++) {
        //     Serial.print('\t');
        //     Serial.println(seq[i], BIN);
        // }
        NRF_PWM0->TASKS_SEQSTART[0] = 1;
        prevTime = curTime;
    }
}
