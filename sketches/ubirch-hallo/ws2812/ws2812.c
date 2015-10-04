  /*
   * !!! Following desciption applies for 8 MHz only !!!
   */
  
  /*
   * always burst 3 NRZ bits per compiled byte
   * 	- frequency must be calibrated 8MHz
   * 	- clocktime will be 125ns
   * 	- port-toggle (sbi) needs 2 cycles - use out instead!

   * new timing WS2812B:
   * 	- LOW:	111  0000000	( H:0.375us L:0.875us )
   * 	- HIGH:	11111   0000	( H:0.75us  L:0.500us )
   * 
   * original (old) timing WS2812:
   * 	- LOW:	111   000000	( H:0.375us L:0.75us  )
   * 	- HIGH:	111111 00000	( H:0.75us  L:0.625us )
   * 
   * so each burst has at least 21 cycles for preparing next burst
   * 
   */
  
  /*
   * Z register (r31:30) 
   * 	- points to next sequence to execute
   *    - forr speed reasons r31 stays fix
   * 	- only r30 is updated with precompiled byte
   * 	  (this is the reason, why we need alignment!)
   */
  
  /*
   * Y register (r29:28) (movw backup in r3:r2)
   * 	- keeps track of next compiled byte to read
   * 	- needs X register to detect overflow
   */
  
  /* Backup for Y (original start of buffer) in r3:r2 */
  
 /* use r4:r5 to use "out" opcode instead "sbi" - save 1 cycle !
  * r4 : outputvalue of "WS2812_DO_PORT" when "WS2812_DO_BIT" is LOW
  * r5 : outputvalue of "WS2812_DO_PORT" when "WS2812_DO_BIT" is HIGH
  */
  
  /*
   * X register (r27:r26) 
   * 	- point to first byte NOT in Y-buffer
   * 
   * 	(2)		ld	r30, Y+
   * 	(1)		cp	r28, r26
   * 	(1)		cpc	r29, r27
   * 	(1)		brcs	goon_nooverflow
   * 	(1)		movw	r28, r2
   * 
   * 	goon_nooverflow:	(at this point always 6 cycles are burned)
   * 	
   */
  
  /* 
   * virtual 24bit register 23 : 25:24 is decrementing burstcounter (4 cycles)
   * 
   * 	(2)		sbiw	r24, 0x1
   * 	(1)		sbc	r23, __zero_reg__
   *    (1)		breq	end_of_this
   * 	...
   * 			ijmp	(jump to next burst)
   * 	end_of_this:	rjmp	exit_this_function
   * 	
   */

#define WS2812_C__6d5a167d9974483bbda3d548072a3557	1

#include "ws2812.h"

#include <stdlib.h>

#ifndef WS2812_SINGLESEQUENCES
void onlyASMSymbol_WS2812_sequences(void) __attribute__ ((naked,used,noinline,aligned (512)));

void onlyASMSymbol_WS2812_sequence_LLL(void);
void onlyASMSymbol_WS2812_sequence_LLH(void);
void onlyASMSymbol_WS2812_sequence_LHL(void);
void onlyASMSymbol_WS2812_sequence_LHH(void);
void onlyASMSymbol_WS2812_sequence_HLL(void);
void onlyASMSymbol_WS2812_sequence_HLH(void);
void onlyASMSymbol_WS2812_sequence_HHL(void);
void onlyASMSymbol_WS2812_sequence_HHH(void);

void onlyASMSymbol_WS2812_sequences(void) {
  asm volatile (
#endif


#if (F_CPU == 8000000ULL) 
#include "burstsequence_8mhz.h"
#elif (F_CPU == 16000000ULL) 
#include "burstsequence_16mhz.h"
#else
#error "no code for suiteable timing found!"
#endif
  
#ifdef WS2812_SINGLESEQUENCES
#else
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}
#endif  

const PROGMEM WS2812_DOSEQ_func_t WS2812_sequencefunction_address[8] = {
  onlyASMSymbol_WS2812_sequence_LLL,
  onlyASMSymbol_WS2812_sequence_LLH,
  onlyASMSymbol_WS2812_sequence_LHL,
  onlyASMSymbol_WS2812_sequence_LHH,

  onlyASMSymbol_WS2812_sequence_HLL,
  onlyASMSymbol_WS2812_sequence_HLH,
  onlyASMSymbol_WS2812_sequence_HHL,
  onlyASMSymbol_WS2812_sequence_HHH
};

void WS2812_transmit_precompiled_sequence(uint8_t *sequence_precompiled, size_t sequence_size, uint32_t number_of_transmits_24bit) {
  /* 
   * at this point we don't need to care much about timing
   * we just prepare all registers and then fire "onlyASMSymbol_WS2812_sequence*",
   * which will loop during the whole "number_of_transmits".
   */
  asm volatile (
    /* be safe: manually save x, y and z to stack */
    "	push	r31			\n\t"
    "	push	r30			\n\t"
    "	push	r29			\n\t"
    "	push	r28			\n\t"
    "	push	r27			\n\t"
    "	push	r26			\n\t"
    
    /* save status of interrupts */
    "	in	r0,	%[sreg]		\n\t"
    "	cli				\n\t"
    "	push	r0			\n\t"
    
    /* prepare Y, its backup and X  */
    "	movw	r2,	r28		\n\t"	/* backup Y into r3:r2 */
    "	add	r26,	r28		\n\t"	/* add low(X) with low(Y) */
    "	adc	r27,	r29		\n\t"	/* X = Y+%[seqsiz] */
    
    /* prepare predefined HIGH (r5) and LOW (r4) port states */
    "	ldi	r23,	%[dobit]	\n\t"
    "	in	r5,	%[doport]	\n\t"	/* retrieve port status */
    "	or	r5,	r23		\n\t"
    "	mov	r4,	r5		\n\t"
    "	eor	r4,	r23		\n\t"

    /* initialize  virtual 24bit decrementing burstcounter to "number_of_transmits_24bit" */
    "	mov	r23,	%C[notx]	\n\t"
    "	mov	r25,	%B[notx]	\n\t"
    "	mov	r24,	%A[notx]	\n\t"

    /* give burstsequence-code a clear defined __zero_reg__ = r1 */
    "	clr	r1			\n\t"
    

    /* init the Z register - now it becomes hacked: */
    /* use the alignment for access advantage */
#ifdef WS2812_SINGLESEQUENCES
    /* 
     * Every sequence is aligned!
     * So LOW-part of address is always ZERO.
     * Here the sequence-code is the HI-byte of the address
     */
    "	clr	r30			\n\t"	/* LO(Z) is always zero */
#else
    /* 
     * All sequences together are aligned!
     * So HI-part of the address stays always the same,
     * indepenently which sequence-code to process...
     */

    /* Z already initialized (by input) to LLL-sequencefunc */
    /* DO NOT CHANGE HIGH(Z) = r31 */
#endif
    "	ld   "WS2812_SEQREG", Y+	\n\t"	/* load first sequence */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    "	icall				\n\t"	/* do a call NOT a jmp */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    "	nop				\n\t"



    /* restore propably destroyed __zero_reg__ */
    "	clr	__zero_reg__		\n\t"

    /* restore previous state of interrupts */
    "	pop	r26			\n\t"
    "	andi	r26,	%[ibit]		\n\t"
    "	breq	WS2812_transmit_precompiled_sequence__noint%=	\n\t"
    "	sei				\n\t"

    /* restore remaining registers */
    "	WS2812_transmit_precompiled_sequence__noint%=:		\n\t"
    "	pop	r26			\n\t"
    "	pop	r27			\n\t"
    "	pop	r28			\n\t"
    "	pop	r29			\n\t"
    "	pop	r30			\n\t"
    "	pop	r31			\n\t"
    :
    : [sreg]	"I"	(_SFR_IO_ADDR(SREG)),
      [ibit]	"M"	(_BV(SREG_I)),
      [doport]	"I"	(_SFR_IO_ADDR(WS2812_DO_PORT)),
      [dobit]	"M"	(_BV(WS2812_DO_BIT)),
#ifndef WS2812_SINGLESEQUENCES
      [seqfunc]	"z"	(((uint16_t)&onlyASMSymbol_WS2812_sequence_LLL)),
#endif
      [seqcs]	"y"	(((uint16_t)sequence_precompiled)),
      [seqsize]	"x"	(((uint16_t)sequence_size)),
      [notx]	"r"	(number_of_transmits_24bit)
      
    : "r0", "r1", "r2", "r3", "r4", "r5",  /* "r31", "r30", "r29", "r28", "r27", "r26", */ "r25", "r24", "r23"
  );
}

uint8_t WS2812_get_sequencebyte(const uint8_t sequence_threebit) {
  uint8_t 	*funcaddr_flashptr	= (void*)&WS2812_sequencefunction_address[sequence_threebit & 0x7];
#ifdef WS2812_SINGLESEQUENCES
  return pgm_read_byte(&funcaddr_flashptr[1]);	/* return upper byte, since every sequence is aligned */
#else
  return pgm_read_byte(&funcaddr_flashptr[0]); 	/* return lower byte, since all sequences together are aligned */
#endif
}

#if WS2812_WITHCOMPILER
uint8_t* WS2812_compilecolor(uint8_t *output_buffer, WS2812_LEDColor_t *color) {
  uint8_t tmp;
  
  tmp = color->green; 							/* g7 g6 g5 g4 g3 g2 g1 g0 */
  output_buffer[0]=WS2812_get_sequencebyte(tmp >> 5); tmp<<=3;		/* g4 g3 g2 g1 g0 00 00 00 */
  output_buffer[1]=WS2812_get_sequencebyte(tmp >> 5); tmp<<=3;		/* g1 g0 00 00 00 00 00 00 */	
  
  tmp = (color->red >> 2) | tmp;					/* g1 g0 r7 r6 r5 r4 r3 r2 */
  output_buffer[2]=WS2812_get_sequencebyte(tmp >> 5);
  tmp = (color->red << 1);						/* r6 r5 r4 r3 r2 r1 r0 00 */
  output_buffer[3]=WS2812_get_sequencebyte(tmp >> 5); tmp<<=3;		/* r3 r2 r1 r0 00 00 00 00 */
  output_buffer[4]=WS2812_get_sequencebyte(tmp >> 5); tmp<<=3;		/* r0 00 00 00 00 00 00 00 */

  tmp = (color->blue >> 1) | tmp;					/* r0 b7 b6 b5 b4 b3 b2 b1 */
  output_buffer[5]=WS2812_get_sequencebyte(tmp >> 5); tmp<<=3;
  tmp = (color->blue << 2);						/* b5 b4 b3 b2 b1 b0 00 00 */
  output_buffer[6]=WS2812_get_sequencebyte(tmp >> 5); tmp<<=3;		/* b2 b1 b0 00 00 00 00 00 */
  output_buffer[7]=WS2812_get_sequencebyte(tmp >> 5);
  
  return &output_buffer[8];
}

uint8_t* WS2812_compileRGB(uint8_t *output_buffer, const uint8_t red,  const uint8_t green, const uint8_t blue) {
  WS2812_LEDColor_t color;
  
  color.red	= red;
  color.green	= green;
  color.blue	= blue;

  return WS2812_compilecolor(output_buffer, &color);
}

void WS2812_compile(uint8_t *output_buffer, WS2812_LEDColor_t *color, size_t colors) {
  while (colors) {
    output_buffer = WS2812_compilecolor(output_buffer, color);

    color = &color[1];
    colors--;
  }
}
#endif
