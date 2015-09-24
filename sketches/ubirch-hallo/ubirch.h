#ifndef _UBIRCH_H_
#define _UBIRCH_H_


#define UBIRCH_NO1_PIN_LED      13
#define UBIRCH_NO1_PIN_WATCHDOG 6

#define enable_led()    digitalWrite(UBIRCH_NO1_PIN_LED, HIGH)
#define disable_led()   digitalWrite(UBIRCH_NO1_PIN_LED, LOW)

#define enable_watchdog()   pinMode(UBIRCH_NO1_PIN_WATCHDOG, INPUT)
#define disable_watchdog()  pinMode(UBIRCH_NO1_PIN_WATCHDOG, OUTPUT)

// ERROR CODES
#define HALLO_ERROR_AUDIO   1
#define HALLO_ERROR_SDCARD  2

void error(uint8_t error);

#endif // _UBIRCH_H_