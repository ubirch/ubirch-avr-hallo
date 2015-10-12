#ifndef _UBIRCH_MAIN_H_
#define _UBIRCH_MAIN_H_

// this interval is an approximation (like 3-5minutes)
#define MESSAGE_REQUEST_INTERVAL    (30*60000)

// the colors used to indicate status
#define STATE_C_OFF     0,0,0
#define STATE_C_INIT    0,32,0
#define STATE_C_ERROR   128,0,0
#define STATE_C_OK      0,255,0
// busy indicates the system is busy and cannot accept user input
#define STATE_C_BUSY    0,0,5
#define STATE_C_MESSAGE  0,128,0


// config.h contains sensitive information, like the
// cell phone APN, username and password
#include "config.h"

#define RECORD_BUFFER_SIZE  512

// == pins used for the Audio Trinket ==

// VS1053 Breakout settings
// VCC/3V3
// GND
// MOSI
// MISO
// SCLK
#define DREQ            9      // VS1053 Data request, ideally an Interrupt pin
#define CARDCS          10     // Card chip select pin
#define BREAKOUT_RESET  -1     // VS1053 reset pin (output)
#define BREAKOUT_CS     A1     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    A0     // VS1053 Data/command select pin (output)

#define UBIRCH_NO1_PIN_LED      13
#define UBIRCH_NO1_PIN_WATCHDOG 6

// == special ubirch #1 stuff ==
#define enable_led()    digitalWrite(UBIRCH_NO1_PIN_LED, HIGH)
#define disable_led()   digitalWrite(UBIRCH_NO1_PIN_LED, LOW)

#define enable_watchdog()   pinMode(UBIRCH_NO1_PIN_WATCHDOG, INPUT)
#define disable_watchdog()  pinMode(UBIRCH_NO1_PIN_WATCHDOG, OUTPUT)

#undef NDEBUG
#ifndef NDEBUG
    MinimumSerial minimumSerial;

    #define PRINT(s) minimumSerial.print(F(s))
    #define PRINTLN(s) minimumSerial.println(F(s))
    #define DEBUG(s) minimumSerial.print(s)
    #define DEBUGLN(s) minimumSerial.println(s)
#else
    #define PRINT(s)
    #define PRINTLN(s)
    #define DEBUG(s)
    #define DEBUGLN(s)
#endif

SdFat SD;

// the server url for up and download (BASE)
const char SERVER_URL[] PROGMEM = "http://api.ubirch.com:23456/";

#endif // _UBIRCH_MAIN_H_