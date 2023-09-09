#include <stdint.h>
#include <Wire.h>
#include "BNO055.h"
#include "mbed.h"

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

mbed::Ticker ticker;

const char *titles[] = {"Heading", "Roll", "Pitch"};
volatile uint8_t drdy;
int angle[3];
volatile int temp;
long prevTime;

void timer_callback() {
    // if (NRF_TIMER1->EVENTS_COMPARE[0]) {
    drdy = 1;
    //    NRF_TIMER1->EVENTS_COMPARE[0] = 0;
    //    temp = NRF_TIMER1->EVENTS_COMPARE[0];
    //    temp;
    //}
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

    // Timer interrupt for the IMU
    // NRF_TIMER1->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_32;
    // NRF_TIMER1->CC[0] = 5000;
    // NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    // NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    // NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    // NVIC_EnableIRQ(TIMER1_IRQn);
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

    ticker.attach(&timer_callback, 0.012);
    curTime = millis();
    units = gyro.readRegister(UNIT_SEL);
    Serial.println("Beginning Calibration");
    makeTransition = 1;
    nextState = CALIBRATING;
    prevTime = millis();
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
        // if (millis() - prevTime > 10) {
        if (drdy) {
            // Serial.println(NRF_TIMER1->CC[1]);
            for (int i = 0; i < 3; i++) {
                angle[i] = gyro.readEul((Axis)i);
                if (units & 4)
                    angle[i] /= 900;
                else
                    angle[i] /= 16;
                Serial.print(titles[i]);
                Serial.print(": ");
                Serial.println(angle[i]);
            }
            drdy = 0;
            // NRF_TIMER1->TASKS_CLEAR = 1;
            // NRF_TIMER1->EVENTS_COMPARE[0] = 0;
            prevTime = millis();
        }
        break;
    default:
        break;
    }
    if (makeTransition) {
        // Small things to do on exiting a state
        switch (state) {
        case CONFIGURING:
            if (nextState == FLYING)
                NRF_TIMER1->TASKS_START = 1;
            break;
        case CALIBRATING:
            NRF_TIMER1->TASKS_START = 1;
            NRF_TIMER1->TASKS_CLEAR = 1;
            break;
        case FLYING:
            NRF_TIMER1->TASKS_STOP = 1;
        }
        makeTransition = 0;
        state = nextState;
    }
}
