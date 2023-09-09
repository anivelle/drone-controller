#include <stdint.h>
#include <Wire.h>
#include "BNO055.h"

typedef enum State { CONFIGURING, CALIBRATING, FLYING } State_t;
BNO055 gyro(&Wire);
int err;
unsigned int curTime;
int units;
uint8_t calibration;
State state;
State nextState;
uint8_t makeTransition = 0;
uint8_t calib_state = 0xFF;

const char *titles[] = {"Heading", "Roll", "Pitch"};
volatile uint8_t drdy;
volatile int angle[3];
int temp;

void nrfx_timer_1_irq_handler() {
    if (NRF_TIMER1->EVENTS_COMPARE[0]) {
        for (int i = 0; i < 3; i++) {
            angle[i] = gyro.readEul((Axis)i);
            if (units & 4)
                angle[i] /= 900;
            else
                angle[i] /= 16;
        }
        drdy = 1;
    }
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;
    temp = NRF_TIMER1->EVENTS_COMPARE[0];
}

void setup() {
    state = CONFIGURING;
    Serial.begin(115200);
    // Very important for serial to work properly (inside setup, at least)
    while (!Serial) {
    }
    Wire.begin();
    // Configuration stuff here
    delay(850); // Startup time for I2C? Not sure if necessary

    NRF_TIMER1->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_128;
    NRF_TIMER1->CC[0] = 125;
    NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Enabled
                           << TIMER_INTENSET_COMPARE0_Msk;
    NRFX_IRQ_ENABLE(TIMER1_IRQn);
    drdy = 0;
    // Ends the config mode
    err = gyro.changeMode(NDOF);
    if (err != 0) {
        Serial.println("ERROR: Did not write properly");
        Serial.println(err);
    }
    err = gyro.readRegister(OPR_MODE);
    if (err != NDOF) {
        Serial.println("ERROR: Operation mode not set");
        Serial.println(err);
    } else {
        Serial.println("We're good");
    }
    curTime = millis();
    units = gyro.readRegister(UNIT_SEL);
    Serial.println("Beginning Calibration");
    makeTransition = 1;
    nextState = CALIBRATING;
}

void loop() {
    switch (state) {
    case CONFIGURING:
        break;
    case CALIBRATING:
        calibration = gyro.readRegister(CALIB_STAT);
        if (calibration & (3 << 4) && calib_state & (3 << 4)) {
            calib_state ^= (3 << 4);
            Serial.println("Gyro calibration done");
        }
        if (calibration & (3 << 2) && calib_state & (3 << 2)) {
            calib_state ^= (3 << 2);
            Serial.println("Accelerometer calibration done");
        }
        if (calibration & 3 && calib_state & 3) {
            calib_state ^= 3;
            Serial.println("Magnetometer calibration done");
        }
        if (calibration & (3 << 6)) {
            calib_state ^= (3 << 6);
            Serial.println("Done Calibrating");
            nextState = FLYING;
            makeTransition = true;
        }
        break;
    case FLYING:
        // Serial.println(gyro.readRegister(INT_STA), HEX);
        if (drdy) {
            for (int i = 0; i < 3; i++) {
                Serial.print(titles[i]);
                Serial.print(": ");
                Serial.println(angle[i]);
            }
            drdy = 0;
        }
        NRF_TIMER1->TASKS_START = 1;
        break;
    default:
        break;
    }
    if (makeTransition) {
        makeTransition = 0;
        state = nextState;
    }
}
