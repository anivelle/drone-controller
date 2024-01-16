#include <stdio.h>
#include <stdint.h>
#include "mbed.h"
#include "DShot.h"
#include "PWM.h"

#define TELEMETRY 0
#define POTENTIOMETER A0

uint16_t commands[4];
uint16_t throttle;

uint16_t seq[SEQ_SIZE];
uint16_t seqOld[SEQ_SIZE / 4];
unsigned int prevTime;
unsigned int curTime;
uint8_t counter = 0;

/**
 * Takes a command (currently in the form 0000TSSSSSSSSSSS), and calculates a
 * checksum based on the 12 set bits. Modifies the command in place.
 */
void CalcChecksumOld(uint16_t *command) {
    uint16_t temp = *command;
    temp = ((temp ^ (temp >> 4)) ^ (temp >> 8)) & 0x0F;
    *command = (*command << 4) | temp;
}

void CreateSequenceOld(uint16_t command, uint16_t sequence[17]) {
    uint16_t comm = (command << 1) | TELEMETRY;
    CalcChecksumOld(&comm);
    int j;
    // Send LSB last
    for (int i = 0; i < 16; i++) {
        sequence[i] = (5 * (1 << ((comm >> (15 - i)) & 1))) | (1 << 15);
    }
    sequence[16] = 0 | (1 << 15);
}

void setup() {
    // Serial.begin(9600);
    // while (!Serial) {
    // }
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
    // NRF_PWM0->PSEL.OUT[0] = P1_12;
    // NRF_PWM0->ENABLE = 1;
    // NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up;
    // NRF_PWM0->COUNTERTOP = 13;
    // NRF_PWM0->SEQ[0].CNT = SEQ_SIZE;
    // NRF_PWM0->SEQ[0].PTR = (uint32_t)&seq;
    // NRF_PWM0->LOOP = 0;
    // NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
    // NRF_PWM0->DECODER = PWM_DECODER_LOAD_Common |
    //                     (PWM_DECODER_MODE_RefreshCount <<
    //                     PWM_DECODER_MODE_Pos);
    // NRF_PWM0->SEQ[0].REFRESH = 0;
    // NRF_PWM0->SEQ[0].ENDDELAY = 0;
    // NRF_PWM0->TASKS_SEQSTART[0] = 1;
    prevTime = millis();
    // commands = 0;
}

void loop() {
    curTime = micros();
    // Serial.println("Hello");
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
        // Serial.println(command);
        CreateSequence(commands, seq);
        CreateSequenceOld(commands[1], seqOld);
        // if (counter == 100) {
        //     Serial.println(counter);
        //     for (int i = 0; i < 17; i++) {
        //         Serial.print(seqOld[i] & 0xFF);
        //         Serial.print(" ");
        //     }
        //     Serial.println();
        //     for (int j = 0; j < 4; j++) {
        //         for (int i = 0; i < 17; i++) {
        //             Serial.print(seq[(i << 2) + j] & 0xFF);
        //             Serial.print(" ");
        //         }
        //         Serial.println();
        //     }
        // }
        // NRF_PWM0->SEQ[0].PTR = (uint32_t)&seq;
        // for (int i = 0; i < 16; i++) {
        //     Serial.print('\t');
        //     Serial.println(seq[i], BIN);
        // }
        PWM_SendCommand((uint32_t)&seq);
        // NRF_PWM0->TASKS_SEQSTART[0] = 1;
        prevTime = curTime;
        if (counter < 255)
            counter++;
    }
}
