#ifndef DSHOT_H
#define DSHOT_H

#include <stdint.h>

#define SEQ_SIZE 17 * 4
#define TELEMETRY_BIT 0

/**
 * Takes a command (currently in the form 0000TSSSSSSSSSSS), and calculates a
 * checksum based on the 12 set bits. Modifies the command in place.
 */
void CalcChecksum(uint16_t *command);

void CreateSequence(uint16_t command[4], uint16_t sequence[SEQ_SIZE]);

#endif
