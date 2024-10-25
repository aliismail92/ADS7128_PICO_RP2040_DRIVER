#set minimum version

cmake_minimum_required(VERSION 3.12)

#include build function from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

#set name of project (as PROJECT_NAME) and c/c++ standards

project(main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#create a pico-sdk subdirectory in our prjoect for the libraries

pico_sdk_init()

#tell cmake where to find executable source files
add_executable(${PROJECT_NAME}
main.c
)


#create map/bin/hex/uf2 files

pico_add_extra_outputs(${PROJECT_NAME})

add_subdirectory(lib)

#Link to pico_stdlib (GPIO,time etc)
target_link_libraries(${PROJECT_NAME}
pico_stdlib
hardware_i2c
ADS7128
hardware_pwm
hardware_adc
)



#enable usb and disable uart

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
