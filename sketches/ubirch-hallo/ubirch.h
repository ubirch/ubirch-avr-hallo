#ifndef _UBIRCH_H_
#define _UBIRCH_H_

// SIM800H settings
#define UBIRCH_NO1_SIM800_RX    2
#define UBIRCH_NO1_SIM800_TX    3
#define UBIRCH_NO1_SIM800_RST   4
#define UBIRCH_NO1_SIM800_KEY   7
#define UBIRCH_NO1_SIM800_PS    8

#define UBIRCH_NO1_PIN_LED      13
#define UBIRCH_NO1_PIN_WATCHDOG 6

#define enable_led()    digitalWrite(UBIRCH_NO1_PIN_LED, HIGH)
#define disable_led()   digitalWrite(UBIRCH_NO1_PIN_LED, LOW)

#define enable_watchdog()   pinMode(UBIRCH_NO1_PIN_WATCHDOG, INPUT)
#define disable_watchdog()  pinMode(UBIRCH_NO1_PIN_WATCHDOG, OUTPUT)

#endif // _UBIRCH_H_