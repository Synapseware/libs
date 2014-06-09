/*
 * usitwi.asm
 *
 *  Created: 12/14/2011 5:50:21 PM
 *   Author: matthew
 */ 


; ================================================================================================
;	CONSTANTS
; ================================================================================================
.ifdef ATTINY2313
	.EQU	TWI_DDR				= DDRB
	.EQU	TWI_PORT			= PORTB
	.EQU	TWI_PINS			= PINB
	.EQU	SCL					= PORTB7
	.EQU	SDA					= PORTB5
    .EQU	TWI_START			= USISIF
	.EQU	TWI_BUFF_SZ			= 16
.endif

.ifdef ATTINY25 | ATTINY45 | ATTINY85
    .EQU	TWI_DDR				= DDRB
    .EQU	TWI_PORT			= PORTB
    .EQU	TWI_PINS			= PINB
    .EQU	SDA					= PORTB0
    .EQU	SCL					= PORTB2
    .EQU	TWI_START			= USISIF
	.EQU	TWI_BUFF_SZ			= 16
.endif

.EQU	USI_SLAVE_CHECK_ADDRESS						= 0x00
.EQU	USI_SLAVE_SEND_DATA							= 0x01
.EQU	USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA		= 0x02
.EQU	USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA		= 0x03
.EQU	USI_SLAVE_REQUEST_DATA						= 0x04
.EQU	USI_SLAVE_GET_DATA_AND_SEND_ACK				= 0x05


; ================================================================================================
;	DATA SEGMENT
; ===============================================================================================
.DSEG
	TWI_STATUS:		.db			0
	TWI_RX_HEAD:	.db			0
	TWI_TX_HEAD:	.db			0
	TWI_TX_DATA:	.BYTE		TWI_BUFF_SZ
	TWI_RX_DATA:	.BYTE		TWI_BUFF_SZ


; ================================================================================================
; USI TWI INTERRUPT HANDLERS
; ================================================================================================
.CSEG


; ================================================================================================
; USI Start Condition detected
; ================================================================================================
ISR_USIStart:
	push	rmp
	in		rmp, SREG
	push	rmp

	; we need to receive the address byte from our master, then prepare to read
	; new brightness data map
	
	; set TWI status register accordingly
	ldi		rmp, USI_SLAVE_CHECK_ADDRESS
	sts		TWI_STATUS, rmp

	; set SDA as input
	cbi		TWI_DDR, SDA

_usistart_1:
	; wait for SCL to go low to ensure start condition is complete
	in		rmp, TWI_PINS					; get TWI pin data
	sbrc	rmp, SCL						; skip next if SCL is low
	rjmp	_usistart_1

	; enable overflow interrupt and keep start condition
	ldi		rmp, (1<<USISIE) | (1<<USIOIE) | (1<<USIWM1) | (1<<USIWM0) | (1<<USICS1)
	out		USICR, rmp

	; set to sample 8 bits
	ldi		rmp, (1<<TWI_START) | (1<<USIOIF) | (1<<USIPF) | (1<<USIDC)
	out		USISR, rmp

	pop		rmp
	out		SREG, rmp
	pop		rmp
	reti


; ================================================================================================
; USI Overflow
; When enabled, this will act as our receive-data interrupt
; ================================================================================================
ISR_USIOverflow:
	push	rmp
	in		rmp, SREG
	push	rmp

	;
	; check status field
	;
	lds		rmp, TWI_STATUS

	; address-match
	cpi		rmp, USI_SLAVE_CHECK_ADDRESS
	breq	_USI_SLAVE_CHECK_ADDRESS

	; request data
	cpi		rmp, USI_SLAVE_REQUEST_DATA
	breq	_USI_SLAVE_REQUEST_DATA

	cpi		rmp, USI_SLAVE_GET_DATA_AND_SEND_ACK
	breq	_USI_SLAVE_GET_DATA_AND_SEND_ACK

	; no status match
	rjmp	_SET_USI_TO_TWI_START_CONDITION_MODE

_USI_SLAVE_CHECK_ADDRESS:
	cpi		rTWI_DATA, TW_ADDRESS
	breq	_usi_addrcmp1					; successful address match
	tst		rTWI_DATA
	breq	_usi_addrcmp1					; TWI broadcast address (?)
_SET_USI_TO_TWI_START_CONDITION_MODE:
	ldi		rmp, (1<<USISIE)|(1<<USIWM1)|(1<<USICS1)
	out		USICR, rmp
	ldi		rmp, (1<<USIOIF)|(1<<USIPF)|(1<<USIDC)
	out		USISR, rmp
	rjmp	_usi_done
_usi_addrcmp1:
	ldi		rmp, USI_SLAVE_REQUEST_DATA		; store valid-status
	sts		TWI_STATUS, rmp
	rcall	TWI_ACK
	rjmp	_usi_done

_USI_SLAVE_REQUEST_DATA:
	ldi		rmp, USI_SLAVE_GET_DATA_AND_SEND_ACK
	sts		TWI_STATUS, rmp
	rcall	TWI_SetToReceive
	rjmp	_usi_done

_USI_SLAVE_GET_DATA_AND_SEND_ACK:
	ldi		rmp, USI_SLAVE_REQUEST_DATA		; store valid-status
	sts		TWI_STATUS, rmp
	rcall	TWI_ACK
	push	ZH
	push	ZL

	; capture USI data
	in		rTWI_DATA, USIDR

	; prepare buffer
	ldi		ZH, HIGH(TWI_RX_DATA)
	ldi		ZL, LOW(TWI_RX_DATA)
	lds		rmp, TWI_RX_HEAD
	add		ZL, rmp
	ldi		rmp, 0
	adc		ZH, rmp

	; store the TWI data
	st		Z, rTWI_DATA

	; update RX head address
	lds		rmp, TWI_RX_HEAD
	inc		rmp
	andi	rmp, (TWI_BUFF_SZ - 1)
	sts		TWI_RX_HEAD, rmp

	pop		ZL
	pop		ZH

	rjmp	_usi_done

_usi_done:
	pop		rmp
	out		SREG, rmp
	pop		rmp
	reti


; ================================================================================================
; TWI Receive
; ================================================================================================
TWI_SetToReceive:
	cbi		TWI_DDR, SDA					; set SDA as input
	ldi		rmp, (1<<USIOIF)|(1<<USIPF)|(1<<USIDC)
	out		USISR, rmp
	ret


; ================================================================================================
; TWI ACK
; ================================================================================================
TWI_ACK:
	clr		rmp
	out		USIDR, rmp						; clear USIDR
	sbi		TWI_DDR, SDA					; set SDA as output
	ldi		rmp, (1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0E<<USICNT0); Clear all flags, except Start Cond
	out		USISR, rmp
	ret


; ================================================================================================
; TWI NACK
; ================================================================================================
TWI_NACK:
	ret


; ================================================================================================
; Initialize USI for TWI mode
; ================================================================================================
TWI_Init:
	push	rmp

	; setup SCL & SDA
	sbi		TWI_PORT, SCL
	sbi		TWI_PORT, SDA
	sbi		TWI_DDR, SCL
	cbi		TWI_DDR, SDA

	; setup USI in TWI, Slave mode, interrupts
	; i2c/twi address will be 0110,101,r/w
	; Start Condition Interrupt Enable
	; Two-wire mode. Uses SDA (DI) and SCL (USCK) pins
	; Hold SCL low...
	; External, positive edge (clock source)
	ldi		rmp, (1<<USISIE) | (1<<USIWM1) | (1<<USICS1)					
	out		USICR, rmp

	// clear all flags and reset
	ldi		rmp, 0xF0
	out		USISR, rmp

	; set start status value
	clr		rmp
	sts		TWI_STATUS, rmp

	pop		rmp
	ret
