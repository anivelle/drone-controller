#include <stdio.h>
#include <stdint.h>
#include "mbed.h"

#define SEQ_SIZE 17
#define TELEMETRY 0
uint16_t command;

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
    for (int i = 15; i > 0; i--) {
        sequence[i] = (5 * (1 << ((command >> i) & 1))) | (1 << 15);
    }
    sequence[16] = 1 << 15;
}

void setup() {
    Serial.begin(9600);
    uint16_t seq[SEQ_SIZE];
    uint16_t command = 1046;
    CreateSequence(command, seq);

    NRF_TIMER2->INTENSET = 0x000F0000;
    pinMode(P1_11, OUTPUT);
    digitalWrite(P1_11, LOW);
    NRF_PWM0->PSEL.OUT[0] = 11 | (1 << 5);
    NRF_PWM0->ENABLE = 1;
    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up;
    NRF_PWM0->COUNTERTOP = 13;
    NRF_PWM0->LOOP = 0;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
    NRF_PWM0->DECODER = PWM_DECODER_LOAD_Common |
                        (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM0->SEQ[0].PTR = (uint32_t)&seq;
    NRF_PWM0->SEQ[0].CNT = SEQ_SIZE;
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
    NRF_PWM0->TASKS_SEQSTART[0] = 1;
}

void loop() {}
