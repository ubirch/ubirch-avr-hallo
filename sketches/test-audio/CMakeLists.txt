add_executable_avr(test-audio main.cpp vs1053/VS1053.cpp vs1053/VS1053_FilePlayer.cpp)
# special external dependencies can be added like this and will be downloaded once
# arguments: <target> <name> <git url>
target_sketch_library(test-audio SdFat "")
