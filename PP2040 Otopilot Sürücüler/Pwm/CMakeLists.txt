cmake_minimum_required(VERSION 3.12)

# Kaynak dosyalarını belirtin
add_library(pwm 
    PCA9685_servo.cpp
    PCA9685_servo.h
    PCA9685_servo_controller.cpp
    PCA9685_servo_controller.h

)

target_include_directories(pwm PUBLIC 
        Library/pwm 
        ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c)


target_link_libraries(pwm PUBLIC
    pico_stdlib
    hardware_i2c)

        



