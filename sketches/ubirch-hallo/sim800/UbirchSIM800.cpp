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

extern MinimumSerial minimumSerial;

//#define NDEBUG

// show minimumSerial output only in non-release mode
#ifndef NDEBUG
#   define PRINT(s) minimumSerial.print(F(s))
#   define PRINTLN(s) minimumSerial.println(F(s))
#   define DEBUG(...) minimumSerial.print(__VA_ARGS__)
#   define DEBUGQ(...) minimumSerial.print("'"); minimumSerial.print(__VA_ARGS__); minimumSerial.print("'")
#   define DEBUGLN(...) minimumSerial.println(__VA_ARGS__)
#   define DEBUGQLN(...) minimumSerial.print("'"); minimumSerial.print(__VA_ARGS__); minimumSerial.println("'")
#else
#   define PRINT(s)
#   define PRINTLN(s)
#   define DEBUG(...)
#   define DEBUGQ(...)
#   define DEBUGLN(...)
#   define DEBUGQLN(...)
#endif

#define println_param(prefix, p) print(F(prefix)); print(F(",\"")); print(p); println(F("\""));

UbirchSIM800::UbirchSIM800() {
}

bool UbirchSIM800::reset() {
    return reset(_serialSpeed);
}

bool UbirchSIM800::reset(uint32_t serialSpeed) {
    _serial.begin(_serialSpeed);

    pinMode(SIM800_RST, OUTPUT);
    digitalWrite(SIM800_RST, HIGH);
    delay(10);
    digitalWrite(SIM800_RST, LOW);
    delay(100);
    digitalWrite(SIM800_RST, HIGH);

    delay(7000);

    while (_serial.available()) _serial.read();

    expect_OK(F("AT"));
    expect_OK(F("AT"));
    expect_OK(F("AT"));
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
    //PRINT("readline: timeout="); DEBUGLN(timeout);
    uint16_t idx = 0;
    while (--timeout) {
        //if((timeout % 1000) == 0) DEBUG("=");
        while (_serial.available()) {
            char c = (char) _serial.read();
            if (c == '\r') {
                //PRINTLN("(CR)");
                continue;
            }
            if (c == '\n') {
                //PRINTLN("(NL)");
                if (!idx) continue;
                timeout = 0;
                break;
            }
            //DEBUG(c, HEX);
            if (idx < max - 1) buffer[idx++] = c;
        }

        if (timeout == 0) {
            //PRINTLN("(TIMEOUT)");
            break;
        }
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
    DEBUGQLN(s);
    _serial.print(s);
}

void UbirchSIM800::print(uint16_t s) {
    PRINT(">>> ");
    DEBUGLN(s);
    _serial.print(s);
}


void UbirchSIM800::print(const char *s) {
    PRINT(">>> ");
    DEBUGQLN(s);
    _serial.print(s);
}

void UbirchSIM800::println(const __FlashStringHelper *s) {
    PRINT(">>> ");
    DEBUGQLN(s);
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
    DEBUGQLN(s);
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
    DEBUGQLN(buf);
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
    char buf[100];
    unsigned int len = readline(buf, 100, timeout);
    PRINT("<<< (");
    DEBUG(len);
    PRINT(") ");
    DEBUGQLN(buf);
    return sscanf_P(buf, (const char PROGMEM *) pattern, ref) == 1;
}

bool UbirchSIM800::expect_scan(const __FlashStringHelper *pattern, void *ref, void *ref1, uint16_t timeout) {
    char buf[100];
    unsigned int len = readline(buf, 100, timeout);
    PRINT("<<< (");
    DEBUG(len);
    PRINT(") ");
    DEBUGQLN(buf);
    return sscanf_P(buf, (char PROGMEM *) pattern, ref, ref1) == 2;
}

bool UbirchSIM800::expect_scan(const __FlashStringHelper *pattern, void *ref, void *ref1, void *ref2,
                               uint16_t timeout) {
    char buf[100];
    unsigned int len = readline(buf, 100, timeout);
    PRINT("<<< (");
    DEBUG(len);
    PRINT(") ");
    DEBUGQLN(buf);
    return sscanf_P(buf, (char PROGMEM *) pattern, ref, ref1, ref2) == 1;
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

bool UbirchSIM800::registerNetwork(uint16_t timeout) {
    PRINTLN("!!! waiting for network registration");
    uint8_t n = 0;
    expect_OK(F("AT"));
    timeout = timeout / 1000;
    for (; --timeout;) {
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
        if ((n == 1 || n == 5)) return true;
        delay(1000);
    }
    return false;
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

uint16_t UbirchSIM800::GET(const char *url, uint32_t &length) {
    expect_OK(F("AT+HTTPTERM"));
    delay(100);

    if (!expect_OK(F("AT+HTTPINIT"))) return 1000;
    if (!expect_OK(F("AT+HTTPPARA=\"CID\",1"))) return 1001;
    if (!expect_OK(F("AT+HTTPPARA=\"UA\",\"UBIRCH#1\""))) return 1002;
    if (!expect_OK(F("AT+HTTPPARA=\"REDIR\",1"))) return 1003;
    println_param("AT+HTTPPARA=\"URL\"", url);
    if (!expect_OK()) return 1003;

    if (!expect_OK(F("AT+HTTPACTION=0"))) return 1004;
    uint16_t s;
    expect_scan(F("+HTTPACTION: 0,%d,%lu"), &s, &length, 60000);

    PRINT("!!! HTTP: ");
    DEBUG(s);
    PRINT(" - ");
    DEBUGLN(length);

    return s;
}

size_t UbirchSIM800::GETReadPayload(char *buffer, uint32_t start, size_t length) {
    print(F("AT+HTTPREAD="));
    print(start);
    print(F(","));
    println(length);

    uint16_t available;
    expect_scan(F("+HTTPREAD: %lu"), &available);
    PRINT("~~~ PACKET: ");
    DEBUGLN(available);

    size_t idx = read(buffer, available);
    if(!expect_OK()) return 0;

    PRINT("~~~ DONE: ");
    DEBUGLN(idx);
    return idx;
}

inline size_t UbirchSIM800::read(char *buffer, size_t length) {
    uint32_t idx = 0;
    PRINT("expected: "); DEBUGLN(length);
    while (length) {
        while (_serial.available()) {
            buffer[idx++] = (char) _serial.read();
            length--;
            DEBUG(buffer[idx-1]);
        }
    }
    PRINT("GOT "); DEBUGLN(idx);
    return idx;
}

bool UbirchSIM800::connect(const char *address, uint16_t port, uint16_t timeout) {
    if (!expect(F("AT+CIPSHUT"), F("SHUT OK"))) return false;
    if (!expect_OK(F("AT+CMEE=2"))) return false;
    if (!expect_OK(F("AT+CIPQSEND=1"))) return false;

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
        expect_scan(F("%s"), ipaddress);
        connected = strcmp_P(ipaddress, PSTR("ERROR")) != 0;
        if (!connected) delay(1);
    } while (timeout-- && !connected);

    if (!connected) return false;

    print(F("AT+CIPSTART=0,\"TCP\",\""));
    print(address);
    print(F("\",\""));
    print(port);
    println(F("\""));
    if (!expect_OK()) return false;
    if (!expect(F("0, CONNECT OK"), 30000)) return false;

    return connected;
}

bool UbirchSIM800::status() {
    println(F("AT+CIPSTATUS=0"));

    char status[80];
    expect_scan(F("+CIPSTATUS: %s"), status);
    DEBUGLN(status);
    if (!expect_OK()) return false;

    return strcmp_P(status, PSTR("CONNECTED")) < 0;
}

bool UbirchSIM800::disconnect() {
    return expect_OK(F("AT+CIPCLOSE=0"));
};

bool UbirchSIM800::send(char *buffer, size_t size, size_t &accepted) {
    print(F("AT+CIPSEND=0,"));
    println(size);

    if (!expect(F("> "))) return false;
    _serial.write(buffer, size);

    uint32_t a;
    if(!expect_scan(F("DATA ACCEPT: 0,%lu"), &a, 3000)) {
        // we have a buffer of 319488 bytes, so we are optimistic,
        // even if a temporary fail occurs and just carry on
        // (verified!)
        //return false;
    }

    return a == size;
}

size_t UbirchSIM800::receive(char *buffer, size_t size) {
    size_t actual;
    for(actual = 0; actual < size;) {
        uint8_t chunk = min(size - actual, 128);
        print(F("AT+CIPRXGET=2,0,"));
        println(chunk);

        uint32_t requested, confirmed;
        if(!expect_scan(F("+CIPRXGET: 2,%*d,%lu,%u"), &requested, &confirmed)) return 0;

        actual += read(buffer, confirmed);
    }

    return actual;
}

/*

AT+SAPBR=3,1,"APN","internet.eplus.de"
AT+SAPBR=3,1,"USER","eplus"
AT+SAPBR=3,1,"PWD","grps"
AT+CIPSHUT
AT+CIPMUX=1
AT+CSTT="internet.eplus.de"
AT+CIICR
AT+CIFSR
AT+CIPSTART=0,"TCP","api.ubirch.com","23456"
AT+CMEE=2
AT+CIPQSEND=0
AT+CIPSEND=0,10

 */