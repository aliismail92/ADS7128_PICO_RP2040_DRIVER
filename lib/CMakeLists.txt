add_library(ADS7128 ADS7128.h ADS7128.c)


target_include_directories(ADS7128 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(ADS7128
pico_stdlib 
hardware_i2c
)
