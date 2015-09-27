#ifndef _UBIRCH_MAIN_H_
#define _UBIRCH_MAIN_H_

// config.h contains sensitive information, like the
// cell phone APN, username and password
#include "config.h"

// == pins used for the Audio Trinket ==

// VS1053 Breakout settings
// VCC/3V3
// GND
// MOSI
// MISO
// SCLK
#define DREQ            9       // VS1053 Data request, ideally an Interrupt pin
#define CARDCS          10     // Card chip select pin
#define BREAKOUT_RESET  SDA      // VS1053 reset pin (output)
#define BREAKOUT_CS     A1     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    A0      // VS1053 Data/command select pin (output)

#define UBIRCH_NO1_PIN_LED      13
#define UBIRCH_NO1_PIN_WATCHDOG 6

// == special ubirch #1 stuff ==
#define enable_led()    digitalWrite(UBIRCH_NO1_PIN_LED, HIGH)
#define disable_led()   digitalWrite(UBIRCH_NO1_PIN_LED, LOW)

#define enable_watchdog()   pinMode(UBIRCH_NO1_PIN_WATCHDOG, INPUT)
#define disable_watchdog()  pinMode(UBIRCH_NO1_PIN_WATCHDOG, OUTPUT)

#define PRINT(s) minimumSerial.print(F(s))
#define PRINTLN(s) minimumSerial.println(F(s))
#define DEBUG(s) minimumSerial.print(s)
#define DEBUGLN(s) minimumSerial.println(s)

// generic classes we use
MinimumSerial minimumSerial;
SdFat SD;

// some functions used predeclared

extern unsigned int __heap_start;
extern void *__brkval;

// print free memory
static void freeMem();

// blink regularly when no error halt was issued
static void haltOK();

// send blink signals for error codes and never return
static void haltError(uint8_t code);

// do some blinking
static void blink(uint8_t n, unsigned long speed);

// create our test file
void createTestFile();

#endif // _UBIRCH_MAIN_H_