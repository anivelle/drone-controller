#include "PWM.h"

void PWM_Init(uint32_t seqSize, uint16_t *seqPtr) {
    NRF_PWM0->ENABLE = 1;

    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up;
    NRF_PWM0->COUNTERTOP = 13;
    NRF_PWM0->LOOP = 0;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
    NRF_PWM0->DECODER = PWM_DECODER_LOAD_Individual |
                        (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    // I think this may be okay just being set once
    NRF_PWM0->SEQ[0].PTR = (uint32_t)seqPtr;
    NRF_PWM0->SEQ[0].CNT = seqSize * 4;
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
}

int PWM_AddPins(uint8_t channel, PinName pin) {
    // PWM cannot be enabled
    if (NRF_PWM0->ENABLE == 1)
        return -1;
    // Only 4 channels
    if (channel > 3)
        return -2;
    // Bits 0-4 indicate pin number, bit 5 is port
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    NRF_PWM0->PSEL.OUT[channel] = pin;
    return 0;
}

void PWM_SendCommand() { NRF_PWM0->TASKS_SEQSTART[0] = 1; }
