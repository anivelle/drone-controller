#include <stdio.h>
#include <stdint.h>
#include "mbed.h"

uint16_t command;

/**
 * Takes a command (currently in the form 0000TSSSSSSSSSSS), and calculates a
 * checksum based on the 12 set bits. Modifies the command in place.
 */
void CalcChecksum(uint16_t *command) {
    uint16_t temp = *command;
    temp = (temp ^ (temp >> 4) ^ (temp >> 8)) & 0x0F;
    *command |= temp << 12;
}

void setup() {
    Serial.begin(9600);
    NRF_TIMER2->INTENSET = 0x000F0000;
}

void loop() {}
