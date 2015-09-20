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
#include <SoftwareSerial.h>

#include <Adafruit_FONA.h>

#include "config.h"
#include "UbirchSIM800.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_KEY 7
#define FONA_PS 8

#define PIN_LED 13
#define PIN_WATCHDOG 6

// show debug output only in non-release mode
#ifndef NDEBUG
#   define DEBUG(s) Serial.println(F(s))
#else
#   define DEBUG(s)
#endif

// set serial port baud if undefined
#ifndef BAUD
#   define BAUD 115200
#endif

SoftwareSerial softwareSerial = SoftwareSerial(FONA_TX, FONA_RX);
UbirchSIM800 sim800 = UbirchSIM800(FONA_RST, FONA_KEY, FONA_PS);

void blink(uint8_t n, long speed) {
    digitalWrite(PIN_LED, LOW);
    for (uint8_t i = n * 2; i > 0; i--) {
        Serial.print('.');
        digitalWrite(PIN_LED, i % 2 == 0 ? HIGH : LOW);
        delay(speed);
    }
    digitalWrite(PIN_LED, LOW);
    Serial.println();
}

void setup() {
    Serial.begin(BAUD);
    DEBUG("###");

    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_WATCHDOG, OUTPUT);

    blink(10, 500);

    softwareSerial.begin(9600);

    // edit APN settings in config.h
    sim800.setGPRSNetworkSettings(F(FONA_APN), F(FONA_USER), F(FONA_PASS));
    sim800.begin(softwareSerial);

/*

    cli();

    // reset the timer registers
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    OCR1A = 16000000UL / 256 - 1;
    TCCR1B |= _BV(CS01); // prescale 8 selected (still fast enough)
    TCCR1B |= _BV(WGM12); // CTC mode
    TIMSK1 |= _BV(OCIE1A); // timer compare interrupt

    sei();
*/
}

// read what is available from the serial port and send to modem
//ISR(TIMER1_COMPA_vect) {
//    while (Serial.available() > 0) sim800h.write((uint8_t) Serial.read());
//}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

// the main loop just reads the responses from the modem and
// writes them to the serial port
void loop() {

    DEBUG("STARTING TEST PROCEDURE");
    for (int i = 0; i < 5; i++) {
        sim800.wakeup();

        uint8_t n;
        do {
            n = sim800.getNetworkStatus();  // Read the Network / Cellular Status
            Serial.print(F("Network status "));
            Serial.print(n);
            Serial.print(F(": "));
            switch (n) {
                case 0:
                    Serial.println(F("Not registered"));
                    break;
                case 1:
                    Serial.println(F("Registered (home)"));
                    break;
                case 2:
                    Serial.println(F("Not registered (searching)"));
                    break;
                case 3:
                    Serial.println(F("Denied"));
                    break;
                case 4:
                    Serial.println(F("Unknown"));
                    break;
                case 5:
                    Serial.println(F("Registered roaming"));
                    break;
                default:
                    Serial.println(F("???"));
                    break;
            }
            delay(1000);
        } while (!(n == 1 || n == 5));

        if (sim800.sendCheckReply(F("AT+SAPBR=3,1,\"APN\",\""
                                            FONA_APN
                                            "\""), F("OK"), 10000))
            DEBUG("APN OK");
        if (sim800.sendCheckReply(F("AT+SAPBR=3,1,\"USER\",\""
                                            FONA_USER
                                            "\""), F("OK"), 10000))
            DEBUG("APN USER OK");
        if (sim800.sendCheckReply(F("AT+SAPBR=3,1,\"PWD\",\""
                                            FONA_PASS
                                            "\""), F("OK"), 10000))
            DEBUG("APN PASSWORD OK");
        sim800.sendCheckReply(F("AT&W"), F("OK"));

        sim800.enableGPRS(true);
        while (sim800.GPRSstate() == 0) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        Serial.println("GPRS OK");

        // check again that our GPRS connection is valid, try reconnect if not
        while(!sim800.sendCheckReply(F("AT+SAPBR=1,1"), F("OK"))) {
            sim800.sendCheckReply(F("AT+SAPBR=0,1"), F("OK"));
            delay(500);
        }

        sim800.HTTP_init();
        uint16_t status, len;
        char *url = (char *) "http://thingspeak.ubirch.com/update?key=1234567890&batt=50";
        sim800.HTTP_GET_start(url, &status, &len);
        sim800.HTTP_GET_end();
        Serial.print("STATUS: ");
        Serial.println(status);
        Serial.print("LENGTH: ");
        Serial.println(len);

        // this will not work on your device (the specific IP of my SIM is whitelisted on that server!)
        sim800.sendCheckReply(F("AT+SMTPSRV=mail.jugel.info,25"), F("OK"));
        sim800.sendCheckReply(F("AT+SMTPFROM=leo@ubirch.com,leo"), F("OK"));
        sim800.sendCheckReply(F("AT+SMTPRCPT=0,0,trigger@recipe.ifttt.com"), F("OK"));
        sim800.sendCheckReply(F("AT+SMTPSUB=ubirch no1 here"), F("OK"));
        sim800.sendCheckReply(F("AT+SMTPBODY=12"), F("DOWNLOAD"));
        sim800.sendCheckReply(F("Hello Alice!"), F("OK"));
        sim800.sendCheckReply(F("AT+SMTPSEND"), F("OK"));

        delay(30000);
        if (i % 2 == 0) sim800.shutdown();
    }

    DEBUG("TEST PROCEDURE DONE");
    while (1);
}

#pragma clang diagnostic pop
