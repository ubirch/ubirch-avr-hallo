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

#include <Arduino.h>

#define PIN_LED 13
#define PIN_WATCHDOG 6

// show debug output only in non-release mode
#ifndef NDEBUG
#   define D_println(s) Serial.println(F(s))
#else
#   define DEBUG(s)
#endif

// set serial port baud if undefined
#ifndef BAUD
#   define BAUD 115200
#endif

#include <Adafruit_VS1053.h>

// These are the pins used for the breakout example
#define BREAKOUT_RESET  SDA      // VS1053 reset pin (output)
#define BREAKOUT_CS     A1     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    A0      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 10     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 9       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer =
        // create breakout-example object!
        Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

void blink(uint8_t n, unsigned long speed) {
    digitalWrite(PIN_LED, LOW);
    for (int i = n * 2; i > 0; i--) {
        Serial.print('.');
        digitalWrite(PIN_LED, i % 2 == 0 ? HIGH : LOW);
        delay(speed);
    }
    digitalWrite(PIN_LED, LOW);
    Serial.println();
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
    while(true) {

        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files
            //Serial.println("**nomorefiles**");
            break;
        }
        for (uint8_t i=0; i<numTabs; i++) {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.println("/");
            printDirectory(entry, numTabs+1);
        } else {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

void setup() {
    Serial.begin(BAUD);
    blink(4, 1000);

    Serial.println("Adafruit VS1053 Simple Test");

    if (! musicPlayer.begin()) { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
        while (1) blink(2, 500);
    }
    Serial.println(F("VS1053 found"));


    if(!SD.begin(CARDCS)) {  // initialise the SD card
        Serial.println(F("CARD not found"));
        while(1) blink(2, 500);
    }
    Serial.println(F("SD card found"));

    printDirectory(SD.open("/"), 0);

    // Set volume for left, right channels. lower numbers == louder volume!
    musicPlayer.setVolume(1,1);

    // Timer interrupts are not suggested, better to use DREQ interrupt!
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int


}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// the main loop just reads the responses from the modem and
// writes them to the serial port
void loop() {


    // Play one file, don't return until complete
    Serial.println(F("Playing track 001"));
    musicPlayer.startPlayingFile("track001.mp3");

    // File is playing in the background
    if (musicPlayer.stopped()) {
        Serial.println("Done playing music");
        while (1);
    }
    if (Serial.available()) {
        char c = Serial.read();

        // if we get an 's' on the serial console, stop!
        if (c == 's') {
            musicPlayer.stopPlaying();
        }

        // if we get an 'p' on the serial console, pause/unpause!
        if (c == 'p') {
            if (! musicPlayer.paused()) {
                Serial.println("Paused");
                musicPlayer.pausePlaying(true);
            } else {
                Serial.println("Resumed");
                musicPlayer.pausePlaying(false);
            }
        }
    }

    delay(100);
}

#pragma clang diagnostic pop
