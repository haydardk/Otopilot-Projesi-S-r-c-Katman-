cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(project_name)

pico_sdk_init()

add_executable(icm20948
	main.c
)

#Add pico_stdlib library ,add more if used
target_link_libraries(icm20948 pico_stdlib hardware_spi) # hardware_i2c eklendi

#enable usb output, disable uart output
pico_enable_stdio_usb(icm20948 1)
pico_enable_stdio_uart(icm20948 0)

#Need to generate UF2 file for upload to RP2040
pico_add_extra_outputs(icm20948)