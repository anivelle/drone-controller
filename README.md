# PID Drone Controller

This is my repository for the drone controller I am building using the Arduino
Nano 33 BLE. This is an exercise for me in working directly with the Arduino's
hardware peripherals, as well as learning about PID (I have never taken a
feedback control course).

I'll be using some sort of time of flight (TOF) sensor to give the drone the
ability to hover at a certain height, along with a 9-axis IMU (currently the
BNO055 because I have it) to keep the drone stabilized in the air. That is all
that is currently planned, but if I enjoy this enough I might try autonomous
movement.

ICM-20948 code pulled from [Sparkfun's Arduino
Library](https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary/tree/main)
because it turns out the DMP-related code is far more complex than I need to
work with. I am trying to reinvent the wheel, but not that much.

