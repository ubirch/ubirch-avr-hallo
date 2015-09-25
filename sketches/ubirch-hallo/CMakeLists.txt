add_executable_avr(ubirch-hallo
        main.cpp
        checksum.c
        sim800/UbirchSIM800.cpp
        vs1053/Adafruit_VS1053.cpp
        vs1053/Adafruit_VS1053_FilePlayer.cpp
        )
target_link_libraries(ubirch-hallo SoftwareSerial)
target_sketch_library(ubirch-hallo SdFat "locally checked in")
target_sketch_library(ubirch-hallo Adafruit_FONA_Library https://github.com/adafruit/Adafruit_FONA_Library)

# copy the config.h.template to config.h in case it is not there; it is ignored by .git!
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    message(AUTHOR_WARNING "In directory 'sketches/fewl': installing the template config.h, please edit!")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.template ${CMAKE_CURRENT_SOURCE_DIR}/config.h COPYONLY)
endif ()
