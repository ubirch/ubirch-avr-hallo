add_executable_avr(test-audio main.cpp)
#target_link_libraries(test-audio arduino-core)

# special external dependencies can be added like this and will be downloaded once
# arguments: <target> <name> <git url>
target_sketch_library(test-audio Adafruit_SD https://github.com/adafruit/SD)
target_sketch_library(test-audio Adafruit_VS1053_Library https://github.com/adafruit/Adafruit_VS1053_Library)
