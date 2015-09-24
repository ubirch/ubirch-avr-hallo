/***************************************************
  This is a library for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <avr/interrupt.h>
#include <Arduino.h>
#include <SdFat.h>
#include "VS1053_FilePlayer.h"

extern SdFat SD;
static Adafruit_VS1053_FilePlayer *myself;

#ifndef _BV
#define _BV(x) (1<<(x))
#endif

#if defined(__AVR__)
SIGNAL(TIMER0_COMPA_vect) {
        myself->feedBuffer();
}
#endif

static void feeder(void) {
    myself->feedBuffer();
}

#define VS1053_CONTROL_SPI_SETTING  SPISettings(250000,  MSBFIRST, SPI_MODE0)
#define VS1053_DATA_SPI_SETTING     SPISettings(8000000, MSBFIRST, SPI_MODE0)


static const uint8_t dreqinttable[] = {
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined(__AVR_ATmega8__)
        2, 0,
        3, 1,
#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
2, 0,
3, 1,
21, 2,
20, 3,
19, 4,
18, 5,
#elif  defined(__AVR_ATmega32U4__) && defined(CORE_TEENSY)
5, 0,
6, 1,
7, 2,
8, 3,
#elif  defined(__AVR_AT90USB1286__) && defined(CORE_TEENSY)
0, 0,
1, 1,
2, 2,
3, 3,
36, 4,
37, 5,
18, 6,
19, 7,
#elif  defined(__arm__) && defined(CORE_TEENSY)
0, 0, 1, 1, 2, 2, 3, 3, 4, 4,
5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
20, 20, 21, 21, 22, 22, 23, 23, 24, 24,
25, 25, 26, 26, 27, 27, 28, 28, 29, 29,
30, 30, 31, 31, 32, 32, 33, 33,
#elif  defined(__AVR_ATmega32U4__)
3, 0,
2, 1,
0, 2,
1, 3,
7, 4,
#elif defined(__AVR_ATmega256RFR2__)
4, 0,
5, 1,
#elif  defined(__SAM3X8E__)
  0, 0, 1, 1, 2, 2, 3, 3, 4, 4,
  5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
  10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
  15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
  20, 20, 21, 21, 22, 22, 23, 23, 24, 24,
  25, 25, 26, 26, 27, 27, 28, 28, 29, 29,
  30, 30, 31, 31, 32, 32, 33, 33,

#endif
};

boolean Adafruit_VS1053_FilePlayer::useInterrupt(uint8_t type) {
    myself = this;  // oy vey

    if (type == VS1053_FILEPLAYER_TIMER0_INT) {
#if defined(__AVR__)
        OCR0A = 0xAF;
        TIMSK0 |= _BV(OCIE0A);
        return true;
#elif defined(__arm__) && defined(CORE_TEENSY)
        IntervalTimer *t = new IntervalTimer();
        return (t && t->begin(feeder, 1024)) ? true : false;
#else
        return false;
#endif
    }
    if (type == VS1053_FILEPLAYER_PIN_INT) {
        for (uint8_t i = 0; i < sizeof(dreqinttable); i += 2) {
            //Serial.println(dreqinttable[i]);
            if (_dreq == dreqinttable[i]) {
#ifdef SPI_HAS_TRANSACTION
                SPI.usingInterrupt(dreqinttable[i+1]);
#endif
                attachInterrupt(dreqinttable[i + 1], feeder, CHANGE);
                return true;
            }
        }
    }
    return false;
}

Adafruit_VS1053_FilePlayer::Adafruit_VS1053_FilePlayer(
        int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
        int8_t cardcs)
        : Adafruit_VS1053(rst, cs, dcs, dreq) {

    playingMusic = false;

    _cardCS = cardcs;

    // Set the card to be disabled while we get the VS1053 up
    pinMode(_cardCS, OUTPUT);
    digitalWrite(_cardCS, HIGH);
}

Adafruit_VS1053_FilePlayer::Adafruit_VS1053_FilePlayer(
        int8_t cs, int8_t dcs, int8_t dreq,
        int8_t cardcs)
        : Adafruit_VS1053(-1, cs, dcs, dreq) {

    playingMusic = false;

    _cardCS = cardcs;

    // Set the card to be disabled while we get the VS1053 up
    pinMode(_cardCS, OUTPUT);
    digitalWrite(_cardCS, HIGH);
}


Adafruit_VS1053_FilePlayer::Adafruit_VS1053_FilePlayer(
        int8_t mosi, int8_t miso, int8_t clk,
        int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
        int8_t cardcs)
        : Adafruit_VS1053(mosi, miso, clk, rst, cs, dcs, dreq) {

    playingMusic = false;

    // Set the card to be disabled while we get the VS1053 up
    pinMode(_cardCS, OUTPUT);
    digitalWrite(_cardCS, HIGH);
}

boolean Adafruit_VS1053_FilePlayer::begin(void) {
    uint8_t v = Adafruit_VS1053::begin();

    //dumpRegs();
    //Serial.print("Version = "); Serial.println(v);
    return (v == 4);
}


boolean Adafruit_VS1053_FilePlayer::playFullFile(const char *trackname) {
    if (!startPlayingFile(trackname)) return false;

    while (playingMusic) {
        // twiddle thumbs
        feedBuffer();
    }
    // music file finished!
    return true;
}

void Adafruit_VS1053_FilePlayer::stopPlaying(void) {
    // cancel all playback
    sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_CANCEL);

    // wrap it up!
    playingMusic = false;
    currentTrack.close();
}

void Adafruit_VS1053_FilePlayer::pausePlaying(boolean pause) {
    if (pause)
        playingMusic = false;
    else {
        playingMusic = true;
        feedBuffer();
    }
}

boolean Adafruit_VS1053_FilePlayer::paused(void) {
    return (!playingMusic && currentTrack);
}

boolean Adafruit_VS1053_FilePlayer::stopped(void) {
    return (!playingMusic && !currentTrack);
}


boolean Adafruit_VS1053_FilePlayer::startPlayingFile(const char *trackname) {
    // reset playback
    sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
    // resync
    sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
    sciWrite(VS1053_REG_WRAM, 0);

    currentTrack = SD.open(trackname);
    if (!currentTrack) {
        return false;
    }

    // As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0
    sciWrite(VS1053_REG_DECODETIME, 0x00);
    sciWrite(VS1053_REG_DECODETIME, 0x00);


    playingMusic = true;

    // wait till its ready for data
    while (!readyForData());


    // fill it up!
    while (playingMusic && readyForData())
        feedBuffer();

//  Serial.println("Ready");

    return true;
}

void Adafruit_VS1053_FilePlayer::feedBuffer(void) {
    static uint8_t running = 0;
    uint8_t sregsave;

#ifndef __SAM3X8E__
    // Do not allow 2 copies of this code to run concurrently.
    // If an interrupt causes feedBuffer() to run while another
    // copy of feedBuffer() is already running in the main
    // program, havoc can occur.  "running" detects this state
    // and safely returns.
    sregsave = SREG;
    cli();
    if (running) {
        SREG = sregsave;
        return;  // return safely, before touching hardware!  :-)
    } else {
        running = 1;
        SREG = sregsave;
    }
#endif

    if (!playingMusic) {
        running = 0;
        return; // paused or stopped
    }
    if (!currentTrack) {
        running = 0;
        return;
    }
    if (!readyForData()) {
        running = 0;
        return;
    }

    // Feed the hungry buffer! :)
    while (readyForData()) {
        //UDR0 = '.';

        // Read some audio data from the SD card file
        int bytesread = currentTrack.read(mp3buffer, VS1053_DATABUFFERLEN);

        if (bytesread == 0) {
            // must be at the end of the file, wrap it up!
            playingMusic = false;
            currentTrack.close();
            running = 0;
            return;
        }
        playData(mp3buffer, bytesread);
    }
    running = 0;
    return;
}
