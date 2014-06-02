#ifndef _SPIEEPROM_H
#define _SPIEEPROM_H


#include <types.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>


#include "../drivers/at24c/at24c.h"
#include "../uart/uart.h"



#define CMD_ACK					'A'
#define CMD_COMPLETE			'C'
#define CMD_ERROR				'E'
#define CMD_ABORT				'Q'
#define CMD_HELLO				'H'
#define CMD_BLOCK_SIZE			'B'
#define CMD_TRANSFER_SIZE		'T'
#define CMD_MODE				'M'


#define TRANSFER_ACK			0xFF
#define TRANSFER_NACK			0xDE
#define TRANSFER_ERR			0x00
#define TRANSFER_SUCCESS		0x01





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void spie_init(void);
void spie_showHelp(void);
uint8_t spie_putFile(void);
uint8_t spie_getFile(void);
void spie_format(void);
void spie_writeCannedData(void);
void spie_putstr(const char * pstr);
void spie_process(char data);




#endif
