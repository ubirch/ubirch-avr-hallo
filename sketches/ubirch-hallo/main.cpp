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

#define BUFSIZE 512

MinimumSerial minimumSerial;

#define PRINT(s) minimumSerial.print(F(s))
#define PRINTLN(s) minimumSerial.println(F(s))
#define DEBUG(s) minimumSerial.print(s)
#define DEBUGLN(s) minimumSerial.println(s)

extern SdFat SD;

extern "C" {
#include "checksum.h"
}

static UbirchSIM800 sim800 = UbirchSIM800();
static Adafruit_VS1053_FilePlayer musicPlayer =
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


void haltOK();

// send blink signals for error codes and never return
void haltError(uint8_t code);

void createTestFile();

void setup() {
    // configure the initial values for UART and the connection to SIM800
    Serial.begin(BAUD);

    // initially disable the watchdog, it confused people
    disable_watchdog();

    blink(3, 1000);

    if (!SD.begin(CARDCS)) {  // initialise the SD card
        PRINTLN("SDCARD not found");
        haltError(HALLO_ERROR_SDCARD);
    }
    PRINTLN("Initialized SD-Card");

    if (!musicPlayer.begin()) { // initialise the music player
        PRINTLN("Couldn't find VS1053, do you have the right pins defined?");
        haltError(HALLO_ERROR_AUDIO);
    }
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
    PRINTLN("Initialized VS1053");

    musicPlayer.setVolume(1, 1);

    createTestFile();

    haltOK();

    if (SD.remove("test.ogg")) PRINTLN("Deleted download file...");;
    static File f = SD.open("test.ogg", O_WRONLY | O_CREAT | O_TRUNC);
    PRINTLN("Created download file ...");

    PRINTLN("Starting download test...");

    PRINT("Free memory = ");
    DEBUGLN(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));

    PRINTLN("Initializing SIM800");
    sim800.reset();
    sim800.setAPN(F(SIM800_APN), F(SIM800_USER), F(SIM800_PASS));

    PRINTLN("SIM800 wakeup...");
    if (!sim800.wakeup()) haltError(1);
    PRINTLN("SIM800 waiting for network registration...");
    while (!sim800.registerNetwork()) {
        sim800.shutdown();
        sim800.wakeup();
    }
    PRINTLN("SIM800 enabling GPRS...");
    if (!sim800.enableGPRS()) haltError(3);

    PRINTLN("Allocating buffer & downloading data...");
    static uint32_t length = 0;
    uint16_t status = sim800.GET("http://api.ubirch.com/rp15/app/test2.ogg", length);
    DEBUGLN(status);
    DEBUGLN(length);

//    PRINTLN("Press enter to download...");
//    while(minimumSerial.read() == -1);

    if (length > 0) {
        static char *buffer = (char *) malloc(BUFSIZE);
        uint32_t pos = 0, r;
        do {
            r = sim800.GETReadPayload(buffer, pos, BUFSIZE);
            f.write(buffer, r);
            if ((pos % 10 * 1024) == 0) {
                if (f.sync()) PRINT("! ");
                DEBUG(pos);
                PRINTLN("");
            } else
                PRINT(".");
            pos += r;
        } while (pos < length);

        free(buffer);
    }
    if (f.close()) PRINTLN("Download file closed");

    PRINTLN("");
    PRINT("Free memory = ");
    DEBUGLN(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));

    PRINTLN("Done.");

    musicPlayer.setVolume(1, 1);
    PRINTLN("Playing downloaded file...");
    musicPlayer.startPlayingFile("test.ogg");

    sim800.shutdown();

    PRINTLN("THANK YOU!");


}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// the main loop just reads the responses from the modem and
// writes them to the serial port
void loop() {
    haltOK();
}


void createTestFile() {
    PRINTLN("CREATING TEST FILE");
    File testFile = SD.open("test.txt", O_WRONLY | O_CREAT | O_TRUNC);
    char buffer[BUFSIZE] = "";
    for (int line = 0; line < 300; line++) {
        for (int i = 0; i < BUFSIZE; i++) buffer[i] = 0;
        sprintf(buffer, "%06u ", line);
        for (uint16_t c = 7; c < BUFSIZE; c++) {
            buffer[c] = (char) (c & 0x000f) + '0';
        }
        DEBUG(buffer);
        testFile.write(buffer, BUFSIZE);
        for (uint16_t c = 0; c < BUFSIZE - 1; c++) {
            buffer[c] = (char) (c & 0x000f) + '0';
        }
        buffer[BUFSIZE - 1] = '\n';
        DEBUGLN(buffer);
        testFile.write(buffer, BUFSIZE);
    }
    free(buffer);
    testFile.close();
    PRINTLN("CREATED TEST FILE");
    DEBUG(testFile.size()); PRINTLN(" bytes");
}

// send blink signals for error codes and never return
void haltError(uint8_t code) {
    pinMode(UBIRCH_NO1_PIN_LED, OUTPUT);
    for (; ;) {
        for (uint8_t i = 0; i < code; i++) {
            enable_led();
            delay(500);
            disable_led();
            delay(500);
            PRINT("ERROR: ");
            DEBUGLN(code);
        }
        delay(5000);
    }
}


void haltOK() {
    pinMode(UBIRCH_NO1_PIN_LED, OUTPUT);
    for (; ;) {
        enable_led();
        delay(1000);
        disable_led();
        delay(1000);
        PRINT("+");
    }
}

#pragma clang diagnostic pop
