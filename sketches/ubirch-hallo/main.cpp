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

struct state_t {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t numled : 3; // 0-7
    uint8_t pulse : 1;
    uint8_t message : 1;
};

volatile state_t state;

inline void disable_pulse() {
    TIMSK1 &= ~(1 << OCIE1A);  // enable timer compare interrupt
}

inline void enable_pulse() {
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
}

void show_color(uint8_t r, uint8_t g, uint8_t b, uint8_t n = 7) {
    UCSR0B &= ~_BV(RXEN0);
    uint8_t buffer[8];
    WS2812_compileRGB(buffer, r, g, b);
    WS2812_DO_DDR |= _BV(WS2812_DO_BIT);
    WS2812_transmit_precompiled_sequence(buffer, sizeof(buffer), 8 * n);
    _delay_us(58);
    UCSR0B |= _BV(RXEN0);
}

void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t n = 7) {
    state.red = r;
    state.green = g;
    state.blue = b;
    state.numled = n;
    show_color(r, g, b, n);
}

ISR(TIMER1_COMPA_vect) {
    if (state.pulse) {
        float sin_start = 4.712;

        sin_start = sin_start + 0.1;
        if (sin_start > 10.995) sin_start = 4.712;

        float factor = sin(sin_start);
        uint8_t rh = state.red / 2;
        uint8_t gh = state.green / 2;
        uint8_t bh = state.blue / 2;

        show_color((const uint8_t) (factor * rh + rh),
                   (const uint8_t) (factor * gh + gh),
                   (const uint8_t) (factor * bh + bh), state.numled);
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

inline void halt(uint8_t c) {
    set_color(STATE_C_ERROR, state.numled);
    OCR1A = (F_CPU / 256 / 30);  // compare match register 16MHz/256/xxHz
    state.pulse = 1;

    while (1) {
        DEBUG(c);
        _delay_ms(1000);
    };
}

#pragma clang diagnostic pop


void setup() {
#ifndef NDEBUG
    // configure the initial values for UART and the connection to SIM800
    minimumSerial.begin((uint32_t) 115200);
#endif

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

    state.pulse = 1;
    set_color(STATE_C_OFF);

    // == SETUP ===============================================
    set_color(STATE_C_INIT, 1);
    if (!vs1053.begin()) {
        PRINTLN("VS1053 not found");
        halt(1);
    }
    vs1053.setVolume(255, 255);
    PRINTLN("VS1053 initialized ");

    set_color(STATE_C_INIT, 2);
    while (!SD.begin(CARDCS)) {
        PRINTLN("SDCARD not found, please insert");
        state.pulse = 1;
        set_color(STATE_C_ERROR, 2);
        _delay_ms(10000);
    }
    state.pulse = 0;
    PRINTLN("SDCARD initialized");

    set_color(STATE_C_INIT, 3);
    i2c_init(I2C_SPEED_400KHZ);
    if (!mpr_reset()) {
        PRINTLN("MPR121 not found");
        halt(2);
    }
    state.pulse = 0;
    PRINTLN("MPR121 initialized");

    set_color(STATE_C_INIT, 4);
    PRINTLN("SIM800 wakeup");
    if (!sim800.wakeup()) {
        PRINTLN("SIM800 wakeup error");
        halt(3);
    }
    sim800.setAPN(F(SIM800_APN), F(SIM800_USER), F(SIM800_PASS));

    set_color(STATE_C_INIT, 5);
    PRINTLN("SIM800 waiting for network registration");
    while (!sim800.registerNetwork()) {
        sim800.shutdown();
        sim800.wakeup();
    }

    set_color(STATE_C_INIT, 6);
    PRINTLN("SIM800 enabling GPRS");
    if (!sim800.enableGPRS()) {
        PRINTLN("SIM800 can't enable GPRS");
        halt(4);
    }
    PRINTLN("SIM800 initialized");

    set_color(STATE_C_OK);
    _delay_ms(1000);

    set_color(STATE_C_OFF);
}

// play a file from the SD card
uint8_t play(const char *fname) {
    SD.cacheClear();

    vs1053.setVolume(10, 10);
    vs1053.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
    vs1053.startPlayingFile(fname);

    uint8_t unfinished = 0;
    while (vs1053.playingMusic) {
        if (!(mpr_status() & _BV(0))) {
            unfinished = 1;
            vs1053.stopPlaying();
        }
    }
    // TODO hack to disable the timer interrupt
    TIMSK0 &= ~_BV(OCIE0A);

    set_color(0, 0, 128);
    while (mpr_status() & _BV(0)) _delay_ms(20);
    set_color(0, 0, 0);

    vs1053.reset();
    vs1053.setVolume(255, 255);
    return unfinished;
}

// saves recorded bytes to the given file
uint16_t saveRecordedData(uint8_t *buffer, size_t buffer_size, File &file, bool flush = false) {
    uint16_t available = vs1053.recordedWordsWaiting();

    // read one word less if this is the last block to record
    if (flush) available -= 1;

    // if the waiting bytes are less than out buffer or we are not flushing, do nothing
    if (!available || (available * 2 < buffer_size && !flush)) return 0;

    uint16_t idx = 0;
    size_t tosave = available;
    while (--tosave) {
        if (idx >= buffer_size) {
            file.write(buffer, idx);
            file.flush();
            idx = 0;
        }
        uint16_t word = vs1053.recordedReadWord();
        buffer[idx++] = word >> 8;
        buffer[idx++] = word & 0xff;
    }
    if (idx != 0) file.write(buffer, idx);
    file.flush();

    DEBUG(available * 2);
    PRINTLN(" bytes saved");

    return available * 2;
}

// record message to SD card
void record(const char *fname) {
    // we need to set a certain volume for recording, also affects the speaker unfortunately
    vs1053.setVolume(100, 100);

    // this takes a while ....
    vs1053.prepareRecordOgg((char *) "v44k1q05.img");

    SD.cacheClear();
    SD.remove(fname);
    File file = SD.open(fname, O_CREAT | O_TRUNC | O_WRITE);
    uint8_t *buffer = (uint8_t *) malloc(RECORD_BUFFER_SIZE);

    vs1053.startRecordOgg(false);
    set_color(31, 0, 0);

    // loop while checking touch and saving the data
    uint32_t saved = 0;
    while (mpr_status() & _BV(0)) {
        saved += saveRecordedData(buffer, RECORD_BUFFER_SIZE, file);
    }
    vs1053.stopRecordOgg();

    // save the last part
    saved += saveRecordedData(buffer, RECORD_BUFFER_SIZE, file);

    // stop and finalize
    saved += saveRecordedData(buffer, RECORD_BUFFER_SIZE, file, true);

    // read the very last word and save the first half
    uint16_t last_word = vs1053.recordedReadWord();
    file.write(last_word >> 8);
    saved++;

    // check whether we need to write the lower byte as well
    vs1053.sciRead(VS1053_SCI_AICTRL3);
    if (!(vs1053.sciRead(VS1053_SCI_AICTRL3) & _BV(2))) {
        file.write(vs1053.recordedReadWord() & 0xFF);
        saved++;
    }
    file.flush();
    file.close();
    free(buffer);

    DEBUG(saved);
    PRINTLN(" bytes recorded");

    set_color(0, 0, 0);

    vs1053.reset();
    vs1053.setVolume(255, 255);
}

// send file to server (upload)
bool sendFile(const char *fname, uint8_t retries) {
    uint16_t status;
    uint32_t length;

    do {
        SD.cacheClear();
        File file = SD.open(fname, O_RDONLY);
        status = sim800.HTTP_post("http://api.ubirch.com:23456/u/1", length, file, file.fileSize());
        file.close();

        if(status == 200) break;
    } while (--retries);
    return false;
}

// receive file from server (download)
bool receiveFile(const char *fname) {
    uint16_t status;
    uint32_t length;

    SD.cacheClear();
    SD.remove(fname);

    File file = SD.open(fname, O_WRONLY | O_CREAT | O_TRUNC);
    if (!file) {
        PRINTLN("ERROR: can't delete file");
        return false;
    }
    status = sim800.HTTP_get("http://api.ubirch.com:23456/d/1", length, file);
    file.close();

    return status == 200;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"


void loop() {
    long timer = 10000;

    // check if the sensor 0 was touched
    if ((mpr_status() & _BV(0))) {
        if (state.message) {
            // if a message is available, play it
            set_color(STATE_C_OFF);
            state.pulse = 0;

            PRINTLN("playing downloaded message");
            state.message = play("m.ogg");

            // keep the "message available, if not finished
            if (state.message) {
                set_color(0, 255, 0);
                state.pulse = 1;
            }
        } else {
            // if no message is available, record a new message
            disable_pulse();
            set_color(STATE_C_OFF);

            PRINTLN("recording message");
            disable_pulse();
            record("r.ogg");
            enable_pulse();

            set_color(STATE_C_BUSY);
            PRINTLN("sending recorded message");
            sendFile("r.ogg", 3);
            set_color(STATE_C_OFF);
            state.message = 0;
            enable_pulse();
        }
    } else {
        // this counter is our interval for checking remotely for a new message
        if (--timer < 0) {
            timer = MESSAGE_REQUEST_INTERVAL;

            // if we don't have a message locally available, check remotely
            if (!state.message) {
                disable_pulse();
                set_color(STATE_C_BUSY);
                if (receiveFile("m.ogg")) {
                    PRINTLN("RECEIVED NEW MESSAGE");
                    set_color(0, 255, 0);
                    state.pulse = 1;
                    state.message = 1;
                } else {
                    PRINTLN("NO NEW MESSAGE (or error)");
                    state.pulse = 0;
                    set_color(STATE_C_OFF);
                    state.message = 0;
                }
                enable_pulse();
            }
        }
        if (timer % 1000 == 0) {
            DEBUG(state.message ? "!" : ".");
        }
    }
}

#pragma clang diagnostic pop