all:
	arduino-cli compile -b arduino:mbed_nano:nano33ble -v \
	--library /home/alex/Documents/Arduino/libraries/STM32duino_VL53L4CX \
	--library ./libraries/DSHOT \
	--build-path ./build .

compile:
	arduino-cli compile -b arduino:mbed_nano:nano33ble -v \
	--library /home/alex/Documents/Arduino/libraries/STM32duino_VL53L4CX \
	--library ./DSHOT \
	--build-path ./build --only-compilation-database .

upload:
	arduino-cli upload -b arduino:mbed_nano:nano33ble -v --input-dir ./build \
	-p /dev/ttyACM0
