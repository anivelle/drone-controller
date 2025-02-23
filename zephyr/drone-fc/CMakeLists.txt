cmake_minimum_required(VERSION 3.20.0)


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(drone-fc)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/icm20948/icm20948.c)
# zephyr_compile_options(-Wno-unused-variable -Wno-comment)
