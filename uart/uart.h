#ifndef __UART_H
#define __UART_H



#include <util/setbaud.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <inttypes.h>
#include <stdio.h>
#include <types.h>
#include "../asyncTypes.h"

// ugh...
#include "../../core/board.h"


class Uart
{
public:
	typedef void (*uart_rx_callback_t)(char);
	typedef void (*uart_tx_callback_t)(void);
	typedef void (*uart_asyncCallback_t)(void);
	typedef char (*uart_readCallback_t)(void);

	typedef char (*f_reader_t)(volatile const char**);


	Uart(void);
	void init(void);

	void putstr(const char* pstr);
	void putstrM(const char* pstr);
	void putstrAM(const char * pstr, uart_asyncCallback_t callBack);

	char * getstr(char * pstr, uint16_t max);

	void beginReceive(uart_rx_callback_t callBack);
	void endReceive(void);

	void beginTransmit(uart_tx_callback_t callBack);
	void endTransmit(void);

	void write(char x);
	uint8_t dataWaiting(void);
	char read(void);

	void sendBuff(const char * buffer, uint16_t length);
	void receiveBuff(char * buffer, uint16_t length);

	void receiveHandler(char data);
	void transmitHandler(void);

private:
	inline char sram_read(const char** buff)
	{
		return *(*buff)++;
	}
	inline char pgm_read(const char** buff)
	{
		return pgm_read_byte((*buff)++);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void uart_putstrAMHandler(void)
	{
		char data = pgm_read_byte(_txAsyncData++);
		if (data == 0)
		{
			endTransmit();
			if (_txAsyncCallback)
				_txAsyncCallback();
			_txAsyncCallback = 0;
			_asyncBusy = 0;
		}		
		else
			write(data);
	}
	void uart_putstrAM(const char * pstr, uart_asyncCallback_t callBack)
	{
		while(_asyncBusy);
		
		_txAsyncCallback	= callBack;
		_txAsyncData		= pstr;
		_asyncBusy			= 1;

		beginTransmit(callBack);

		write(pgm_read_byte(_txAsyncData++));
	}


	uart_tx_callback_t		_uart_tx_callback;
	uart_rx_callback_t		_uart_rx_callback;
	const char*				_txAsyncData;
	uint16_t				_txAsyncLen;
	char					_asyncBusy;

	uart_asyncCallback_t	_txAsyncCallback;
	uart_asyncCallback_t	_rxAsyncCallback;

	char					_uartReceiveData;
};


#endif
