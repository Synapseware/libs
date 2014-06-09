; ************************************************************************************************
; EEPROM Utilities
;	2011 - Matthew Potter
; ************************************************************************************************


.DSEG
	EEPROM_DATA:	.BYTE	1		; EEPROM data to write, or EEPROM data that was read
	EEPROM_ADDR:	.BYTE	1		; EEPROM data Address
									; Bit 7 is read/write flag (0 = read, 1 = write)

.CSEG


; ================================================================================================
; ISR_EEPROM_Ready
;	Interrupt handler for EEPROM
;	
; ================================================================================================
ISR_EEPROM_Ready:
	
	reti

; ================================================================================================
; EEPROM_Ready_Write
;	
; ================================================================================================
EEPROM_Ready_Write:
	lds		r16, EEPROM_DATA
	lds		r17, EEPROM_ADDR
	rcall	EEPROM_Write
	reti


; ================================================================================================
; EEPROM_Ready_Read
;	Interrupt handler for EEPROM
; ================================================================================================
EEPROM_Ready_Read:
	lds		r17, EEPROM_ADDR
	rcall	EEPROM_Read
	reti

; ================================================================================================
; EEPROM_Init
;	Enables interrupts
; ================================================================================================
EEPROM_Init:
	sbi EECR, EERIE
	ret


; ================================================================================================
;	EEPROM_Write
;	Writes data to the EEPROM (blocks while EEPROM is not ready)
;	Parameters:
;		r16:	data to write
;		r17:	address to write to
; ================================================================================================
EEPROM_Write:
	sbic	EECR,EEPE				; Wait for completion of previous write
	rjmp	EEPROM_Write
	andi	r17, 0x7F
	out		EEAR, r17				; Set up address (r17) in address register
	out		EEDR,r16				; Write data (r16) to data register
	sbi		EECR,EEMPE				; Write logical one to EEMPE
	sbi		EECR,EEPE				; Start eeprom write by setting EEPE
	ret


; ================================================================================================
;	EEPROM_Read
;	Reads data from the EEPROM (blocks while EEPROM is not ready)
;	Parameters:
;		r16:	data that was read
;		r17:	address to read data from
; ================================================================================================
EEPROM_Read:
	sbic	EECR,EEPE				; Wait for completion of previous write
	rjmp	EEPROM_Read
	andi	r17, 0x7F
	out		EEAR, r17				; Set up address (r17) in address register
	sbi		EECR,EERE				; Start eeprom read by writing EERE
	in		r16,EEDR					; Read data from data register
	ret
