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

#define BUFSIZE 64

MinimumSerial minimumSerial;

#define PRINT(s) minimumSerial.print(F(s))
#define PRINTLN(s) minimumSerial.println(F(s))
#define DEBUG(s) minimumSerial.print(s)
#define DEBUGLN(s) minimumSerial.println(s)

extern SdFat SD;

UbirchSIM800 sim800 = UbirchSIM800();
Adafruit_VS1053_FilePlayer vs1053 =
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

void freeMem();

void setup() {
    // configure the initial values for UART and the connection to SIM800
    Serial.begin(BAUD);

    // initially disable the watchdog, it confused people
    disable_watchdog();

    blink(3, 1000);

    // == SETUP ===============================================
    if (!SD.begin(CARDCS)) {
        PRINTLN("SDCARD not found");
        haltError(0);
    }
    createTestFile();
    PRINTLN("SDCARD initialized");

    PRINTLN("SIM800 wakeup");
    if (!sim800.wakeup()) haltError(1);
    sim800.setAPN(F(SIM800_APN), F(SIM800_USER), F(SIM800_PASS));

    PRINTLN("SIM800 waiting for network registration");
    while (!sim800.registerNetwork()) {
        sim800.shutdown();
        sim800.wakeup();
    }
    PRINTLN("SIM800 enabling GPRS");
    if (!sim800.enableGPRS()) haltError(3);
    PRINTLN("SIM800 initialized");

}

inline bool sendFile(const char *address, uint16_t port, const char *fname) {
    if (!sim800.connect(address, port)) {
        sim800.shutdown();
        haltError(10);
    }

    static char *buffer = (char *) malloc(BUFSIZE);

    int r = 0;
    static uint32_t n = 0;

    SD.cacheClear();
    File file = SD.open(fname, O_RDONLY);
    if (!file) {
        PRINT("could not open file: ");
        DEBUGLN(fname);
        return false;
    }
    while ((r = file.read(buffer, BUFSIZE)) > 0) {
        DEBUG(n);
        PRINT(" ");
        if (r == -1) {
            PRINTLN("ERROR READING");
            return false;
        } else {
            if (r < BUFSIZE) PRINTLN("EOF");
            static size_t accepted = 0;
            if (!sim800.send(buffer, (size_t) r, accepted)) {
                PRINT("ERROR: ");
                DEBUG(r);
                PRINT(" ");
                DEBUGLN(accepted);
            } else {
                PRINTLN("OK");
            }
            n += r;
        }
    }
    file.close();
    free(buffer);

    return sim800.disconnect();
}

void freeMem() {
    PRINT("Free memory = ");
    DEBUGLN(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// the main loop just reads the responses from the modem and
// writes them to the serial port
static int c = 'd';

void loop() {
//    freeMem();
//    PRINTLN("MENU [u - upload, d - download, p - play file]");
//    while ((c = minimumSerial.read()) == -1);
//    while (minimumSerial.read() != -1);

    switch ((char) c) {
        case -1:
            break;
        case 'u':
            PRINTLN("sending file");
            if (sendFile("api.ubirch.com", 23456, "TEST.TXT")) {
                PRINTLN("SUCCESS");
            } else {
                PRINTLN("FAILED");
            }
            break;
        case 'd':
            PRINTLN("receiving file");
            SD.cacheClear();
            static uint32_t length;
            static File file = SD.open("TEST.OGG", O_WRONLY | O_CREAT | O_TRUNC);
            static uint16_t result = sim800.HTTP_get("http://api.ubirch.com/rp15/app/test2.ogg", length, file);
            file.close();
            switch(result) {
                case 200: 
                    PRINT("200 OK (");
                    DEBUG(length);
                    PRINTLN(" bytes)");
                    c = 'p';
                    break;
                default: 
                    DEBUG(result);
                    PRINT("??? (");
                    DEBUG(length);
                    PRINTLN(" bytes)");
                    c = 'x';
            }
            break;
        case 'p':
            if (!vs1053.begin()) {
                PRINTLN("VS1053 not found");
                haltError(1);
            }
            vs1053.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
            PRINTLN("VS1053 initialized ");

            vs1053.setVolume(1, 1);
            PRINTLN("playing downloaded file");
            vs1053.playFullFile("TEST.OGG");
            c = 'x';
            break;
        default:
            break;

    }
}

void createTestFile() {
    if (SD.exists("TEST.TXT")) return;

    File testFile = SD.open("TEST.TXT", O_CREAT | O_WRONLY | O_TRUNC);

    PRINTLN("CREATING TEST FILE");
    char number[80];
    for (int line = 0; line < 3187; line++) {
        sprintf(number, "%06u ", line);
        testFile.write(number, 7);
        DEBUG(number);
        for (uint16_t c = 0; c < 73; c++) {
            uint8_t b = (uint8_t) ('0' + (c % 10));
            testFile.print((char) b);
            DEBUG((char) b);
        }
        PRINTLN("");
        testFile.println();
    }
    testFile.close();
    PRINTLN("CREATED TEST FILE");
    DEBUG(testFile.size());
    PRINTLN(" bytes");
}

// send blink signals for error codes and never return
void haltError(uint8_t code) {
    PRINT("ERROR: ");
    DEBUGLN(code);
    pinMode(UBIRCH_NO1_PIN_LED, OUTPUT);
    for (; ;) {
        for (uint8_t i = 0; i < code; i++) {
            enable_led();
            delay(500);
            disable_led();
            delay(500);
        }
        PRINT("E");
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
