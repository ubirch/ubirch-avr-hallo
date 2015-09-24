/**
 * ...
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

#ifndef UBIRCH_SIM800_H
#define UBIRCH_SIM800_H

// SIM800H settings
#include <stdint.h>
#include <SoftwareSerial.h>

#define SIM800_RX    2
#define SIM800_TX    3
#define SIM800_RST   4
#define SIM800_KEY   7
#define SIM800_PS    8

#define DEFAULT_SERIAL_TIMEOUT 1000

class UbirchSIM800 {

private:

    void print(const __FlashStringHelper *s);

    void print(const char *s);

    void print(uint16_t s);

    void println(const __FlashStringHelper *s);

    void println(const char *s);


    void println(uint16_t s);

protected:
    SoftwareSerial _serial = SoftwareSerial(SIM800_TX, SIM800_RX);
    const __FlashStringHelper *_apn;
    const __FlashStringHelper *_user;
    const __FlashStringHelper *_pass;

    unsigned int readline(char *buffer, size_t max, uint16_t timeout);

    void eatEcho();

    bool expect(const __FlashStringHelper *expected, uint16_t timeout);

    bool expect(const __FlashStringHelper *cmd, const __FlashStringHelper *expected,
                uint16_t timeout = DEFAULT_SERIAL_TIMEOUT);

    bool expect_OK(uint16_t timeout = DEFAULT_SERIAL_TIMEOUT);

    bool expect_OK(const __FlashStringHelper *cmd, uint16_t timeout = DEFAULT_SERIAL_TIMEOUT);

    bool expect_scan(const __FlashStringHelper *pattern, void *ref, uint16_t timeout = DEFAULT_SERIAL_TIMEOUT);

    bool expect_scan(const __FlashStringHelper *pattern, void *ref, void *ref1,
                     uint16_t timeout = DEFAULT_SERIAL_TIMEOUT);

public:
    UbirchSIM800();

    void test();

    void setAPN(const __FlashStringHelper *apn, const __FlashStringHelper *user, const __FlashStringHelper *pass);

    bool reset();

    bool shutdown();

    bool wakeup();

    bool registerNetwork();

    bool enableGPRS(uint16_t timeout = 30000);

    bool disableGPRS();

    bool connect(char *address, uint16_t port, uint16_t timeout = 30000);

    uint16_t GET(char *url, size_t& response);

    uint16_t GETReadPayload(char *buffer, uint16_t start, uint16_t length);
};

#endif //UBIRCH_FONA_H
