cmake_minimum_required(VERSION 3.20.0)

set(BOARD arduino_nano_33_ble)
cmake_path(SET ZephyrBase $ENV{ZEPHYR_BASE})
# set(CONF_FILE $ENV{ZEPHYR_BASE}/../modules/lib/Arduino-Zephyr-API/module.conf)
set(DTC_OVERLAY_FILE ${ZephyrBase}/../modules/lib/Arduino-Zephyr-API/variants/${BOARD}/${BOARD}.overlay)
set(EXTRA_DTC_OVERLAY_FILE app.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(drone-fc)

target_sources(app PRIVATE src/main.cpp)
target_sources(app PRIVATE src/vl53lx_class.cpp)
zephyr_compile_options(-Wno-unused-variable -Wno-comment)
