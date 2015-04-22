#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <inttypes.h>

#include "uart.h"


static Uart * _thisUart = NULL;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Default constructor
Uart::Uart(void)
{
	_uart_tx_callback	= 0;
	_uart_rx_callback	= 0;
	_txAsyncData		= 0;
	_txAsyncLen			= 0;
	_asyncBusy			= 0;

	_txAsyncCallback	= 0;
	_rxAsyncCallback	= 0;

	_rxAsyncData		= 0;
	_rxAsyncLen			= 0;
	_uartReceiveData	= 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Starts asynchronous receive mode
void Uart::beginReceive(uart_rx_callback_t callBack)
{
	_uart_rx_callback = callBack;
	UCSR0B |= (1<<RXCIE0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ends asynchronous receive mode
void Uart::endReceive(void)
{
	UCSR0B &= ~(1<<RXCIE0);
	_uart_rx_callback = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Starts asynchronous transmit mode
void Uart::beginTransmit(uart_tx_callback_t callBack)
{
	_uart_tx_callback = callBack;
	UCSR0B |= (1<<TXCIE0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ends asynchronous transmit mode
void Uart::endTransmit(void)
{
	UCSR0B &= ~(1<<TXCIE0);
	_uart_tx_callback = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// writes a string from SRAM
void Uart::putstr(const char * pstr)
{
	char data;
	while (0 != (data = *pstr++))
		write(data);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// writes a string from program memory
void Uart::putstrM(const char * pstr)
{
	char data;
	while (0 != (data = pgm_read_byte(pstr++)))
		write(data);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// gets a string of the specified size and stores it in SRAM at pstr
char * Uart::getstr(char * pstr, uint16_t max)
{
	char * str = pstr;
	uint8_t data;
	max--;			// make sure we have room for the null character
	while (max)
	{
		data = read();
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
void Uart::write(char data)
{
	// wait for empty receive buffer
	while (0 == (UCSR0A & (1<<UDRE0)));

	// send
	UDR0 = data;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 
void Uart::asyncRead(uint8_t data)
{
	// terminate async receive	
	endReceive();

	_uartReceiveData = data;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Reads a byte of data from the UART.
char Uart::read(void)
{
	// wait for data
	while (0 == (UCSR0A & (1<<RXC0)));

	return UDR0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint8_t Uart::dataWaiting(void)
{
	// returns 0 if no data waiting
	return (UCSR0A & (1<<RXC0));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Uart::sendBuff(const char * buffer, uint16_t length)
{
	while (length)
	{
		write(*buffer++);
		length--;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Uart::receiveBuff(char * buffer, uint16_t length)
{
	endReceive();
	while (length)
	{
		*buffer++ = read();
		length--;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initialize the USART/UART
void Uart::init(void)
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
// Should be called by the UART RX ISR
void Uart::receiveHandler(char data)
{
	if (_uart_rx_callback)
		_uart_rx_callback(data);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Should be called by the UART TX ISR
void Uart::transmitHandler(void)
{
	if (_uart_tx_callback)
		_uart_tx_callback();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// receive buffer interrupt vector
ISR(USART_RX_vect)
{
#ifdef serial_led_on
	serial_led_on();
#endif
	uint8_t data = UDR0;

	if (_thisUart)
		_thisUart->receiveHandler(data);
#ifdef serial_led_off
	serial_led_off();
#endif
}

/*
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// transmit interrupt vector
ISR(USART_TX_vect)
{
	#ifdef serial_led_on
	serial_led_on();
	#endif

	if (_thisUart)
		_thisUart->transmitHandler();

	#ifdef serial_led_off
	serial_led_off();
	#endif
}
*/
