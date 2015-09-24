/** 
 * ubirch-hallo (the code that drives the puppet)
 *
 * ...
 *
 * @author Matthias L. Jugel
 * 
 * == LICENSE ==
 * Copyright 2015 ubirch GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
  */

#include <SdFat.h>
#include <MinimumSerial.h>
#include "main.h"
#include "sim800/UbirchSIM800.h"
#include "vs1053/Adafruit_VS1053_FilePlayer.h"

MinimumSerial debug;

#define PRINT(s) debug.print(F(s))
#define PRINTLN(s) debug.println(F(s))
#define DEBUG(s) debug.print(s)
#define DEBUGLN(s) debug.println(s)

extern SdFat SD;

extern "C" {
#include "ubirch.h"
#include "checksum.h"
}

UbirchSIM800 sim800 = UbirchSIM800();
Adafruit_VS1053_FilePlayer musicPlayer =
        Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

void blink(uint8_t n, unsigned long speed) {
    digitalWrite(UBIRCH_NO1_PIN_LED, LOW);
    for (int i = n * 2; i > 0; i--) {
        PRINT(".");
        digitalWrite(UBIRCH_NO1_PIN_LED, i % 2 == 0 ? HIGH : LOW);
        delay(speed);
    }
    digitalWrite(UBIRCH_NO1_PIN_LED, LOW);
    PRINTLN("");
}

extern unsigned int __heap_start;
extern void *__brkval;


void setup() {
    // configure the initial values for UART and the connection to SIM800
    Serial.begin(BAUD);

    // initially disable the watchdog, it confused people
    disable_watchdog();
    pinMode(UBIRCH_NO1_PIN_LED, OUTPUT);

    blink(3, 1000);

    if (!musicPlayer.begin()) { // initialise the music player
        PRINTLN("Couldn't find VS1053, do you have the right pins defined?");
        error(HALLO_ERROR_AUDIO);
    }
    PRINTLN("Initialized VS1053");
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);

    if (!SD.begin(CARDCS)) {  // initialise the SD card
        PRINTLN("SDCARD not found");
        error(HALLO_ERROR_SDCARD);
    }
    PRINTLN("Initialized SD-Card");

    static File f = SD.open("test.mp3", O_RDWR|O_CREAT|O_TRUNC);
    PRINTLN("Created file test.mp3");

    PRINTLN("Starting download test...");

    PRINT("Free memory = ");
    DEBUGLN(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));

    PRINTLN("Initializing SIM800");
    sim800.reset();
    sim800.setAPN(F(SIM800_APN), F(SIM800_USER), F(SIM800_PASS));

    PRINTLN("SIM800 wakeup...");
    if (!sim800.wakeup()) error(1);
    PRINTLN("SIM800 waiting for network registration...");
    if (!sim800.registerNetwork()) error(2);
    PRINTLN("SIM800 enabling GPRS...");
    if (!sim800.enableGPRS()) error(3);

    PRINTLN("Allocating buffer & downloading data...");
    static size_t length = 0;
    uint16_t status = sim800.GET("http://api.ubirch.com/rp15/app/test.mp3", length);
    DEBUGLN(status);
    DEBUGLN(length);

    if(length > 0) {
#define BUFSIZE 512
        static char *buffer = (char *) malloc(BUFSIZE);
        buffer[BUFSIZE] = 0;
        uint16_t pos = 0, r;
        do {
            r = sim800.GETReadPayload(buffer, pos, BUFSIZE);
            f.write(buffer, r);
            if((pos % 10*1024) == 0) { DEBUG(pos); PRINTLN(" "); } else PRINT(".");
            pos += r;
        } while (pos < length);

        free(buffer);
    }
    f.close();

    PRINTLN("");
    PRINTLN("Done.");

    sim800.shutdown();

    PRINTLN("WELCOME!");

    PRINT("Free memory = ");
    DEBUGLN(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// the main loop just reads the responses from the modem and
// writes them to the serial port
void loop() {
    musicPlayer.setVolume(1, 1);
    PRINTLN("Playing downloaded file...");
    musicPlayer.playFullFile("test.mp3");
//    PRINTLN("Playing music ...");
//    musicPlayer.playFullFile("TRACK001.MP3");
}

#pragma clang diagnostic pop
