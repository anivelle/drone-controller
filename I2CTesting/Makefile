all:
	arduino-cli compile -b arduino:mbed_nano:nano33ble -v \
	--libraries /home/alex/Documents/Arduino/libraries \
	--libraries ../libraries \
	--build-path ./build .

compile:
	arduino-cli compile -b arduino:mbed_nano:nano33ble -v \
	--libraries /home/alex/Documents/Arduino/libraries\
	--libraries ../libraries \
	--build-path ./build --only-compilation-database .

upload:
	arduino-cli upload -b arduino:mbed_nano:nano33ble -v --input-dir ./build \
	-p /dev/ttyACM0
