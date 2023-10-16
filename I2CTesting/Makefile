all:
	arduino-cli compile -b arduino:mbed_nano:nano33ble -v \
	--libraries /home/alex/Documents/Arduino/libraries \
	--libraries ../libraries \
	--build-path ./build .

upload:
	arduino-cli upload -b arduino:mbed_nano:nano33ble -v --input-dir ./build \
	-p /dev/ttyACM0
	screen /dev/ttyACM0 115200
