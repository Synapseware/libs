; ================================================================================================
; Constants
; ================================================================================================
.EQU		BRT_STEPS		= 8
.EQU		PWM_PORT		= PORTD
.EQU		PWM_DDR			= DDRD


; ================================================================================================
;	D A T A  S E G M E N T
; ================================================================================================
.DSEG
	DIRECTION:		.db		0
	PWM_VAL:		.db		0
	ACTIVE_LED:		.db		0
	OFFSET:			.db		0


.CSEG
; ================================================================================================
; PWM_Channel_Driver
; Runs the PWM channels
; - First portion always turns off all LEDs and turns on the active LED
; - If we've completed a full LED refresh, then the PWM value data is updated.
; - This function should be executed on 4us intervals, or at a rate that allows
;	the entire output bank to cycle at round 60Hz to avoid visual flickr
;	For example, with 16 LEDs (PWM channels) @ 60Hz:
;		16 channels * 256/8 brightness levels * 60Hz = 30,720 calls/second
;		42 channels * 256/8 brightness levels * 60Hz = 80,640 calls/second
; Uses:
;	X, Z, rmp, rtmp
; To invoke:
;	Registers:
;		rACTIVE_LED
;		rPWM_VAL
;	Constants:
;		TOTAL_CHANNELS	(8bit)
;	Data:
;		on_values			(8bit value table, SRAM, element count = TOTAL_CHANNELS)
;		LED_PORT_VALUES		(8bit value table, PGRM, element count = TOTAL_CHANNELS)
;		LED_DDRX_VALUES		(8bit value table, PGRM, element count = TOTAL_CHANNELS)
; ================================================================================================
PwmChnDrvr_Main:
	push	rmp
	push	rtmp
	push	XH
	push	XL
	push	ZH
	push	ZL

	; turn off all the LEDs
	clr		rmp
	out		PWM_DDR, rmp
	out		PWM_PORT, rmp

	; Check the active LED channel against the total channels and reset
	; if we've passed out total channel count on the previous run
	cpi		rACTIVE_LED, TOTAL_CHANNELS
	brlo	_pcd_activate_channel
	clr		rACTIVE_LED

	; increment the brightness comparison register
	ldi		rmp, BRT_STEPS
	add		rPWM_VAL, rmp

_pcd_activate_channel:
	push	rACTIVE_LED							; save the current LED value
	add		rACTIVE_LED, rOFFSET				; add the offset
	andi	rACTIVE_LED, TOTAL_CHANNELS - 1		; TOTAL_CHANNELS - 1
	ldi		XH, HIGH(on_values)
	ldi		XL, LOW(on_values)
	clr		rtmp
	add		XL, rACTIVE_LED						; add current LED to table index
	adc		XH, rtmp
	pop		rACTIVE_LED							; restore the current LED value
	ld		rtmp, X								; load current on_value
	cp		rtmp, rPWM_VAL						; compare with brightness value
	breq	_pcd_activate_next
	brlo	_pcd_activate_next
	ldi		ZH, HIGH(2*LED_PORT_VALUES)
	ldi		ZL, LOW(2*LED_PORT_VALUES)			; load PORT pointer
	clr		rtmp
	add		ZL, rACTIVE_LED						; add active LED offset
	adc		ZH, rtmp
	lpm		rtmp, Z								; load port value
	in		rmp, PWM_PORT
	or		rtmp, rmp
	out		PWM_PORT, rtmp
	ldi		ZH, HIGH(2*LED_DDRX_VALUES)
	ldi		ZL, LOW(2*LED_DDRX_VALUES)			; load DDRX pointer
	clr		rtmp
	add		ZL, rACTIVE_LED						; add active LED offset
	adc		ZH, rtmp
	lpm		rtmp, Z								; load ddr value
	in		rmp, PWM_DDR
	or		rtmp, rmp
	out		PWM_DDR, rtmp

_pcd_activate_next:
	inc		rACTIVE_LED							; move to the next active LED.

_pcd_next_return:
	pop		ZL
	pop		ZH
	pop		XL
	pop		XH
	pop		rtmp
	pop		rmp

	ret


; ================================================================================================
; Sets the direction of rotation
; 0		= Clockwise
; !0	= Counter-clockwise
; Pass new direction value in r16
; ================================================================================================
PwmChnDrvr_SetDirection:
	sts		DIRECTION, rmp
	ret


; ================================================================================================
; Updates the offset (which causes the rotation effect)
; ================================================================================================
PwmChnDrvr_UpdateOffset:
	lds		rmp, DIRECTION
	tst		rmp
	brne	_offset_left
_offset_right:
	inc		rOFFSET
	andi	rOFFSET, (TOTAL_CHANNELS-1)
	rjmp	_offset_done
_offset_left:
	dec		rOFFSET
	andi	rOFFSET, (TOTAL_CHANNELS-1)
_offset_done:
	ret


; ================================================================================================
; Sets up default values
; ================================================================================================
PwmChnDrvr_Init:
	ldi		rmp, 0
	sts		DIRECTION, rmp
	sts		PWM_VAL, rmp
	sts		ACTIVE_LED, rmp
	sts		OFFSET, rmp
	ret


; ================================================================================================
;	B E G I N  A L L  T A B L E  S T O R A G E
; ================================================================================================
;	Indexes                     1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16
LED_PORT_VALUES:		.db		0x01,  0x02,  0x02,  0x04,  0x04,  0x08,  0x08,  0x10,  0x01,  0x04,  0x04,  0x10,  0x01,  0x08,  0x02,  0x10
LED_DDRX_VALUES:		.db		0x03,  0x03,  0x06,  0x06,  0x0C,  0x0C,  0x18,  0x18,  0x05,  0x05,  0x14,  0x14,  0x09,  0x09,  0x12,  0x12


