cmake_minimum_required(VERSION 3.12)

add_library(barometer
    bme280.cpp
    bme280.h
)


target_include_directories(barometer PUBLIC
    Library/barometer
    ${PICO_SDK_PATH}/src/rp2_common/pico_time/include
    # ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c/include/hardware
    ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c
            
)

target_link_libraries(barometer
        hardware_i2c
        pico_stdlib
)

