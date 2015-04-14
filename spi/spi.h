#ifndef SPI_H_
#define SPI_H_
/*
 * spi.h
 *
 * Created: 9/7/2011 9:50:37 PM
 *  Author: Matthew
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif


// SPI Port definitions
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega48__)
	#define PORT_SPI		PORTB
	#define DDR_SPI			DDRB
	#define SCK_SPI			PORTB5
	#define MISO_SPI		PORTB4
	#define MOSI_SPI		PORTB3
	#define SS_SPI			PORTB2
 	#define SPI_SPCR		SPCR
 	#define SPI_SPSR		SPSR
 	#define SPI_SPDR		SPDR
 	#define SPI_SUPPORT_EMBEDDED
#elif defined (__AVR_ATmega8U2__)
 	#define PORT_SPI		PORTB
	#define DDR_SPI			DDRB
	#define SCK_SPI			PORTB1
	#define MISO_SPI		PORTB3
	#define MOSI_SPI		PORTB2
	#define SS_SPI			PORTB0
 	#define SPI_SPCR		SPCR
 	#define SPI_SPSR		SPSR
 	#define SPI_SPDR		SPDR
 	#define SPI_SUPPORT_EMBEDDED
#elif defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__)
 	#ifndef USER_SS_LINE
 		#error You must define USER_SS_LINE, on the same port as MISO, MOSI and SCK
 	#endif
 	#define PORT_SPI		PORTA
	#define DDR_SPI			DDRA
	#define SCK_SPI			PORTA4
	#define MISO_SPI		PORTA5
	#define MOSI_SPI		PORTA6
	#define SS_SPI			USER_SS_LINE
 	#define SPI_SPCR		USICR
 	#define SPI_SPSR		USISR
 	#define SPI_SPDR		USIDR
 	#define SPI_SUPPORT_BASIC
#else
 	#error Unsupported or undefined MCU
#endif


// non-part specific defines
#if defined(SPI_SUPPORT_EMBEDDED)
	#define SPI_IF			SPIF
	#define SPI_IE			SPIE
	#define SPI_ISR			SPI_STC_vect
#elif defined(SPI_SUPPORT_BASIC)
 	#define SPI_IF			USIOIF
	#define SPI_IE			USIOIE
	#define SPI_ISR			USI_OVF_vect
#else
 	#error SPI support not defined
#endif


// Declare SPI register speed settings
#if defined (SPI_CLK_4)
	#define SPI_setSpeed()	SPI_SPCR |= (0)
#elif defined (SPI_CLK_16)
	#define SPI_setSpeed()	SPI_SPCR |= (1<<SPR0)
#elif defined (SPI_CLK_64)
	#define SPI_setSpeed()	SPI_SPCR |= (1<<SPR1)
#elif defined (SPI_CLK_128)
	#define SPI_setSpeed()	SPI_SPCR |= (1<<SPR0)|(1<<SPR1)
#endif

#if defined (SPI_CLK_2X)
	#define SPI_doubleSpeed()	SPI_SPSR |= (1<<SPI2X)
#else
	#define SPI_doubleSpeed()	SPI_SPSR &= ~(1<<SPI2X)
#endif


// Declare callback functions
typedef void (*spi_rx_callback_t)(uint8_t);
typedef void (*spi_tx_callback_t)(void);
typedef void (*spi_asyncCallback_t)(void);



// Declare public library functions
void SPI_MasterInit(void);
void SPI_MasterTransmit(uint8_t cData);
void SPI_WriteBytes(uint8_t * buffer, uint16_t length);
uint8_t SPI_ReadByte(void);
void SPI_WriteByte(uint8_t data);
void SPI_WriteBytesA(uint8_t * pdata, uint16_t length, spi_tx_callback_t onComplete);



#ifdef  __cplusplus
}
#endif

#endif /* SPI_H_ */
