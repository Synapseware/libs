#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <inttypes.h>

#include "uart.h"

// USART async receive values
volatile static uart_tx_callback_t		uart_tx_callback	= 0;
volatile static uart_rx_callback_t		uart_rx_callback	= 0;
volatile const char*					_txAsyncData		= 0;
volatile uint16_t						_txAsyncLen			= 0;
volatile char							_asyncBusy			= 0;

volatile static uart_asyncCallback_t	_txAsyncCallback	= 0;
volatile static uart_asyncCallback_t	_rxAsyncCallback	= 0;

volatile char*							_rxAsyncData		= 0;
volatile uint16_t						_rxAsyncLen			= 0;
volatile static char					_uartReceiveData	= 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
typedef char (*f_reader_t)(volatile const char**);
inline static char sram_read(volatile const char** buff)
{
	return *(*buff)++;
}
inline static char pgm_read(volatile const char** buff)
{
	return pgm_read_byte((*buff)++);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Starts asynchronous receive mode
inline void uartBeginReceive(uart_rx_callback_t callBack)
{
	uart_rx_callback = callBack;
	UCSR0B |= (1<<RXCIE0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ends asynchronous receive mode
inline void uartEndReceive()
{
	UCSR0B &= ~(1<<RXCIE0);
	uart_rx_callback = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Starts asynchronous transmit mode
inline void uartBeginSend(uart_tx_callback_t callBack)
{
	uart_tx_callback = callBack;
	UCSR0B |= (1<<TXCIE0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ends asynchronous transmit mode
void uartEndTransmit()
{
	UCSR0B &= ~(1<<TXCIE0);
	uart_tx_callback = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// writes a string from program memory
void uart_putstrM(const char * pstr)
{
	char data;
	while (0 != (data = pgm_read_byte(pstr++)))
		uartWrite(data);
}
void uart_putstrAMHandler(void)
{
	char data = pgm_read_byte(_txAsyncData++);
	if (data == 0)
	{
		uartEndTransmit();
		if (_txAsyncCallback)
			_txAsyncCallback();
		_txAsyncCallback = 0;
		_asyncBusy = 0;
	}		
	else
		uartWrite(data);
}
void uart_putstrAM(const char * pstr, uart_asyncCallback_t callBack)
{
	while(_asyncBusy);
	
	_txAsyncCallback	= callBack;
	_txAsyncData		= pstr;
	_asyncBusy			= 1;

	uartBeginSend(uart_putstrAMHandler);

	uartWrite(pgm_read_byte(_txAsyncData++));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// writes a string from SRAM
void uart_putstr(const char * pstr)
{
	char data;
	while (0 != (data = *pstr++))
		uartWrite(data);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// gets a string of the specified size and stores it in SRAM at pstr
char * uart_getstr(char * pstr, uint16_t max)
{
	char * str = pstr;
	uint8_t data;
	max--;			// make sure we have room for the null character
	while (max)
	{
		data = uartRead();
		if (data == '\r' || data == '\n' || data == '\0')
			break;
		*str = data;
		str++;
		max--;
	}
	*str='\0';
	
	return pstr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// writes a character of data to the UART
void uartWrite(char data)
{
	// wait for empty receive buffer
	while (0 == (UCSR0A & (1<<UDRE0)));

	// send
	UDR0 = data;
}

void uartAsyncRead(uint8_t data)
{
	// terminate async receive	
	uartEndReceive();

	_uartReceiveData = data;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Reads a byte of data from the UART.
char uartRead()
{
	// wait for data
	while (0 == (UCSR0A & (1<<RXC0)));

	return UDR0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint8_t uartDataWaiting()
{
	// returns 0 if no data waiting
	return (UCSR0A & (1<<RXC0));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void uartSendBuff(const char * buffer, uint16_t length)
{
	while (length)
	{
		uartWrite(*buffer++);
		length--;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void uartReceiveBuff(char * buffer, uint16_t length)
{
	uartEndReceive();
	while (length)
	{
		*buffer++ = uartRead();
		length--;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initialize the USART/UART
void uart_init()
{
#if defined (serial_led_en) && defined (serial_led_off)
	serial_led_en();
	serial_led_off();
#endif

	UCSR0A = 0;
	UCSR0B = 0;
	UCSR0C = 0;

	// set baud values from macro
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	// enable uart RX and TX
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	// set 8N1 frame format
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// receive buffer interrupt vector
ISR(USART_RX_vect)
{
#ifdef serial_led_on
	serial_led_on();
#endif
	uint8_t data = UDR0;
	if (uart_rx_callback)
		uart_rx_callback(data);
#ifdef serial_led_off
	serial_led_off();
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// transmit interrupt vector
ISR(USART_TX_vect)
{
#ifdef serial_led_on
	serial_led_on();
#endif
	if (uart_tx_callback)
		uart_tx_callback();
#ifdef serial_led_off
	serial_led_off();
#endif
}
