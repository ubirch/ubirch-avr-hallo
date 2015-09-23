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

#include <Adafruit_VS1053.h>
#include <SoftwareSerial.h>
#include "main.h"
#include "ubirch_sim800.h"

extern "C" {
#include "error.h"
#include "checksum.h"
}


SoftwareSerial sim800Serial = SoftwareSerial(UBIRCH_NO1_SIM800_TX, UBIRCH_NO1_SIM800_RX);
UbirchSIM800 sim800 = UbirchSIM800(&sim800Serial, UBIRCH_NO1_SIM800_RST, UBIRCH_NO1_SIM800_KEY, UBIRCH_NO1_SIM800_PS);
Adafruit_VS1053_FilePlayer musicPlayer =
        Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

#define BUF_SIZE 32
char buffer[BUF_SIZE];

void blink(uint8_t n, unsigned long speed) {
    digitalWrite(UBIRCH_NO1_PIN_LED, LOW);
    for (int i = n * 2; i > 0; i--) {
        Serial.print('.');
        digitalWrite(UBIRCH_NO1_PIN_LED, i % 2 == 0 ? HIGH : LOW);
        delay(speed);
    }
    digitalWrite(UBIRCH_NO1_PIN_LED, LOW);
    Serial.println();
}

void setup() {
// initially disable the watchdog, it confused people
    disable_watchdog();
    pinMode(UBIRCH_NO1_PIN_LED, OUTPUT);

    // configure the initial values for UART and the connection to SIM800
    Serial.begin(BAUD);
    sim800Serial.begin(9600);

    // edit APN settings in config.h
    sim800.setGPRSNetworkSettings(F(SIM800_APN), F(SIM800_USER), F(SIM800_PASS));

    blink(5, 1000);
    Serial.println("WELCOME!");

    if (! musicPlayer.begin()) { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
        error(HALLO_ERROR_AUDIO);
    }

    if(!SD.begin(CARDCS)) {  // initialise the SD card
        Serial.println(F("SDCARD not found"));
        error(HALLO_ERROR_SDCARD);
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// the main loop just reads the responses from the modem and
// writes them to the serial port
void loop() {

    File f = SD.open("SHORT.OGG");
    unsigned long size = f.size();
    if(size > 0) {
        Serial.print(f.name());
        Serial.print(" (");
        Serial.print(size);
        Serial.println(")");
    }

    sim800.wakeup();
    sim800.ensureConnection();

    if(sim800.TCPconnect("api.ubirch.com", 23456)) {
        Serial.println("Connection established.");
        int n = 0;
        while((n = f.readBytes(buffer, BUF_SIZE-1)) != 0) {
            uint16_t checksum = fletcher16((uint8_t *) buffer, (size_t) n);
            Serial.print(n); Serial.print(" "); Serial.println(checksum, 16);
            sim800.TCPsend(buffer, n);
        };
        f.close();
    } else {
        Serial.println("Connection could not be established.");
    };
    sim800.TCPclose();

    delay(30000);
    sim800.shutdown();

    Serial.println("FINISHED");
    error(0);
}

#pragma clang diagnostic pop
