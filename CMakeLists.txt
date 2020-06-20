cmake_minimum_required(VERSION 2.8)

set(AVR_MCU "atmega2560")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/avr.toolchain.cmake")

project(AVR-Hydroponics C)

add_definitions(-DF_CPU=16000000)

add_avr_executable(${PROJECT_NAME}
    src/Firmware/main.c
)