cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(LedMatrixProject C CXX ASM)

pico_sdk_init()

add_executable(LedMatrixProject main.c)

pico_set_program_name(LedMatrixProject "LedMatrixProject")
pico_set_program_version(LedMatrixProject "0.1")

pico_generate_pio_header(LedMatrixProject ${CMAKE_CURRENT_LIST_DIR}/led_matrix.pio)

pico_enable_stdio_uart(LedMatrixProject 0)
pico_enable_stdio_usb(LedMatrixProject 1)

target_link_libraries(LedMatrixProject
    pico_stdlib
    hardware_pio
    hardware_adc
)

target_include_directories(LedMatrixProject PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(LedMatrixProject)