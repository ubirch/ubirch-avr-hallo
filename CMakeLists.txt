#=============================================================================
# ubirch #1 cmake file
#=============================================================================
cmake_minimum_required(VERSION 3.0)

# include the local configuration
include(config.cmake)

# the toolchain file (see https://github.com/ubirch/avr-arduino-cmake)
set(CMAKE_TOOLCHAIN_FILE cmake/avr-arduino-toolchain.cmake)

# project name and code types we use
project(ubirch C CXX ASM)

# add the sketches directory (contains sketch directories and a "libraries" dir where dependent
# libs are extracted
add_sketches(sketches)
