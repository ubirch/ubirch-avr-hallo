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

// show minimumSerial output only in non-release mode
#ifndef NDEBUG
#   define D_println(s) Serial.println(F(s))
#else
#   define DEBUG(s)
#endif

// set serial port baud if undefined
#ifndef BAUD
#   define BAUD 115200
#endif

#include "vs1053/VS1053.h"
#include "vs1053/VS1053_FilePlayer.h"
#include <SdFat.h>

extern SdFat SD;

// These are the pins used for the breakout example
#define BREAKOUT_RESET  SDA      // VS1053 reset pin (output)
#define BREAKOUT_CS     A1     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    A0      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 10     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 9       // VS1053 Data request, ideally an Interrupt pin

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

static FILE uartout;

static int uart_putchar(char c, FILE *stream) {
    Serial.write(c);
    return 0;
}

void setup_serial() {
    Serial.begin(BAUD);

    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout;
}

Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

extern unsigned int __heap_start;
extern void *__brkval;


void setup() {
    setup_serial();

    blink(4, 1000);

    Serial.println(F(__FILE__));
    Serial.print(F("Free memory = "));
    Serial.println(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));

    if (!player.begin()) { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    }

    if (!SD.begin(CARDCS)) {  // initialise the SD card
        Serial.println(F("SDCARD not found"));
    }

    Serial.println();
    player.setVolume(1, 1);

    player.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

    player.startPlayingFile("TRACK001.MP3");
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// the main loop just reads the responses from the modem and
// writes them to the serial port
void loop() {
    Serial.println("P");
    delay(5000);
}

#pragma clang diagnostic pop
