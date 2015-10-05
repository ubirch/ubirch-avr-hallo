
#ifndef WS2812_H__6d5a167d9974483bbda3d548072a3557
#define WS2812_H__6d5a167d9974483bbda3d548072a3557	1

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#ifndef WS2812_DO_DDR
#	define WS2812_DO_DDR		DDRD
#endif

#ifndef WS2812_DO_PORT
#	define WS2812_DO_PORT		PORTD
#endif

#ifndef WS2812_DO_BIT
#	define WS2812_DO_BIT		PD0
#endif

#define WS2812_HAVECOMPILER
#ifdef WS2812_HAVECOMPILER
#	define WS2812_WITHCOMPILER	1
#else
#	define WS2812_WITHCOMPILER	0
#endif


// for debugging purpose or if sequences become to large:
// #define WS2812_SINGLESEQUENCES	1




#ifdef WS2812_C__6d5a167d9974483bbda3d548072a3557
#	define WS2812PUBLIC	
#else
#	define WS2812PUBLIC	extern
#endif

#ifdef WS2812_SINGLESEQUENCES
#	define WS2812_SEQREG	"r31"
#else
#	define WS2812_SEQREG	"r30"
#endif


typedef struct __WS2812_LEDColor WS2812_LEDColor_t;

struct __WS2812_LEDColor {
  uint8_t	blue;
  uint8_t	red;
  uint8_t	green;
} __attribute__((packed)); 

typedef void (*WS2812_DOSEQ_func_t)(void);
WS2812PUBLIC const PROGMEM WS2812_DOSEQ_func_t WS2812_sequencefunction_address[8];

/*
 * repeatingly transmits a sequence of a total of "number_of_transmits" sequence-bytes (from "WS2812_get_sequencebyte"), when
 * the "sequence_precompiled"-array is (only) "sequence_size" elements large. (It rings the buffer.)
 * 
 * WARNING: Will deactivate interrupts for the time beeing active. WATCHDOG need to be off !
 */
WS2812PUBLIC void WS2812_transmit_precompiled_sequence(uint8_t *sequence_precompiled, size_t sequence_size, uint32_t number_of_transmits_24bit);

/*
 * translates/compiles 3 bits for output into a sequence-byte for the transmit engine
 */
WS2812PUBLIC uint8_t WS2812_get_sequencebyte(const uint8_t sequence_threebit);


#if WS2812_WITHCOMPILER
/*
 * Compiles a color into an 8 byte sequence starting at "output_buffer" and returns pointer to buffer at the end of these 8 bytes
 * WARNING: "output_buffer" needs at least 8 bytes in size
 */
WS2812PUBLIC uint8_t* WS2812_compilecolor(uint8_t *output_buffer, WS2812_LEDColor_t *color);
WS2812PUBLIC uint8_t* WS2812_compileRGB(uint8_t *output_buffer, const uint8_t red,  const uint8_t green, const uint8_t blue);

/*
 * Compiles an array of colors into a sequence for transmit, starting with array position 0
 * WARNING: output_buffer needs 8 Bytes per color
 */
WS2812PUBLIC void WS2812_compile(uint8_t *output_buffer, WS2812_LEDColor_t *color, size_t colors);
#endif

#endif
