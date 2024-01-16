#ifndef PWM_H
#define PWM_H
#include <stdint.h>
#include "mbed.h"
#include "DShot.h"

void PWM_Init(uint32_t seqAddr);

/**
 * Connects a digital pin to the PWM module
 * Important note: PWM cannot be enabled before adding a pin. Check the
 * Nano 33 BLE pinout for info on port and pin values
 */
int PWM_AddPins(uint8_t channel, PinName pin);

void PWM_SendCommand(uint32_t addr);
#endif
