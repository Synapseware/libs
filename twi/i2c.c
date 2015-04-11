#include "i2c.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
volatile fVoidCallback i2c_callBack = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ISR(TWI_vect)
{
	// invoke the callback method if one is given
	if (0 != i2c_callBack)
		i2c_callBack();

	twi_led_off();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void i2cSendStart(void)
{
	// send start condition
	TWCR = TWI_START;
}
inline void i2cSendStartAsync(fVoidCallback cb)
{
	i2c_callBack = cb;
	TWCR = TWI_START | (1<<TWIE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void i2cSendStop(void)
{
	// transmit stop condition
    TWCR = TWI_STOP;
}
inline void i2cSendStopAsync(fVoidCallback cb)
{
	i2c_callBack = cb;
    TWCR =  TWI_STOP | (1<<TWIE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void i2cWaitForComplete(void)
{
#ifdef release
	// wait for i2c interface to complete operation
    while (!(TWCR & (1<<TWINT)));
#endif

	twi_led_off();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void i2cWaitForStop(void)
{
#ifdef release
	while (!(inb(TWCR) & BV(TWSTO)));
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void i2cSendByte(uint8_t data)
{
	twi_led_on();
	TWDR = data;
	TWCR = TWI_SEND;
}
inline void i2cSendByteAsync(uint8_t data, fVoidCallback cb)
{
	i2c_callBack = cb;
	twi_led_on();
	TWDR = data;
	TWCR = TWI_SEND | (1<<TWIE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void i2cAck(void)
{
	TWCR |= (1<<TWEA);
}
void i2cAckA(fVoidCallback cb)
{
	i2c_callBack = cb;
	TWCR = TWI_ACK | (1<<TWIE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void i2cNack(void)
{
	TWCR &= ~TWEA;
}
void i2cNackA(fVoidCallback cb)
{
	i2c_callBack = cb;
	TWCR = TWI_NACK | (1<<TWIE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uint8_t i2cGetReceivedByte(void)
{
	// retrieve received data uint8_t from i2c TWDR
	return TWDR;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uint8_t i2cGetStatus(void)
{
	// retrieve current i2c status from i2c TWSR
	// bits 1:0 on ATMega are for the prescaler, so mask them out!
	return TWSR & TWSR_STATUS_MASK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sends the specified value, repeat count times
void i2cMasterRawWriteRepeat(uint16_t count, uint8_t value)
{
	// send data
	while (count > 0)
	{
		i2cSendByte(value);
		i2cWaitForComplete();
		count--;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sends a block of raw-data over the TWI interface
void i2cMasterRawWrite(uint16_t length, uint8_t * data)
{
	// send data
	while (length > 0)
	{
		i2cSendByte(*data++);
		i2cWaitForComplete();
		length--;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sends a block of data without a trailing stop
uint8_t i2cMasterSendNoStopNI(uint8_t deviceAddr, uint16_t length, uint8_t * data)
{
	// send start condition
	i2cSendStart();
    i2cWaitForComplete();

	// send device address with write
	i2cSendByte(deviceAddr & I2C_WRITE);
	i2cWaitForComplete();

	// check if device is present and live
	// device did not ACK it's address,
	// data will not be transferred
	// return error
	if (TW_STATUS != TW_MT_SLA_ACK)
		return I2C_ERROR_NODEV;

	i2cMasterRawWrite(length, data);

	return I2C_OK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sends a block of data with a trailing stop
uint8_t i2cMasterSendNI(uint8_t deviceAddr, uint16_t length, uint8_t * data)
{
	uint8_t retval = i2cMasterSendNoStopNI(deviceAddr, length, data);

	// transmit stop condition
	// leave with TWEA on for slave receiving
	i2cSendStop();
	i2cWaitForStop();

	return retval;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint8_t i2cMasterReceiveNI(uint8_t deviceAddr, uint16_t length, uint8_t * data)
{
	// send start condition
	i2cSendStart();
	i2cWaitForComplete();

	// send device address with read
	i2cSendByte(deviceAddr | I2C_READ);
	i2cWaitForComplete();

	// check if device is present and alive
	// device did not ACK it's address,
	// data will not be transferred
	// return error
	if (TW_STATUS != TW_MR_SLA_ACK)
		return I2C_ERROR_NODEV;

	// accept receive data and ack it
	while (length > 1)
	{
		i2cAck();
		i2cWaitForComplete();
		*data++ = i2cGetReceivedByte();
		length--;
	}

	// accept receive data and nack it (last-uint8_t signal)
	i2cNack();
	i2cWaitForComplete();
	*data++ = i2cGetReceivedByte();

	// transmit stop condition
	i2cSendStop();
	i2cWaitForStop();

	return I2C_OK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void i2cInit(unsigned short bitrateKHz)
{
	// bring pins high
	PORTC |= (1<<TWI_SDA) | (1<<TWI_SCL);

	// set pins as output
	DDRC |= (1<<TWI_SDA) | (1<<TWI_SCL);

	enable_twi_led();
	twi_led_off();

	TWCR = 0;

	// set i2c bitrate
	// SCL freq = F_CPU/(16+2*TWBR)) (prescaler = 1)

	// calculate bitrate division
	uint8_t bitrate_div = ((F_CPU/1000L)/bitrateKHz);
	if (bitrate_div >= 16)
		bitrate_div = (bitrate_div-16) >> 1;

	TWBR = bitrate_div;
}
