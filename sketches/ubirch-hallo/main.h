#ifndef _UBIRCH_MAIN_H_
#define _UBIRCH_MAIN_H_

// config.h contains sensitive information, like the
// cell phone APN, username and password
#include "config.h"

// == pins used for the Audio Trinket ==

// VS1053 Breakout settings
#define DREQ            9       // VS1053 Data request, ideally an Interrupt pin
#define CARDCS          10     // Card chip select pin
#define BREAKOUT_RESET  SDA      // VS1053 reset pin (output)
#define BREAKOUT_CS     A1     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    A0      // VS1053 Data/command select pin (output)

#endif // _UBIRCH_MAIN_H_