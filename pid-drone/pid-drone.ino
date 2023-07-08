#include <cstdio>
#include <stdint.h>
#include "mbed.h"
#include "VL53L4CX.h"

#define SEQ_SIZE 17
#define TELEMETRY_BIT 0

uint16_t seq[SEQ_SIZE * 4];
uint16_t commands[4];

/**
 * Takes a command (currently in the form 0000TSSSSSSSSSSS), and calculates a
 * checksum based on the 12 set bits. Modifies the command in place.
 */
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
        Serial.println(temp[i]);
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
            if (j == 1) {
                Serial.println(sequence[((15 - i) << 2) + j] & 31);
            }
        }
    }
    // The last compares of the PWM sequences are always 0, so the last four
    // values of the sequence are 0
    for (int i = 0; i < 4; i++) {
        sequence[SEQ_SIZE * 4 - i - 1] = 1 << 15;
    }
}

void PWM_Init() {
    NRF_PWM0->ENABLE = 1;

    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up;
    NRF_PWM0->COUNTERTOP = 13;
    NRF_PWM0->LOOP = 0;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
    NRF_PWM0->DECODER = PWM_DECODER_LOAD_Individual |
                        (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    // I think this may be okay just being set once
    NRF_PWM0->SEQ[0].PTR = (uint32_t)&seq;
    NRF_PWM0->SEQ[0].CNT = SEQ_SIZE * 4;
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
}

/**
 * Connects a digital pin to the PWM module
 * Important note: PWM cannot be enabled before adding a pin. Check the
 * Nano 33 BLE pinout for info on port and pin values
 */
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

VL53L4CX distanceSensor(&Wire, A1);
uint8_t dataReady;
VL53L4CX_MultiRangingData_t rangeData;
int status;

void setup() {
    distanceSensor.VL53L4CX_WaitDeviceBooted();
    distanceSensor.VL53L4CX_DataInit();
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Wire.begin();
    distanceSensor.begin();
    distanceSensor.VL53L4CX_Off();
    distanceSensor.InitSensor(0x12);
    commands[0] = 1046;
    commands[1] = 500;
    commands[2] = 1500;
    commands[3] = 2000;
    // CreateSequence(commands, seq);

    // NRF_TIMER2->INTENSET = 0x000F0000;
    // PWM_AddPins(0, P1_11);
    // PWM_AddPins(1, P1_12);
    // PWM_AddPins(2, P1_13);
    // PWM_AddPins(3, P1_14);
    // PWM_Init();
    // PWM_SendCommand();
    distanceSensor.VL53L4CX_StartMeasurement();
}

void loop() {
    status = distanceSensor.VL53L4CX_GetMeasurementDataReady(&dataReady);
    Serial.println(status);
    if (!status && dataReady) {
        distanceSensor.VL53L4CX_GetMultiRangingData(&rangeData);
        for (int i = 0; i < rangeData.NumberOfObjectsFound; i++) {
            char text[23];
            sprintf(text, "Found %1d: Distance % 4d", i,
                    rangeData.RangeData[i].RangeMilliMeter);
            Serial.println(text);
        }
        Serial.println();
    }
}
