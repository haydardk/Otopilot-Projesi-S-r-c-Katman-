cmake_minimum_required(VERSION 3.12)


add_library(sbus 
    Sbus.cpp
    Sbus.h)



target_include_directories(sbus PUBLIC 
    Library/sbus 
    ${PICO_SDK_PATH}/src/rp2_common/hardware_uart
    ${PICO_SDK_PATH}/src/rp2_common/hardware_dma)


target_link_libraries(sbus PUBLIC
    pico_stdlib
    hardware_uart
    hardware_dma)
