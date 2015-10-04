add_executable_avr(ubirch-hallo
        main.cpp
        checksum.c
        i2c/i2c_core.c
        i2c/i2c_registers.c
        mpr121/mpr121.c
        sim800/UbirchSIM800.cpp
        vs1053/Adafruit_VS1053.cpp
        vs1053/Adafruit_VS1053_FilePlayer.cpp
        ws2812/ws2812.c
        )
target_link_libraries(ubirch-hallo SoftwareSerial)
target_sketch_library(ubirch-hallo SdFat "locally checked in")

# copy the config.h.template to config.h in case it is not there; it is ignored by .git!
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    message(AUTHOR_WARNING "In directory 'sketches/fewl': installing the template config.h, please edit!")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.template ${CMAKE_CURRENT_SOURCE_DIR}/config.h COPYONLY)
endif ()
