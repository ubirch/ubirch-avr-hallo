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
#include "i2c/i2c.h"
#include "mpr121/mpr121.h"

static UbirchSIM800 sim800 = UbirchSIM800();
static Adafruit_VS1053_FilePlayer vs1053 =
        Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);


bool sendFile(const char *fname, uint8_t retries);

bool receiveFile(const char *fname) ;

void setup() {
    // configure the initial values for UART and the connection to SIM800
    minimumSerial.begin((uint32_t) BAUD);

    // initially disable the watchdog, it confused people
    enable_watchdog();

    blink(3, 1000);

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

    disable_watchdog();

    // == SETUP ===============================================
    if (!SD.begin(CARDCS)) {
        PRINTLN("SDCARD not found");
        haltError(1);
    }
    createTestFile();
    PRINTLN("SDCARD initialized");

    if (!vs1053.begin()) {
        PRINTLN("VS1053 not found");
        haltError(1);
    }
    vs1053.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
    PRINTLN("VS1053 initialized ");

    i2c_init(I2C_SPEED_400KHZ);
    if (!mpr_reset()) {
        PRINTLN("MPR121 not found");
        haltError(5);
    }
    PRINTLN("MPR121 initialized");
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"


void loop() {
    static File file;
    uint8_t retries = 2;

    uint16_t lasttouched = 0;
    uint16_t mpr121_status = 0;

    static int c = 'u';

//    freeMem();
//    PRINTLN("MENU [u - upload, d - download, p - play file, c - touch test]");
//    while ((c = minimumSerial.read()) == -1);
//    while (minimumSerial.read() != -1);

    switch ((char) c) {
        case -1:
            break;

        case 'u':
            PRINTLN("sending file");
            c = sendFile("test.ogg", retries) ? 'd' : 'x';
            file.close();
            break;

        case 'd':
            PRINTLN("receiving file");
            c = receiveFile("received.ogg") ? 'p' : 'x';
            break;

        case 'p':
            vs1053.setVolume(1, 1);
            PRINTLN("playing downloaded file");
            vs1053.playFullFile("received.ogg");
            c = 'x';
            break;

        case 'c':
            while (1) {
                // Get the currently touched pads
                mpr121_status = mpr_status();

                for (uint8_t i = 0; i < 12; i++) {
                    // it if *is* touched and *wasnt* touched before, alert!
                    if ((mpr121_status & _BV(i)) && !(lasttouched & _BV(i))) {
                        DEBUG(i);
                        PRINTLN(" touched");
                    }
                    // if it *was* touched and now *isnt*, alert!
                    if (!(mpr121_status & _BV(i)) && (lasttouched & _BV(i))) {
                        DEBUG(i);
                        PRINTLN(" released");
                    }
                }

                // reset our state
                lasttouched = mpr121_status;
            }

            break;

        default:
            haltOK();
            break;

    }
}

#pragma clang diagnostic pop

bool sendFile(const char *fname, uint8_t retries) {
    uint16_t status;
    uint32_t length;

    SD.cacheClear();
    do {
        char date[10], time[10], tz[5];
        sim800.time(date, time, tz);
        PRINT("START: ");
        DEBUGLN(time);
        File file = SD.open(fname, O_RDONLY);
        status = sim800.HTTP_post("http://api.ubirch.com:23456/upload", length, file, file.fileSize());
        file.close();
        sim800.time(date, time, tz);
        PRINT("END  : ");
        DEBUGLN(time);
        switch (status) {
            case 200:
                PRINT("200 OK (");
                DEBUG(length);
                PRINTLN(" bytes)");
                return true;
            case 408:
                PRINTLN("CONNECTION TIMEOUT");
                break;
            case 601:
                PRINTLN("CONNECTION REFUSED");
                break;
            default:
                DEBUG(status);
                PRINT("??? (");
                DEBUG(length);
                PRINTLN(" bytes)");
                break;
        }
    } while (--retries);
    return false;
}

bool receiveFile(const char *fname) {
    uint16_t result;
    uint32_t length;

    SD.cacheClear();
    char date[10], time[10], tz[5];
    sim800.time(date, time, tz);
    PRINT("START: ");
    DEBUGLN(time);
    File file = SD.open(fname, O_WRONLY | O_CREAT | O_TRUNC);
    result = sim800.HTTP_get("http://api.ubirch.com:23456/download", length, file);
    file.close();
    sim800.time(date, time, tz);
    PRINT("END  : ");
    DEBUGLN(time);
    file.close();
    switch (result) {
        case 200:
            PRINT("200 OK (");
            DEBUG(length);
            PRINTLN(" bytes)");
            return true;
        case 601:
            PRINTLN("CONNECTION REFUSED");
        default:
            DEBUG(result);
            PRINT("??? (");
            DEBUG(length);
            PRINTLN(" bytes)");
            break;
    }
    return false;
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

static void blink(uint8_t n, unsigned long speed) {
    digitalWrite(UBIRCH_NO1_PIN_LED, LOW);
    for (int i = n * 2; i > 0; i--) {
        PRINT(".");
        digitalWrite(UBIRCH_NO1_PIN_LED, i % 2 == 0 ? HIGH : LOW);
        delay(speed);
    }
    digitalWrite(UBIRCH_NO1_PIN_LED, LOW);
    PRINTLN("");
}

static void freeMem() {
    PRINT("Free memory = ");
    DEBUGLN(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// send blink signals for error codes and never return
static void haltError(uint8_t code) {
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


static void haltOK() {
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
