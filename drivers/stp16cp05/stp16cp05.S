/*
 * stp16cp05.asm
 *
 *  Created: 6/20/2012 8:52:38 AM
 *   Author: Matthew
 *
 *	 stp16cp05 driver file
 *   contains functions which allow the system to write data to the LED driver chips
 */ 


.section .text
; ================================================================================================
;   REGISTER  DEFINITIONS
; ================================================================================================
#define work                    r16
#define temp                    r17
#define rREAD_ADDR              r9
#define rPWM_VAL                r18
#define rACTIVE_LED             r19
#define rOFFSET                 r20
#define rSTP_LOOP               r21
#define rSTP_BIT                r22
#define rSTP_DATA               r23


; ================================================================================================
;	CONSTANTS
; ================================================================================================
STP16_TOTAL_CHANNELS		= 32
STP16_BITS_PER_CHIP			= 16
STP16_TOTAL_CHIPS			= STP16_TOTAL_CHANNELS / STP16_BITS_PER_CHIP
STP16_BRT_STEPS				= 8					; # of graduations between 0 and 255 we care about
STP16_DATA_START			= 0x00				; Starting EEPROM address for brightness data
STP16_DDR					= DDRD
STP16_PORT					= PORTD
STP16_PINS					= PIND
STP16_OE					= PORTD6			; When low, output circuits respond to data
STP16_LE					= PORTD3			; When low, latch circuits hold previous data
STP16_CLK					= PORTD4			; Chips read serial data on 1->0 clock transition
STP16_DATA					= PORTD5			; Write data while clock line is low only


; ================================================================================================
;	DATA SEGMENT
; ===============================================================================================
.section .data
	DIRECTION:			.byte			0
	PWM_VAL:			.byte			0
	ACTIVE_LED:			.byte			0
	OFFSET:				.byte			0


; ================================================================================================
; MOCK-SERIAL DATA
; ================================================================================================
.section .text


; ================================================================================================
; Prepare the LED driver chain
; ================================================================================================
STP16_Init:
	push	work
	push	temp

	; setup DDR port
	in		work, _SFR_IO_ADDR(STP16_DDR)
	ori		work, (1<<STP16_OE) | (1<<STP16_LE) | (1<<STP16_CLK) | (1<<STP16_DATA)
	out		STP16_DDR, work

	; setup default I/O line values
	in		work, _SFR_IO_ADDR(STP16_PORT)
	ori		work, (1<<STP16_OE) | (1<<STP16_LE)
	andi	work, ~((1<<STP16_CLK) | (1<<STP16_DATA))
	out		_SFR_IO_ADDR(STP16_PORT), work

	; setup intial data values
	ldi		work, 0
	sts		DIRECTION, work
	sts		PWM_VAL, work
	sts		ACTIVE_LED, work
	sts		OFFSET, work

	clr		rACTIVE_LED
	clr		rSTP_LOOP
	clr		rSTP_BIT
	clr		rSTP_DATA

	pop		temp
	pop		work
	ret


; ================================================================================================
; STP16_Refresh
; Runs the PWM channels for the STP16CP05 chips.
;
; - First portion always turns off all LEDs and turns on the active LED
; - If we've completed a full LED refresh, then the PWM value data is updated.
; - This function should be executed on 4us intervals, or at a rate that allows
;	the entire output bank to cycle at round 60Hz to avoid visual flickr
;	For example, with 16 LEDs (PWM channels) @ 60Hz:
;		16 channels * 256/8 brightness levels * 60Hz = 30,720 calls/second
;		42 channels * 256/8 brightness levels * 60Hz = 80,640 calls/second
;		32 channels * 2 chips * 16 bits pers per chip * 256/8 brightness levels * 60Hz = 
; ================================================================================================
STP16_Refresh:
	push	work
	push	temp
	push	XH
	push	XL
	push	ZH
	push	ZL

	; setup initializes the chip count and toggles the OE and LE control lines for the chips
	; this loop should be called as frequently as possible!
	ldi		work, STP16_TOTAL_CHIPS
	mov		r10, work						; r10 tracks the number of chips
	mov		rACTIVE_LED, rOFFSET
	sbi		_SFR_IO_ADDR(STP16_PORT), STP16_OE			; disable output
	cbi		_SFR_IO_ADDR(STP16_PORT), STP16_LE			; disable data latch

	add		rPWM_VAL, STP16_BRT_STEPS		; increase brightness steps each loop

	; this loop enumerates all of the chips (currently 2)
	; setup includes initializing the bit count and LED set flag for each chip
	; this loop is basically a bit-pump for the 2 STP16CP05 chips
	; there will be 2 * 16 enumerations (2 chips, 16 bits each)
	_stp_chip_loop:
		ldi		work, 0xFF
		mov		r12, work					; r12 tracks if an LED was used on the chip for each loop
		ldi		work, STP16_BITS_PER_CHIP
		mov		r11, work					; r11 tracks the bits per chip

		; this loop walks all the bits that the current chip supports
		; it starts by setting up the brightness buffer data address and reading in the brightness data
		; for the current LED
		; next, it checks to see if the value is less than the current brightness limit (rPWM_VAL)
		; if it exceeds the limit, then the read value is zero'd out
		; next, it checks to see if it has already displayed a value for this chip, if it has, then it zero's out the brightness data
		; finally, it checks if the value is non-zero, and sets the data line accordingly, then triggers the clock line
		_stp_bit_loop:
			out		_SFR_IO_ADDR(EEARL), rACTIVE_LED		; Set default EEPROM address in address register
			sbi		_SFR_IO_ADDR(EECR), _SFR_IO_ADDR(EERE)	; load the brightness data for the active LED from EEPROM
			in		rSTP_DATA, _SFR_IO_ADDR(EEDR)

			inc		rACTIVE_LED
			andi	rACTIVE_LED, STP16_TOTAL_CHANNELS - 1

			cbi		_SFR_IO_ADDR(STP16_PORT), STP16_CLK	; bring clock line low (move this instruction around to give different timing values for HIGH-LOW transition)

			cp		rPWM_VAL, rSTP_DATA
			brlo	_stp_bit_loop_2
			clr		rSTP_DATA

		_stp_bit_loop_2:
			and		rSTP_DATA, r12			; and the brightness data with r12 (r12 is either 0xFF or 0x00)
			tst		rSTP_DATA
			brne	_stp_bit_loop_H
			rjmp	_stp_bit_loop_L

		_stp_bit_loop_H:
			sbi		_SFR_IO_ADDR(STP16_PORT), STP16_DATA
			;clr		r12						; clear r12 to signal an LED enabled event
			rjmp	_stp_bit_loop_3

		_stp_bit_loop_L:
			cbi		_SFR_IO_ADDR(STP16_PORT), STP16_DATA
			rjmp	_stp_bit_loop_3

		_stp_bit_loop_3:
			sbi		_SFR_IO_ADDR(STP16_PORT), STP16_CLK	; bring clock line high
			dec		r11						; complete bit loop
			brne	_stp_bit_loop

		dec		r10							; complete chip loop
		brne	_stp_chip_loop

	sbi		_SFR_IO_ADDR(STP16_PORT), STP16_LE			; enable data latch
	cbi		_SFR_IO_ADDR(STP16_PORT), STP16_OE			; enable output
	cbi		_SFR_IO_ADDR(STP16_PORT), STP16_CLK			; bring clock line low
	cbi		_SFR_IO_ADDR(STP16_PORT), STP16_LE			; disable data latch

	pop		ZL
	pop		ZH
	pop		XL
	pop		XH
	pop		temp
	pop		work

	ret


; ================================================================================================
; Sets the direction of rotation
; 0		= Clockwise
; !0	= Counter-clockwise
; Pass new direction value in r16
; ================================================================================================
STP16_SetDirection:
	sts		DIRECTION, work
	ret


; ================================================================================================
; Updates the offset (which causes the rotation effect)
; ================================================================================================
STP16_UpdateOffset:
	lds		work, DIRECTION
	tst		work
	brne	_offset_left
_offset_right:
	inc		rOFFSET
	andi	rOFFSET, STP16_TOTAL_CHANNELS - 1
	rjmp	_offset_done
_offset_left:
	dec		rOFFSET
	andi	rOFFSET, STP16_TOTAL_CHANNELS - 1
_offset_done:
	ret


; ================================================================================================
; Returns the STP16 driver channel count (total # of bits available) in the work register
; ================================================================================================
STP16_GetChannelCount:
	ldi		work, STP16_TOTAL_CHANNELS
	ret
