/*
 * spi.c
 *
 * Created: 9/7/2011 9:50:30 PM
 *  Author: Matthew
 */ 

#include "spi.h"


volatile static spi_rx_callback_t		_rx_callBack	= 0;
volatile static spi_tx_callback_t		_tx_callBack	= 0;
volatile static spi_asyncCallback_t		_spi_Complete	= 0;
volatile static uint8_t *				_pdata			= 0;
volatile static uint16_t				_length			= 0;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SPI_MasterInit(void)
{
	_rx_callBack = 0;
	_tx_callBack = 0;

	// Set SS line
	PORT_SPI |=	(1<<SS_SPI);

	// Set MOSI and SCK output, all others input
	DDR_SPI =	(1<<MOSI_SPI) |
				(1<<SCK_SPI) |
				(1<<SS_SPI);

	// set SPI speed
#if defined (SPI_SUPPORT_EMBEDDED)
	SPI_SPCR |=	(1<<SPE) |
				(1<<MSTR);
#elif defined (SPI_SUPPORT_BASIC)
	//SPI_SPCR 
#endif

	SPI_setSpeed();
	SPI_doubleSpeed();

	// prepare interrupt handling functions
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SPI_MasterTransmit(uint8_t cData)
{
	/* Start transmission */
	SPI_SPDR = cData;

	/* Wait for transmission complete */
	while(!(SPI_SPSR & (1<<SPI_IF)));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ISR(SPI_ISR)
{
	if (0 != _spi_Complete)
		_spi_Complete();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SPI_WriteBuffer(uint8_t * buffer, uint16_t length)
{
	if (0 == length)
		return;

	// copy values
	_pdata = buffer;
	_length = length;

	/* Wait for transmission complete */
	while(!(SPI_SPSR & (1<<SPI_IF)));

	// enable interrupt
	SPI_SPCR |= (1<<SPI_IE);

	SPI_SPDR = _pdata[--_length];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uint8_t SPI_ReadByte(void)
{
	return SPI_SPDR;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void SPI_WriteByte(uint8_t data)
{
	// Wait for transmission complete
	while(!(SPI_SPSR & (1<<SPI_IF)));

	SPI_SPDR = data;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// asynchronously writes a block of data to the SPI interface
void spi_writeByteAHandler(void)
{
	if (_length > 0)
	{
		// write another byte
		_length--;
		SPI_WriteByte(*_pdata++);
	}
	else
	{
		// end async operation
		SPI_SPSR &= ~(1<<SPI_IF);

		// invoke callback function
		if (0 != _tx_callBack)
			_tx_callBack();
	}
}
void SPI_WriteBytesA(uint8_t * pdata, uint16_t length, spi_tx_callback_t onComplete)
{
	_pdata			= pdata;
	_length			= length;
	_spi_Complete	= spi_writeByteAHandler;
	_tx_callBack	= onComplete;

	// start the transfer
	spi_writeByteAHandler();
}
