#ifndef F_CPU
	#error please define F_CPU with the correct clock-frequency of your MCU
#endif

#include "ws2812.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#include <string.h>
#include <alloca.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <util/atomic.h>
#include <util/delay.h>


int main(void) {
    uint8_t testbuffer[8];	  
    
    WS2812_DO_DDR |= _BV(WS2812_DO_BIT);
    wdt_disable();
    cli();
    

    // should be white for 1 LED
    WS2812_compileRGB(testbuffer, 0xff, 0xff, 0xff);

    // cycle white to 12 LEDs
    WS2812_transmit_precompiled_sequence(testbuffer, sizeof(testbuffer), 8*12);
    _delay_us(58);
    
    while(1) {
      wdt_reset();
    }
}
