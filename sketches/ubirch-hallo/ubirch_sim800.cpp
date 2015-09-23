/**
 * UbirchSIM800 is a class that extends the FONA code from Adafruit
 * to add support for shutdown and wakeup of the chip to make it
 * reliable to work with.
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

#include "ubirch_sim800.h"

// show debug output only in non-release mode
#ifndef NDEBUG
#   define D_println(s) Serial.print("SIM800: "); Serial.println(s)
#   define D_newline() Serial.println();
#   define D_print(s) Serial.print(s)
#else
#   define D_println(s)
#   define D_newline()
#   define D_print(s)
#endif

UbirchSIM800::UbirchSIM800(Stream *serial, uint8_t rst, uint8_t key, uint8_t ps) : Adafruit_FONA::Adafruit_FONA(rst) {
    _keypin = key;
    _pspin = ps;
    mySerial = serial;
}

void UbirchSIM800::wakeup() {
    D_println(F("wakeup()"));
    // check if the chip is already awake, otherwise start wakeup
    if (!sendCheckReply(F("AT"), F("OK"), 5000)) {
        D_println(F("wakeup(): using PWRKEY wakeup procedure"));
        pinMode(_keypin, OUTPUT);
        do {
            digitalWrite(_keypin, HIGH);
            delay(10);
            digitalWrite(_keypin, LOW);
            delay(1100);
            digitalWrite(_keypin, HIGH);
            delay(2000);
            D_println(digitalRead(_pspin) ? '!' : '.');
        } while (digitalRead(_pspin) == LOW);
        // make pin unused (do not leak)
        pinMode(_keypin, INPUT_PULLUP);
        D_println(F("wakeup(): ok"));
    } else {
        D_println(F("wakeup(): already awake"));
    }

    if (begin(*mySerial)) {
        D_println(F("wakeup(): initialized"));
    } else {
        D_println(F("wakeup(): can't initialize"));
    }

}

void UbirchSIM800::shutdown() {
    D_println("shutdown()");
    // sendCheckReply() doesn't catch the NORMAL POWER DOWN sequence, possibly due to a missing newline?
    getReply(F("AT+CPOWD=1"), (uint16_t) 5000);
    if (strncmp(replybuffer, "NORMAL POWER DOWN", 17) != 0) {
        if (digitalRead(_pspin) == HIGH) {
            D_println(F("shutdown() using PWRKEY, AT+CPOWD=1 failed"));
            pinMode(_keypin, OUTPUT);
            do { digitalWrite(_keypin, LOW); } while (digitalRead(_pspin) == HIGH);
            digitalWrite(_keypin, HIGH);
            pinMode(_keypin, INPUT_PULLUP);
        } else {
            D_println("shutdown(): already shut down");
        }
    }
    D_println(F("shutdown(): ok"));
}

boolean UbirchSIM800::ensureConnection() {
    D_print(F("NETWORK: "));
    uint8_t n;
    do {
        n = getNetworkStatus();  // Read the Network / Cellular Status
        switch (n) {
            case 0:
                D_print(F("_"));
                break;
            case 1:
                D_print(F("H"));
                break;
            case 2:
                D_print(F("S"));
                break;
            case 3:
                D_print(F("D"));
                break;
            case 4:
                D_print(F("?"));
                break;
            case 5:
                D_print(F("R"));
                break;
            default:
                D_print(n);
                break;
        }
        delay(1000);
    } while (!(n == 1 || n == 5));
    D_newline();

    D_print(F("GPRS: "));
    boolean connected = false;
    for(uint8_t c = 0; !connected && c < 100; c++) {
        enableGPRS(true);
        while (GPRSstate() == 0) {
            D_print(F("."));
            delay(1000);
        }
        D_newline();
        // bring connection up, force it
        sendCheckReplyQuoted(F("AT+CSTT="), apn, F("OK"));
        sendCheckReply(F("AT+CIICR"), F("OK"));

        // try five times to get an IP address
        for(uint8_t i = 0; !connected && i < 5; i++) {
            connected = !sendCheckReply(F("AT+CIFSR"), F("ERROR"));
            if (!connected) delay(2000);
        }
    }

    return true;
}
