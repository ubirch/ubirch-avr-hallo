/**
 * ...
 *
 * @author Matthias L. Jugel
 *
 * Copyright 2015 ubirch GmbH (http://www.ubirch.com)
 *
 * == LICENSE ==
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UBIRCH_AUDIOFILEPLAYER_H
#define UBIRCH_AUDIOFILEPLAYER_H

#include <ArduinoFiles.h>
#include "VS1053.h"

#define VS1053_FILEPLAYER_TIMER0_INT 255 // allows useInterrupt to accept pins 0 to 254
#define VS1053_FILEPLAYER_PIN_INT 5

class Adafruit_VS1053_FilePlayer : public Adafruit_VS1053 {
public:
    Adafruit_VS1053_FilePlayer (int8_t mosi, int8_t miso, int8_t clk,
                                int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
                                int8_t cardCS);
    Adafruit_VS1053_FilePlayer (int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
                                int8_t cardCS);
    Adafruit_VS1053_FilePlayer (int8_t cs, int8_t dcs, int8_t dreq,
                                int8_t cardCS);

    boolean begin(void);
    boolean useInterrupt(uint8_t type);
    File currentTrack;
    volatile boolean playingMusic;
    void feedBuffer(void);
    boolean startPlayingFile(const char *trackname);
    boolean playFullFile(const char *trackname);
    void stopPlaying(void);
    boolean paused(void);
    boolean stopped(void);
    void pausePlaying(boolean pause);

private:
    uint8_t _cardCS;
};

#endif //UBIRCH_AUDIOFILEPLAYER_H
