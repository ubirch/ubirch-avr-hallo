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
#include <avr/wdt.h>
#include "main.h"
#include "sim800/UbirchSIM800.h"
#include "vs1053/Adafruit_VS1053_FilePlayer.h"

extern "C" {
#   include "i2c/i2c.h"
#   include "mpr121/mpr121.h"
#   include "ws2812/ws2812.h"
}

UbirchSIM800 sim800 = UbirchSIM800();
Adafruit_VS1053_FilePlayer vs1053 =
        Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

#define STATE_COLOR(r, g, b)  (state.red=(r),state.green=(g),state.blue=(b), sin_start=4.712)

struct state_t {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t pulse : 1;
    uint8_t message : 1;
};

volatile state_t state;
long timer = 10000;
volatile float sin_start = 4.712;

bool sendFile(const char *fname, uint8_t retries);

bool receiveFile(const char *fname);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

inline void halt(uint8_t c) {
    while (1) {
        DEBUG(c);
        _delay_ms(1000);
    };
}

#pragma clang diagnostic pop

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

inline void disable_pulse() {
    TIMSK1 &= ~(1 << OCIE1A);  // enable timer compare interrupt
}

inline void enable_pulse() {
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
}

void setup() {
    // disable all the watchdogs
    wdt_disable();
    disable_watchdog();

    // setup interrupt that controls our pulsing light
    cli();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    OCR1A = (F_CPU / 256 / 10);  // compare match register 16MHz/256/xxHz
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

    // set output register for LEDs
    WS2812_DO_DDR |= _BV(WS2812_DO_BIT);

    sei();

    // configure the initial values for UART and the connection to SIM800
    minimumSerial.begin((uint32_t) 115200);

    blink(3, 1000);

    STATE_COLOR(255, 0, 0);
    state.pulse = 1;

    PRINTLN("SIM800 wakeup");
    if (!sim800.wakeup()) halt(11);
    sim800.setAPN(F(SIM800_APN), F(SIM800_USER), F(SIM800_PASS));

    STATE_COLOR(255, 128, 0);
    PRINTLN("SIM800 waiting for network registration");
    while (!sim800.registerNetwork()) {
        sim800.shutdown();
        sim800.wakeup();
    }

    STATE_COLOR(255, 255, 0);
    PRINTLN("SIM800 enabling GPRS");
    if (!sim800.enableGPRS()) halt(22);
    PRINTLN("SIM800 initialized");


    // == SETUP ===============================================
    if (!SD.begin(CARDCS)) {
        PRINTLN("SDCARD not found");
        halt(33);
    }
    PRINTLN("SDCARD initialized");

    if (!vs1053.begin()) {
        PRINTLN("VS1053 not found");
        halt(44);
    }
    PRINTLN("VS1053 initialized ");

    i2c_init(I2C_SPEED_400KHZ);
    if (!mpr_reset()) {
        PRINTLN("MPR121 not found");
        halt(55);
    }
    PRINTLN("MPR121 initialized");

    STATE_COLOR(0,0,0);
}

ISR(TIMER1_COMPA_vect) {
    if (state.pulse) {
        sin_start = sin_start + 0.1;
        if (sin_start > 10.995) sin_start = 4.712;


        float factor = sin(sin_start);
        uint8_t rh = state.red / 2;
        uint8_t gh = state.green / 2;
        uint8_t bh = state.blue / 2;

        UCSR0B &= ~_BV(RXEN0);
        uint8_t buffer[8];
        WS2812_compileRGB(buffer,
                          (const uint8_t) (factor * rh + rh),
                          (const uint8_t) (factor * gh + gh),
                          (const uint8_t) (factor * bh + bh));
        WS2812_DO_DDR |= _BV(WS2812_DO_BIT);
        WS2812_transmit_precompiled_sequence(buffer, sizeof(buffer), 8 * 7);
        _delay_us(58);
        UCSR0B |= _BV(RXEN0);
    }
}

uint8_t play(const char *fname) {
    vs1053.setVolume(1, 1);
    vs1053.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
    vs1053.startPlayingFile(fname);

    uint8_t unfinished = 0;
    while (vs1053.playingMusic) {
        if (!(mpr_status() & _BV(0))) {
            unfinished = 1;
            vs1053.stopPlaying();
        }
        _delay_ms(100);
    }
    // TODO hack to disable the timer interrupt
    TIMSK0 &= ~_BV(OCIE0A);

    STATE_COLOR(0,0,128);
    while(mpr_status() & _BV(0)) _delay_ms(20);
    STATE_COLOR(0,0,0);

    return unfinished;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"


void loop() {
    // check if the sensor 0 was touched
    if ((mpr_status() & _BV(0))) {
        state.pulse = 1;

        if (state.message) {
            STATE_COLOR(0, 0, 0);
            PRINTLN("playing downloaded message");
            state.message = play("m.ogg");
            // set color to black if we played the whole message
            if (state.message) STATE_COLOR(0, 255, 0);
        } else {
            PRINTLN("recording message");
//            record("r.ogg");
            PRINTLN("sending recorded message");
//            sendFile("r.ogg", 3);
            STATE_COLOR(0, 0, 0);
        }
    } else {
        if (--timer < 0) {
            timer = 30 * 60000;
            // if no message is avaiable, check regularly for a new message
            if (!state.message) {
                disable_pulse();
                if (receiveFile("m.ogg")) {
                    STATE_COLOR(0,255,0);
                    state.pulse = 1;
                    state.message = 1;
                }
                enable_pulse();
            }
        }
        if (timer % 1000 == 0) minimumSerial.print(state.message ? "!" : ".");
    }
}

#pragma clang diagnostic pop

bool sendFile(const char *fname, uint8_t retries) {
    uint16_t status;
    uint32_t length;

    do {
        char date[10], time[10], tz[5];
        sim800.time(date, time, tz);
        PRINT("START: ");
        DEBUGLN(time);

        SD.cacheClear();
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
    uint16_t status;
    uint32_t length;

    SD.cacheClear();
    SD.remove(fname);
    File file = SD.open(fname, O_WRONLY | O_CREAT | O_TRUNC);
    if (!file) return false;

    status = sim800.HTTP_get("http://api.ubirch.com:23456/download", length, file);
    file.close();

    return status == 200;
}


//extern unsigned int __heap_start;
//extern void *__brkval;
//
//static void freeMem() {
//    PRINT("Free memory = ");
//    DEBUGLN(SP - (__brkval ? (uint16_t) __brkval : (uint16_t) &__heap_start));
//}
