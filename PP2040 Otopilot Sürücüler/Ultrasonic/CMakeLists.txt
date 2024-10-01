cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(project_name)

pico_sdk_init()

add_executable(Ultrasonic
	main.cpp
	Lib/ultrasonic.cpp
)

#Add pico_stdlib library ,add more if used
target_link_libraries(Ultrasonic pico_stdlib hardware_timer) # hardware_i2c eklendi

target_include_directories(Ultrasonic PRIVATE ${CMAKE_CURRENT_LIST_DIR} /Lib)

#enable usb output, disable uart output
pico_enable_stdio_usb(Ultrasonic 1)
pico_enable_stdio_uart(Ultrasonic 0)

#Need to generate UF2 file for upload to RP2040
pico_add_extra_outputs(Ultrasonic)