; ================================================================================================
; eeprom.asm
;
;  Created: 12/16/2011 11:55:47 AM
;   Author: matthew
; ================================================================================================


.DEF	rEE_Addr		= r0
.DEF	rEE_Data		= r1


; ================================================================================================
;	CONSTANTS
; ================================================================================================
.ifdef ATTINY2313
	.EQU		PAGE_MASK		= 0x7F
.else
	.EQU		PAGE_MASK		= 0xFF
.endif


; ================================================================================================
;	DATA SEGMENT
; ===============================================================================================
.DSEG
	EE_ADDR:		.byte			0
	EE_DATA:		.byte			0


; ================================================================================================
; EEPROM handling code
; ================================================================================================
.CSEG


; ================================================================================================
; Read brightness data from EEPROM
; ================================================================================================
EEPROM_read:
	; Wait for completion of previous write
	sbic	EECR, EEPE
	rjmp	EEPROM_read

	lds		rEE_Addr, EE_ADDR
	out		EEARL, rEE_Addr				; Set up address in address register
	sbi		EECR, EERE					; Start eeprom read by writing EERE
	in		rEE_Addr, EEDR				; Read data from data register
	sts		EE_DATA, rEE_Addr
	ret


; ================================================================================================
; EEPROM Write
; r17 = Address
; r16 = Data
; ================================================================================================
EEPROM_write:
	; Wait for completion of previous write
	sbic	EECR, EEPE
	rjmp	EEPROM_write

	lds		rEE_Addr, EE_ADDR
	lds		rEE_Data, EE_DATA

	out		EEAR, rEE_Addr				; Set up address in address register
	out		EEDR, rEE_Data				; Write data to data register
	sbi		EECR, EEMPE					; Write logical one to EEMPE
	sbi		EECR, EEPE					; Start eeprom write by setting EEPE
	ret


; ================================================================================================
; Writes a block of data to the EEPROM
;  X		= Address to read from
;  r16		= Number of bytes to write
;  EE_ADDR	= EEPROM starting address
; ================================================================================================
EEPROM_WriteBlock:
	ld		rEE_Data, X+				; load data to be written
	sts		EE_DATA, rEE_Data			; store data byte
	rcall	EEPROM_write				; write the data
	lds		rEE_Addr, EE_ADDR
	inc		rEE_Addr
	sts		EE_ADDR, rEE_Addr
	dec		r16
	brne	EEPROM_WriteBlock

	ret


; ================================================================================================
; Initializes the EEPROM driver
; ================================================================================================
EEPROM_Init:
	clr		rmp
	sts		EE_ADDR, rmp
	sts		EE_DATA, rmp
	ret
