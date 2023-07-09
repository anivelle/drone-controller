#include "DShot.h"

void CalcChecksum(uint16_t *command) {
    uint16_t temp = *command;
    temp = ((temp ^ (temp >> 4)) ^ (temp >> 8)) & 0x0F;
    *command = (*command << 4) | temp;
}

void CreateSequence(uint16_t command[4], uint16_t sequence[SEQ_SIZE * 4]) {
    uint16_t temp[4];
    for (int i = 0; i < 4; i++) {
        temp[i] = command[i];
        temp[i] = (temp[i] << 1) | TELEMETRY_BIT;
        CalcChecksum(&temp[i]);
    }
    int j;
    /* Send LSB last
     * 0 is a duty cycle of 5, 1 is a duty cycle of 10, so I simply multiply
     * 5 by 1 or 2, depending on whether the ith bit of command is 1.
     * 1 shifted over 15 indicates the first edge to be falling, so the PWM
     * output starts high. This operates on all four DShot commands, so it kinda
     * takes a bit of time. Good things microcontrollers are fast and these are
     * technically quick commands.
     */
    for (int i = 15; i >= 0; i--) {
        for (j = 0; j < 4; j++) {
            sequence[((15 - i) << 2) + j] =
                (5 * (1 << ((temp[j] >> i) & 1))) | (1 << 15);
        }
    }
    // The last compares of the PWM sequences are always 0, so the last four
    // values of the sequence are 0
    for (int i = 0; i < 4; i++) {
        sequence[SEQ_SIZE * 4 - i - 1] = 1 << 15;
    }
}
