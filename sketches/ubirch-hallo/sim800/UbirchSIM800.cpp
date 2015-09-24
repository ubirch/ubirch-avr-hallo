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

#include <Arduino.h>
#include <MinimumSerial.h>
#include "UbirchSIM800.h"

extern MinimumSerial debug;

// show debug output only in non-release mode
#ifndef NDEBUG
#   define PRINT(s) debug.print(F(s))
#   define PRINTLN(s) debug.println(F(s))
#   define DEBUG(s) debug.print("'"); debug.print(s); debug.print("'")
#   define DEBUGLN(s) debug.print("'"); debug.print(s); debug.println("'")
#else
#   define PRINT(s)
#   define PRINTLN(s)
#   define DEBUG(s)
#   define DEBUGLN(s)
#endif

#define println_param(prefix, p) print(F(prefix)); print(F(",\"")); print(p); println(F("\""));

UbirchSIM800::UbirchSIM800() {
}

bool UbirchSIM800::reset() {
    _serial.begin(19200);

    pinMode(SIM800_RST, OUTPUT);
    digitalWrite(SIM800_RST, HIGH);
    delay(10);
    digitalWrite(SIM800_RST, LOW);
    delay(100);
    digitalWrite(SIM800_RST, HIGH);

    delay(7000);

    while (_serial.available()) _serial.read();

    expect_OK(F("ATE0"));
    bool ok = expect_OK(F("ATE0"));

    expect_OK(F("AT+IFC=0,0")); // No hardware flow control
    expect_OK(F("AT+CIURC=0")); // No "Call Ready"
    while (_serial.available()) _serial.read();

    return ok;
}

void UbirchSIM800::setAPN(const __FlashStringHelper *apn, const __FlashStringHelper *user,
                          const __FlashStringHelper *pass) {
    _apn = apn;
    _user = user;
    _pass = pass;
}

unsigned int UbirchSIM800::readline(char *buffer, size_t max, uint16_t timeout) {
    uint16_t idx = 0;
    for (; timeout--;) {
        while (_serial.available()) {
            char c = (char) _serial.read();
            if (c == '\r') continue;
            if (c == '\n') {
                if (!idx) continue;
                timeout = 0;
                break;
            }
            if (idx < max - 1) buffer[idx++] = c;
        }

        if (timeout == 0) break;
        delay(1);
    }
    buffer[idx] = 0;
    return idx;
};

void UbirchSIM800::eatEcho() {
    while (_serial.available()) {
        _serial.read();
        delay(1);
    }
}

void UbirchSIM800::print(const __FlashStringHelper *s) {
    PRINT(">>> ");
    DEBUGLN(s);
    _serial.print(s);
}

void UbirchSIM800::print(uint16_t s) {
    PRINT(">>> ");
    DEBUGLN(s);
    _serial.print(s);
}


void UbirchSIM800::print(const char *s) {
    PRINT(">>> ");
    DEBUGLN(s);
    _serial.print(s);
}

void UbirchSIM800::println(const __FlashStringHelper *s) {
    PRINT(">>> ");
    DEBUGLN(s);
    _serial.print(s);
    eatEcho();
    _serial.println();
}

void UbirchSIM800::println(uint16_t s) {
    PRINT(">>> ");
    DEBUGLN(s);
    _serial.print(s);
    eatEcho();
    _serial.println();
}

void UbirchSIM800::println(const char *s) {
    PRINT(">>> ");
    DEBUGLN(s);
    _serial.print(s);
    eatEcho();
    _serial.println();
}

bool UbirchSIM800::expect(const __FlashStringHelper *expected, uint16_t timeout) {
    char buf[100];
    unsigned int len = readline(buf, 100, timeout);
    PRINT("<<< (");
    DEBUG(len);
    PRINT(") ");
    DEBUGLN(buf);
    return strcmp_P(buf, (char PROGMEM *) expected) == 0;
}

bool UbirchSIM800::expect(const __FlashStringHelper *cmd, const __FlashStringHelper *expected, uint16_t timeout) {
    println(cmd);
    return expect(expected, timeout);
}

bool UbirchSIM800::expect_OK(uint16_t timeout) {
    return expect(F("OK"), timeout);
}

bool UbirchSIM800::expect_OK(const __FlashStringHelper *cmd, uint16_t timeout) {
    return expect(cmd, F("OK"), timeout);
}

bool UbirchSIM800::expect_scan(const __FlashStringHelper *pattern, void *ref, uint16_t timeout) {
    PRINTLN(">>1<<");
    char buf[100];
    unsigned int len = readline(buf, 100, timeout);
    PRINT("<<< (");
    DEBUG(len);
    PRINT(") ");
    DEBUGLN(buf);
    return sscanf_P(buf, (const char PROGMEM *) pattern, ref) == 1;
}

bool UbirchSIM800::expect_scan(const __FlashStringHelper *pattern, void *ref, void *ref1, uint16_t timeout) {
    PRINTLN(">>2<<");
    char buf[100];
    unsigned int len = readline(buf, 100, timeout);
    PRINT("<<< (");
    DEBUG(len);
    PRINT(") ");
    DEBUGLN(buf);
    return sscanf_P(buf, (char PROGMEM *) pattern, ref, ref1) == 1;
}


bool UbirchSIM800::wakeup() {
    PRINTLN("!!! SIM800 wakeup");

    // check if the chip is already awake, otherwise start wakeup
    if (!expect_OK(F("AT"), 5000)) {
        PRINTLN("!!! using PWRKEY wakeup procedure");
        pinMode(SIM800_KEY, OUTPUT);
        do {
            digitalWrite(SIM800_KEY, HIGH);
            delay(10);
            digitalWrite(SIM800_KEY, LOW);
            delay(1100);
            digitalWrite(SIM800_KEY, HIGH);
            delay(2000);
        } while (digitalRead(SIM800_PS) == LOW);
        // make pin unused (do not leak)
        pinMode(SIM800_KEY, INPUT_PULLUP);
        PRINTLN("!!! SIM800 ok");
    } else {
        PRINTLN("!!! SIM800 already awake");
    }

    return reset();
}

bool UbirchSIM800::shutdown() {
    PRINTLN("!!! SIM800 shutdown");

    disableGPRS();
    if (!expect(F("AT+CPOWD=1"), F("NORMAL POWER DOWN"), 5000)) {
        if (digitalRead(SIM800_PS) == HIGH) {
            PRINTLN("shutdown() using PWRKEY, AT+CPOWD=1 failed");
            pinMode(SIM800_KEY, OUTPUT);
            digitalWrite(SIM800_KEY, LOW);
            for (; digitalRead(SIM800_KEY) == LOW;);
            digitalWrite(SIM800_KEY, HIGH);
            pinMode(SIM800_KEY, INPUT_PULLUP);
        } else {
            PRINTLN("shutdown(): already shut down");
        }
    }
    PRINTLN("shutdown(): ok");
    return true;
}

bool UbirchSIM800::registerNetwork() {
    PRINTLN("!!! waiting for network registration");
    uint8_t n = 0;
    do {
        println(F("AT+CREG?"));
        expect_scan(F("+CREG: 0,%d"), &n);
        switch (n) {
            case 0:
                PRINTLN("_");
                break;
            case 1:
                PRINTLN("H");
                break;
            case 2:
                PRINTLN("S");
                break;
            case 3:
                PRINTLN("D");
                break;
            case 4:
                PRINTLN("?");
                break;
            case 5:
                PRINTLN("R");
                break;
            default:
            DEBUGLN(n);
                break;
        }
        delay(1000);
    } while (!(n == 1 || n == 5));
    return true;
}

bool UbirchSIM800::enableGPRS(uint16_t timeout) {
    expect(F("AT+CIPSHUT"), F("SHUT OK"), 5000);
    expect_OK(F("AT+CIPMUX=1")); // enable multiplex mode
    expect_OK(F("AT+CIPRXGET=1")); // we will receive manually

    bool attached = false;
    for (; !attached && --timeout;) {
        attached = expect_OK(F("AT+CGATT=1"), 10000);
        delay(100);
    }
    if (!attached) return false;

    if (!expect_OK(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), 10000)) return false;

    // set bearer profile access point name
    if (_apn) {
        print(F("AT+SAPBR=3,1,\"APN\",\""));
        print(_apn);
        println(F("\""));
        if (!expect_OK()) return false;

        if (_user) {
            print(F("AT+SAPBR=3,1,\"USER\",\""));
            print(_user);
            println(F("\""));
            if (!expect_OK()) return false;
        }
        if (_pass) {
            print(F("AT+SAPBR=3,1,\"PWD\",\""));
            print(_pass);
            println(F("\""));
            if (!expect_OK()) return false;
        }
    }

    // open GPRS context
    expect_OK(F("AT+SAPBR=1,1"), 30000);

    uint16_t gprsState;
    do {
        println(F("AT+CGATT?"));
        expect_scan(F("+CGATT: %d"), &gprsState);
        delay(1);
    } while (timeout-- && !gprsState);

    return gprsState != 0;
}

bool UbirchSIM800::disableGPRS() {
    expect(F("AT+CIPSHUT"), F("SHUT OK"));
    if (!expect_OK(F("AT+SAPBR=0,1"))) return false;

    return expect_OK(F("AT+CGATT=0"));
}

uint16_t UbirchSIM800::GET(char *url, size_t &length) {
    expect_OK(F("AT+HTTPTERM"));
    delay(100);

    if (!expect_OK(F("AT+HTTPINIT"))) return 1000;
    if (!expect_OK(F("AT+HTTPPARA=\"CID\",1"))) return 1001;
    if (!expect_OK(F("AT+HTTPPARA=\"UA\",\"UBIRCH#1\""))) return 1002;
    if (!expect_OK(F("AT+HTTPPARA=\"REDIR\",1"))) return 1003;
    println_param("AT+HTTPPARA=\"URL\"", url);
    if (!expect_OK()) return 1003;

    if (!expect_OK(F("AT+HTTPACTION=0"))) return 1004;
    uint16_t s, l;
    expect_scan(F("+HTTPACTION: 0,%d,%d"), &s, &l, 60000);
    DEBUGLN(l);
    length = l;

    PRINT("!!! HTTP: ");
    DEBUG(s);
    PRINT(" - ");
    DEBUGLN(length);

    return s;
}

uint16_t UbirchSIM800::GETReadPayload(char *buffer, uint16_t start, size_t length) {
    print(F("AT+HTTPREAD="));
    print(start);
    print(F(","));
    println(length);

    size_t expectedLength;
    expect_scan(F("+HTTPREAD: %d"), &expectedLength);
    PRINT("~~~ PACKET: "); DEBUGLN(expectedLength);

    size_t idx = 0;
    while (expectedLength && (idx < length)) {
        if (_serial.available()) {
            buffer[idx] = (char)_serial.read();
            idx++;
            expectedLength--;
        }
    }
    expect_OK();
    PRINT("~~~ DONE "); DEBUGLN(idx);
    return idx;
}

bool UbirchSIM800::connect(char *address, uint16_t port, uint16_t timeout) {
    // bring connection up, force it
    print(F("AT+CSTT=\""));
    print(_apn);
    println(F("\""));
    if (!expect_OK()) return false;

    if (!expect_OK(F("AT+CIICR"))) return false;

    // try five times to get an IP address
    bool connected = false;
    do {
        char ipaddress[23];
        println(F("AT+CIFSR"));
        expect_scan(F("%s"), &ipaddress);
        connected = strcmp_P(ipaddress, PSTR("ERROR")) != 0;
        if (!connected) delay(1);
    } while (timeout-- && !connected);

    return connected;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void UbirchSIM800::test() {
    for (; ;) {
        char ipaddress[23] = "";
        println(F("AT+CIFSR"));
        expect_scan(F("%s"), ipaddress);
        DEBUGLN(ipaddress);
        for (uint8_t i = 0; i < sizeof(ipaddress); i++) {
            debug.print(ipaddress[i], HEX);
            debug.print(" ");
        }
        debug.println();
        delay(3000);
    }
}

#pragma clang diagnostic pop