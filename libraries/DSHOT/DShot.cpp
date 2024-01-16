#include "DShot.h"

void CalcChecksum(uint16_t *command) {
    uint16_t temp = *command;
    temp = ((temp ^ (temp >> 4)) ^ (temp >> 8)) & 0x0F;
    *command = (*command << 4) | temp;
}

void CreateSequence(uint16_t command[4], uint16_t sequence[SEQ_SIZE]) {
    uint16_t temp[4];
    for (int i = 0; i < 4; i++) {
        temp[i] = command[i];
        temp[i] = (temp[i] << 1) | TELEMETRY_BIT;
        CalcChecksum(&temp[i]);
    }
    int j;

    /* Send LSB last
     * 0 is a duty cycle of 5, 1 is a duty cycle of 10, so I simply multiply
     * 5 by 1 or 2 (through a bitshift), depending on whether the ith bit of 
     * the command is 1. 1 shifted over 15 indicates the first edge to be 
     * falling, so the PWM output starts high. This operates on all four DShot 
     * commands, so it probably takes a bit of time, but the CPU is likely fast 
     * enough that it isn't a problem.  
     */
    for (int i = 0; i < 16; i++) {
        for (j = 0; j < 4; j++) {
            sequence[(i << 2) + j] =
                (5 * (1 << ((temp[j] >> (15 - i)) & 1))) | (1 << 15);
        }
    }

    // The last compare values of the PWM sequences are always 0 (to keep the 
    // line low), so the last four values of the array are 0
    for (int i = 0; i < 4; i++) {
        sequence[SEQ_SIZE - i - 1] = 1 << 15;
    }
}
