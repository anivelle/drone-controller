#include <Arduino.h>
#include <Wire.h>
#include "vl53lx_class.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>


VL53LX sensor(&Wire, D15);

int main(void) {
  sensor.VL53LX_On();

  
  return 0;
}
