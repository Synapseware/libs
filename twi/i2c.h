#ifndef I2C_H
#define IC2_H

#include <types.h>
#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "../asyncTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
Critical TWI/I2C events are:
	Start/Repeated Start
	Byte RX/TX
	Stop

(N)Ack are part of the message packet, and as such, do not raise TWINT events.

*/



#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega48__)
	#define TWI_SDA				PC4
	#define TWI_SCL				PC5
#endif

#if defined (twi_led_PIN) && defined (twi_led_PORT) && defined (twi_led_DDR)
	#define enable_twi_led()		twi_led_DDR |= (1<<twi_led_PIN)
	#define twi_led_on()			twi_led_PORT &= ~(1<<twi_led_PIN)
	#define twi_led_off()			twi_led_PORT |= (1<<twi_led_PIN)
#else
	#warning "LED defines are missing.  There will be no diagnostics."
	#define enable_twi_led()
	#define twi_led_on()
	#define twi_led_off()
#endif


// defines and constants
static const uint8_t TWCR_CMD_MASK			= 0x0F;
static const uint8_t TWSR_STATUS_MASK		= 0xF8;

// return values
static const uint8_t I2C_OK					= 0x00;
static const uint8_t I2C_ERROR_NODEV		= 0x01;
static const uint8_t I2C_ERROR				= 0x02;

// WRITE/READ masks and flags
static const uint8_t I2C_WRITE				= 0xFE;
static const uint8_t I2C_READ				= 0x01;

#define TWI_PRESCALE			(0<<TWPS1) | (0<<TWPS0)
#define TWI_ACK					TWI_PRESCALE | (1<<TWINT) | (1<<TWEN) | (1<<TWEA)
#define TWI_NACK				TWI_PRESCALE | (1<<TWINT) | (1<<TWEN)
#define TWI_SEND				TWI_PRESCALE | (1<<TWINT) | (1<<TWEN)
#define TWI_START				TWI_PRESCALE | (1<<TWINT) | (1<<TWEN) | (1<<TWSTA)
#define TWI_STOP				TWI_PRESCALE | (1<<TWINT) | (1<<TWEN) | (1<<TWSTO)
#define TWI_ENDINT				TWI_PRESCALE | (1<<TWINT)


//! Initialize I2C (TWI) interface
void i2cInit(unsigned short bitrateKHz);

// Low-level I2C transaction commands
//! Send an I2C start condition in Master mode
void i2cSendStart(void);
void i2cSendStartAsync(fVoidCallback cb);

//! Send an I2C stop condition in Master mode
void i2cSendStop(void);
void i2cSendStopAsync(fVoidCallback cb);

void i2cWaitForStop(void);

//! Wait for current I2C operation to complete
void i2cWaitForComplete(void);

//! Send an (address|R/W) combination or a data unsigned char over I2C
void i2cSendByte(unsigned char data);
void i2cSendByteAsync(unsigned char data, fVoidCallback cb);

// Send an ACK signal
void i2cAck(void);
void i2cAckA(fVoidCallback cb);

// Send a NACK signal
void i2cNack(void);
void i2cNackA(fVoidCallback cb);

//! Pick up the data that was received with i2cReceiveByte()
unsigned char i2cGetReceivedByte(void);

//! Get current I2c bus status from TWSR
unsigned char i2cGetStatus(void);

// Sends the specified value, repeat count times
void i2cMasterRawWriteRepeat(uint16_t repeat, unsigned char value);

// Sends a block of raw-data over the TWI interface
void i2cMasterRawWrite(uint16_t length, unsigned char * data);

//! send I2C data to a device on the bus (non-interrupt based) without a trailing stop
unsigned char i2cMasterSendNoStopNI(unsigned char deviceAddr, uint16_t length, unsigned char * data);

//! send I2C data to a device on the bus (non-interrupt based)
unsigned char i2cMasterSendNI(unsigned char deviceAddr, uint16_t length, unsigned char * data);

//! receive I2C data from a device on the bus (non-interrupt based)
unsigned char i2cMasterReceiveNI(unsigned char deviceAddr, uint16_t length, unsigned char * data);


#ifdef  __cplusplus
}
#endif

#endif

