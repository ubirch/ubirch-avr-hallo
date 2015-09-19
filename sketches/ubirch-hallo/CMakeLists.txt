add_executable_avr(ubirch-hallo main.cpp)
target_link_libraries(ubirch-hallo SoftwareSerial)

# special external dependencies can be added like this and will be downloaded once
# arguments: <target> <name> <git url>
target_sketch_library(ubirch-hallo Adafruit_SleepyDog https://github.com/adafruit/Adafruit_SleepyDog)
target_sketch_library(ubirch-hallo Adafruit_SD https://github.com/adafruit/SD)
target_sketch_library(ubirch-hallo Adafruit_FONA_Library https://github.com/adafruit/Adafruit_FONA_Library)
target_sketch_library(ubirch-hallo Adafruit_VS1053_Library https://github.com/adafruit/Adafruit_VS1053_Library)

# copy the config.h.template to config.h in case it is not there; it is ignored by .git!
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    message(AUTHOR_WARNING "In directory 'sketches/fewl': installing the template config.h, please edit!")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.template ${CMAKE_CURRENT_SOURCE_DIR}/config.h COPYONLY)
endif()
