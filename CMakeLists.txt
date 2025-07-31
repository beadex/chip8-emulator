cmake_minimum_required(VERSION 3.26)
project(chip8 C CXX)

find_package(SDL3 REQUIRED)

add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
)

option(ENABLE_DEBUG_FEATURES "Enable extra debug features and define DEBUG macro" ON)
if(ENABLE_DEBUG_FEATURES)
    add_compile_definitions(DEBUG)
endif()

add_executable(chip8)

target_sources(chip8
PRIVATE
    chip8.c
)

target_link_libraries(chip8 SDL3::SDL3)