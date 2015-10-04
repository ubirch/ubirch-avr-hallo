#ifdef WS2812_SINGLESEQUENCES
void onlyASMSymbol_WS2812_sequence_LLL(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_LLL(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_LLL:				\n\t"
#endif
    
    /* FIRST: L--  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    /* 00000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_LLL__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    /* 00 */
    "  onlyASMSymbol_WS2812_sequence_LLL__noo%=:			\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"



    /* SECOND: -L-  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"



    /* THIRD: --L  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_LLL__end%=	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_LLL__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}

void onlyASMSymbol_WS2812_sequence_LLH(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_LLH(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_LLH:				\n\t"
#endif
    
    /* FIRST: L--  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    /* 00000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_LLH__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    /* 00 */
    "  onlyASMSymbol_WS2812_sequence_LLH__noo%=:			\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"



    /* SECOND: -L-  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"



    /* THIRD: --H  (10 clock cycles total) - 13 cycles when exitting ! */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_LLH__end%=	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_LLH__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}

void onlyASMSymbol_WS2812_sequence_LHL(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_LHL(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_LHL:				\n\t"
#endif
    
    /* FIRST: L--  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    /* 00000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_LHL__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    /* 00 */
    "  onlyASMSymbol_WS2812_sequence_LHL__noo%=:			\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"



    /* SECOND: -H-  (10 clock cycles total) */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"



    /* THIRD: --L  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_LHL__end%=	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_LHL__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}

void onlyASMSymbol_WS2812_sequence_LHH(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_LHH(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_LHH:				\n\t"
#endif
    
    /* FIRST: L--  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    /* 00000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_LHH__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    /* 00 */
    "  onlyASMSymbol_WS2812_sequence_LHH__noo%=:			\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"



    /* SECOND: -H-  (10 clock cycles total) */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"



    /* THIRD: --H  (10 clock cycles total) - 13 cycles when exitting ! */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_LHH__end%=	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_LHH__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}

void onlyASMSymbol_WS2812_sequence_HLL(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_HLL(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_HLL:				\n\t"
#endif
    
    /* FIRST: H--  (10 clock cycles total) */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		nop						\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_HLL__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    "  onlyASMSymbol_WS2812_sequence_HLL__noo%=:			\n\t"
    "	/* 1 */		nop						\n\t"



    /* SECOND: -L-  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"



    /* THIRD: --L  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_HLL__end%=	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_HLL__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}

void onlyASMSymbol_WS2812_sequence_HLH(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_HLH(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_HLH:				\n\t"
#endif
    
    /* FIRST: H--  (10 clock cycles total) */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		nop						\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_HLH__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    "  onlyASMSymbol_WS2812_sequence_HLH__noo%=:			\n\t"
    "	/* 1 */		nop						\n\t"



    /* SECOND: -L-  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"



    /* THIRD: --H  (10 clock cycles total) - 13 cycles when exitting ! */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_HLH__end%=	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_HLH__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}

void onlyASMSymbol_WS2812_sequence_HHL(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_HHL(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_HHL:				\n\t"
#endif
    
     /* FIRST: H--  (10 clock cycles total) */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		nop						\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_HHL__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    "  onlyASMSymbol_WS2812_sequence_HHL__noo%=:			\n\t"
    "	/* 1 */		nop						\n\t"



    /* SECOND: -H-  (10 clock cycles total)*/
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"



    /* THIRD: --L  (10 clock cycles total) */
    /* 111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_HHL__end%=	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_HHL__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}

void onlyASMSymbol_WS2812_sequence_HHH(void) __attribute__ ((naked,used,noinline,aligned (512)));
void onlyASMSymbol_WS2812_sequence_HHH(void) {
  asm volatile (
#else
    "  onlyASMSymbol_WS2812_sequence_HHH:				\n\t"
#endif
    
     /* FIRST: H--  (10 clock cycles total) */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		ld   "WS2812_SEQREG", Y+			\n\t"
    "	/* 1 */		cp   r28,	r26				\n\t"
    "	/* 1 */		cpc  r29,	r27				\n\t"
    "	/* 1 */		nop						\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		brcs onlyASMSymbol_WS2812_sequence_HHH__noo%=	\n\t"
    "	/* 1 */		movw r28,	r2				\n\t"
    "  onlyASMSymbol_WS2812_sequence_HHH__noo%=:			\n\t"
    "	/* 1 */		nop						\n\t"



    /* SECOND: -H-  (10 clock cycles total)*/
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		sbiw r24,	0x1				\n\t"
    "	/* 1 */		sbc  r23,	r1				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"



    /* THIRD: --H  (10 clock cycles total) - 13 cycles when exitting ! */
    /* 111111 */
    "	/* 1 */		out  %[doport],	r5				\n\t"
    "	/* 2 */		rjmp .+0		/* waste 2 clocks */	\n\t"
    "	/* 3 */		lpm  r0,	Z	/* waste 3 clocks */	\n\t"
    /* 0000 */
    "	/* 1 */		out  %[doport],	r4				\n\t"
    "	/* 1 */		breq onlyASMSymbol_WS2812_sequence_HHH__end%=	\n\t"
    "	/* 2 */		ijmp						\n\t"
    
    "  onlyASMSymbol_WS2812_sequence_HHH__end%=:			\n\t"
    "   /* 1 */		/* one additional clock breq true branch*/	\n\t"
    "	/* 4 */		ret						\n\t"

#ifdef WS2812_SINGLESEQUENCES
    :
    : [doport]       "I" (_SFR_IO_ADDR(WS2812_DO_PORT))
    :
  );
}
#endif
