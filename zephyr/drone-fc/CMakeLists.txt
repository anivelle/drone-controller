cmake_minimum_required(VERSION 3.20.0)


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(drone-fc)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/sparkfun_icm20948/ICM_20948_C.c)
target_sources(app PRIVATE src/vl53l4cx/vl53l4cx_all.c)
# zephyr_compile_options(-Wno-unused-variable -Wno-comment)
