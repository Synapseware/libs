; ================================================================================================
; usitwi.asm
;
;  Created: 12/14/2011 5:50:21 PM
;   Author: matthew
; ================================================================================================


; ================================================================================================
;   REGISTER  DEFINITIONS
; ================================================================================================
.DEF	rUSI_DATA		= r21


; ================================================================================================
;	CONSTANTS
; ================================================================================================
.ifdef ATTINY2313
	.EQU	USI_DDR				= DDRB
	.EQU	USI_PORT			= PORTB
	.EQU	USI_PINS			= PINB
	.EQU	SCL					= PORTB7
	.EQU	SDA					= PORTB5
    .EQU	USI_START			= USISIF
	.EQU	USI_RX_BUFF_SZ		= 32
	.EQU	USI_TX_BUFF_SZ		= 0
.endif

.ifdef ATTINY25 | ATTINY45 | ATTINY85
    .EQU	USI_DDR				= DDRB
    .EQU	USI_PORT			= PORTB
    .EQU	USI_PINS			= PINB
    .EQU	SDA					= PORTB0
    .EQU	SCL					= PORTB2
    .EQU	USI_START			= USISIF
	.EQU	USI_RX_BUFF_SZ		= 32
	.EQU	USI_TX_BUFF_SZ		= 0
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
	USI_RESULT:		.byte			0
	USI_STATUS:		.byte			0
	USI_RX_HEAD:	.byte			0
	USI_TX_HEAD:	.byte			0
	USI_RX_TAIL:	.byte			0
	USI_TX_TAIL:	.byte			0
	USI_RX_AVAIL:	.byte			0
	USI_TX_AVAIL:	.byte			0
	USI_RX_DATA:	.byte			USI_RX_BUFF_SZ
	USI_TX_DATA:	.byte			USI_TX_BUFF_SZ


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
	sts		USI_STATUS, rmp

	; set SDA as input
	cbi		USI_DDR, SDA

;_usistart_1:
	; wait for SCL to go low to ensure start condition is complete
;	wdr
;	in		rmp, USI_PINS					; get TWI pin data
;	sbrc	rmp, SCL						; skip next if SCL is low
;	rjmp	_usistart_1


	; enable overflow interrupt and keep start condition
	ldi		rmp, (1<<USISIE) | (1<<USIOIE) | (1<<USIWM1) | (1<<USIWM0) | (1<<USICS1)
	out		USICR, rmp

	; set to sample 8 bits
	ldi		rmp, (1<<USI_START) | (1<<USIOIF) | (1<<USIPF) | (1<<USIDC)
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

	; get status field
	lds		rmp, USI_STATUS

	; address-match
	cpi		rmp, USI_SLAVE_CHECK_ADDRESS
	breq	_USI_SLAVE_CHECK_ADDRESS

	; request data
	cpi		rmp, USI_SLAVE_REQUEST_DATA
	breq	_USI_SLAVE_REQUEST_DATA

	cpi		rmp, USI_SLAVE_GET_DATA_AND_SEND_ACK
	breq	_USI_SLAVE_GET_DATA_AND_SEND_ACK

	; no status match
	rjmp	_SET_USI_TO_USI_START_CONDITION_MODE

_USI_SLAVE_CHECK_ADDRESS:
	in		rUSI_DATA, USIDR
	cpi		rUSI_DATA, TW_ADDRESS
	breq	_usi_addrcmp1					; successful address match
	;tst		rUSI_DATA
	;breq	_usi_addrcmp1					; TWI broadcast address (?)

_SET_USI_TO_USI_START_CONDITION_MODE:
	ldi		rmp, USI_SLAVE_CHECK_ADDRESS	; store valid-status
	sts		USI_STATUS, rmp
	ldi		rmp, (1<<USISIE) | (1<<USIWM1) | (1<<USICS1)
	out		USICR, rmp
	ldi		rmp, (1<<USI_START) | (1<<USIOIF) | (1<<USIPF) | (1<<USIDC)
	out		USISR, rmp
	rjmp	_usi_done
_usi_addrcmp1:
	ldi		rmp, USI_SLAVE_REQUEST_DATA		; store valid-status
	sts		USI_STATUS, rmp
	rcall	USI_SendACK
	rjmp	_usi_done

_USI_SLAVE_REQUEST_DATA:
	ldi		rmp, USI_SLAVE_GET_DATA_AND_SEND_ACK
	sts		USI_STATUS, rmp
	rcall	USI_SetToReceive
	rjmp	_usi_done

_USI_SLAVE_GET_DATA_AND_SEND_ACK:
	in		rUSI_DATA, USIDR
	ldi		rmp, USI_SLAVE_REQUEST_DATA
	sts		USI_STATUS, rmp
	rcall	USI_SendACK

	; prepare buffer
	lds		rmp, USI_RX_HEAD
	push	ZH
	push	ZL
	rcall	USI_PrepareZForReceive

	; store the TWI data
	st		Z, rUSI_DATA
	pop		ZL
	pop		ZH

	; update RX head address
	lds		rmp, USI_RX_HEAD
	inc		rmp
	andi	rmp, (USI_RX_BUFF_SZ - 1)
	sts		USI_RX_HEAD, rmp

	; update bytes available
	lds		rmp, USI_RX_AVAIL
	inc		rmp
	andi	rmp, (USI_RX_BUFF_SZ - 1)
	sts		USI_RX_AVAIL, rmp

	rjmp	_usi_done

_usi_done:
	pop		rmp
	out		SREG, rmp
	pop		rmp
	reti


; ================================================================================================
; TWI ACK
; ================================================================================================
USI_SendACK:
	ldi		rmp, 0
	out		USIDR, rmp						; clear USIDR
	sbi		USI_DDR, SDA					; set SDA as output
	ldi		rmp, (1<<USIOIF) | (1<<USIPF) | (1<<USIDC) | (0x0E<<USICNT0); Clear all flags, except Start Cond
	out		USISR, rmp
	ret


; ================================================================================================
; TWI read ACK
; ================================================================================================
USI_ReadACK:
	cbi		USI_DDR, SDA					; set SDA as input
	ldi		rmp, 0
	out		USIDR, rmp						; clear USIDR
	ldi		rmp, (1<<USIOIF) | (1<<USIPF) | (1<<USIDC) | (0x0E<<USICNT0)
	out		USISR, rmp
	ret


; ================================================================================================
; TWI Receive
; ================================================================================================
USI_SetToReceive:
	cbi		USI_DDR, SDA					; set SDA as input
	ldi		rmp, (1<<USIOIF) | (1<<USIPF) | (1<<USIDC)
	out		USISR, rmp
	ret


; ================================================================================================
; USI_ReadNext - reads the next byte from the buffer (and blocks)
; Data is returned in address USI_RESULT
; ================================================================================================
USI_ReadNext:
	push	rmp
_readWait:
	; wait for data
	wdr
	lds		rmp, USI_RX_AVAIL
	tst		rmp
	breq	_readWait

	; subtract 1 from the available count and update
	cli
	lds		rmp, USI_RX_AVAIL
	dec		rmp
	andi	rmp, (USI_RX_BUFF_SZ - 1)
	sts		USI_RX_AVAIL, rmp

	; load data using tail offset
	push	ZH
	push	ZL
	lds		rmp, USI_RX_TAIL
	rcall	USI_PrepareZForReceive

	; load return data
	ld		rmp, Z
	sts		USI_RESULT, rmp
	pop		ZL
	pop		ZH

	; increment tail count and update
	lds		rmp, USI_RX_TAIL
	inc		rmp
	andi	rmp, (USI_RX_BUFF_SZ-1)
	sts		USI_RX_TAIL, rmp

	pop		rmp
	sei
	ret


; ================================================================================================
; Loads the Z register with the receive buffer plus an offset stored in rmp
; ================================================================================================
USI_PrepareZForReceive:
	ldi		ZH, HIGH(USI_RX_DATA)
	ldi		ZL, LOW(USI_RX_DATA)
	add		ZL, rmp
	ldi		rmp, 0
	adc		ZH, rmp
	ret

; ================================================================================================
; Initialize USI for TWI mode
; ================================================================================================
USI_Init:
	; set start status value
	clr		rmp
	sts		USI_RESULT, rmp
	sts		USI_STATUS, rmp
	sts		USI_RX_HEAD, rmp
	sts		USI_TX_HEAD, rmp
	sts		USI_RX_TAIL, rmp
	sts		USI_TX_TAIL, rmp
	sts		USI_RX_AVAIL, rmp
	sts		USI_TX_AVAIL, rmp

	; setup SCL & SDA
	sbi		USI_PORT, SCL
	sbi		USI_PORT, SDA
	sbi		USI_DDR, SCL
	cbi		USI_DDR, SDA

	; setup USI in TWI Slave mode, w/interrupts
	; I2C/TWI address will be 0110,101,0 (write-only)
	; External, positive edge (clock source)
	ldi		rmp, (1<<USISIE) | (1<<USIWM1) | (1<<USICS1)					
	out		USICR, rmp

	; clear all flags and reset
	ldi		rmp, 0xF0
	out		USISR, rmp

	ret
