#include "CustomI2C.h"
#include <stdint.h>
#include "mbed.h"
#include "VL53L4CX.h"

VL53L4CX distanceSensor(&Wire, A1);

void setup() { Wire.begin(); }

void loop() {}
