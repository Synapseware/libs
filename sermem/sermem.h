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
#include "../events/events.h"


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


class Sermem
{
public:
	Sermem(Uart* uart);
	void init(void);
	void showHelp(void);
	uint8_t putFile(void);
	uint8_t getFile(void);
	void format(void);
	void writeCannedData(void);
	void putstr(const char * pstr);
	void process(char data);
	void getFileCallback(void);

private:

	Uart*		_uart;
	uint8_t		_transferPageComplete;
	uint16_t	_bytesTransfered;
	uint32_t	_transferSize;

	uint8_t		_autoMode;

	uint8_t		last_rx;
	uint8_t		last_tx;
	uint8_t		rx_complete;
	uint8_t		tx_complete;
};



#endif
