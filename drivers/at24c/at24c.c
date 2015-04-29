#include "at24c.h"

static	uint16_t		_page;
static	uint16_t		_address;
static	uint8_t			_device;

static	uint8_t			_data;
static	uint8_t *		_pbuffer;
static	uint16_t		_bufferLen;

static	uint8_t			_asyncStep;
static	uint8_t			_busy;

static	fStatusCallback _onComplete;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// invokes any onComplete method and finalizes the async operation
void _eeComplete(uint8_t result)
{
	if (0 != _onComplete)
		_onComplete(result);

	_onComplete		= 0;
	_asyncStep		= ASYNC_COMPLETE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// abort any current transactions
void _asyncError(void)
{
	i2cSendStop();

	_eeComplete(0xFF);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Formats the static device and address values, and persists the given page value
// This can be used to map any page number to a range of devices.  Each device supports
// 512 pages.  So for 2 devices, total pages are 1024 (0-1023 pages).
void ee_mapdevice(uint16_t page)
{
	_page = page;

	// capture A2, A1 and P0 bits into addrBits value
	uint8_t lsb = page >> 8;
	uint8_t addrBits = (lsb << 1) & 0b00001110;

	// merge generic device address with high-order address bits
	_device = AT24C1024_ADDRESS | addrBits;

	// take page LSB * 256.
	_address = (page << 8) & 0xFF00;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// increments the address value, and switches devices when it rolls over
void incrementAddress(void)
{
	// increment address
	_address++;

	// check for address boundary
	if ((_address & 0x00FF) != 0)
		return;

	// increment page
	ee_mapdevice(_page + 1);

	// check for address rollover
	if (_address != 0)
		return;

	// remap the device	
	ee_setpage(_page & ((AT24C1024_MAX_DATA/AT24C1024_PAGE_SIZE) - 1));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Maps the selected page to the device & address, and writes the device and address
// bytes to the device, leaving it in a state to accept more data.
EE_STATUS __writeActiveAddress(uint16_t page)
{
	// map device and address to selected page
	ee_mapdevice(page);

	// send start condition
	i2cSendStart();
	i2cWaitForComplete();

	// send device address with write
	i2cSendByte(_device & I2C_WRITE);
	i2cWaitForComplete();

	// check if device is present and live
	// device did not ACK it's address,
	// data will not be transferred
	// return error
	if (i2cGetStatus() != TW_MT_SLA_ACK)
	{
		i2cSendStop();
		return I2C_ERROR_NODEV;
	}

	// address MSB
	i2cSendByte(_address >> 8);
	i2cWaitForComplete();

	// address LSB
	i2cSendByte(_address & 0xFF);
	i2cWaitForComplete();

	return I2C_OK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Set the current address
EE_STATUS ee_setpage(uint16_t page)
{
	// set current address on device
	__writeActiveAddress(page);

	// transmit stop condition
	// leave with TWEA on for slave receiving
	i2cSendStop();
	i2cWaitForStop();
	//i2cWaitForComplete();

	return I2C_OK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ee_setPageAHandler(void)
{
	// setting the current page in the device requires setting up the
	// device for a 'write' operation, but aborting the write before
	// sending any data to the chip.  a re-start is initiated with the
	// device which starts the sequential read
	switch (_asyncStep)
	{
		// send start signal
		case ASYNC_SEND_START:
			_asyncStep = ASYNC_SEND_DEVICE;
			i2cSendStartAsync(&ee_setPageAHandler);
			return;

		// send device address with write
		case ASYNC_SEND_DEVICE:
			if (i2cGetStatus() != TW_START)
			{
				_asyncError();
				return;
			}

			_asyncStep = ASYNC_SEND_ADDRMSB;
			i2cSendByteAsync(_device & I2C_WRITE, &ee_setPageAHandler);
			return;

		// send address MSB
		case ASYNC_SEND_ADDRMSB:
			if (i2cGetStatus() != TW_MT_SLA_ACK)
			{
				_asyncError();
				return;
			}

			_asyncStep = ASYNC_SEND_ADDRLSB;
			i2cSendByteAsync(_address >> 8, &ee_setPageAHandler);
			return;

		// send address LSB
		case ASYNC_SEND_ADDRLSB:
			if (i2cGetStatus() != TW_MT_DATA_ACK)
			{
				_asyncError();
				return;
			}

			_asyncStep = ASYNC_SEND_STOP;
			i2cSendByteAsync(_address & 0xff, &ee_setPageAHandler);
			return;

		// end the transaction
		case ASYNC_SEND_STOP:
			if (i2cGetStatus() != TW_MT_DATA_ACK)
			{
				_asyncError();
				return;
			}

			i2cSendStop();

			_eeComplete(0);
			return;
	}
}
void ee_setpageA(uint16_t page, fStatusCallback callBack)
{
	if (_asyncStep != ASYNC_COMPLETE)
		return;

	_asyncStep		= ASYNC_SEND_START;
	_onComplete		= callBack;

	ee_mapdevice(page);

	ee_setPageAHandler();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Reads a single byte of data from the EEPROM chip
uint8_t ee_read(void)
{
	// sequential read is:
	//	Master		EEPROM
	//	Start->
	//				<-Ack
	//	Device->
	//				<-Ack
	//				<-Send Byte
	//	(N)Ack->
	//	Stop->
	// The TWI control module knows to ack/nack the response to the slave
	// when in master-receive mode
	
	// send start condition
	i2cSendStart();
	i2cWaitForComplete();

	// send SLA+R
	i2cSendByte(_device | I2C_READ);
	i2cWaitForComplete();

	// nack the last byte sent
	i2cNack();
	i2cWaitForComplete();
	uint8_t data = i2cGetReceivedByte();

	// increment the address pointer	
	//incrementAddress();

	return data;
}
void ee_readEnd(void)
{
	//i2cSendStop();
	//i2cWaitForStop();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initiates an async read operation
void ee_readAHandler(void)
{
	switch(_asyncStep)
	{
		// send a start signal
		case ASYNC_NEXT_START:
			_asyncStep = ASYNC_NEXT_DEVICE;
			i2cSendStartAsync(ee_readAHandler);
			break;

		// send SLA+R
		case ASYNC_NEXT_DEVICE:
			if (i2cGetStatus() != TW_START)
			{
				_asyncError();
				break;
			}

			_asyncStep = ASYNC_NEXT_NACK;
			i2cSendByteAsync(_device | I2C_READ, ee_readAHandler);
			break;

		// setup TWI module to NACK the response
		case ASYNC_NEXT_NACK:
			if (i2cGetStatus() != TW_MR_SLA_ACK)
			{
				_asyncError();
				break;
			}

			_asyncStep = ASYNC_NEXT_READ;
			i2cNackA(ee_readAHandler);
			break;

		// capture the received data	
		case ASYNC_NEXT_READ:
			if (i2cGetStatus() != TW_MR_DATA_NACK)
			{
				_asyncError();
				break;
			}

			// read received data
			_data = i2cGetReceivedByte();

			i2cSendStop();

			// mark transaction as complete
			_eeComplete(_data);

			// increment the address pointer	
			incrementAddress();

			break;
	}
}
void ee_readA(fStatusCallback callBack)
{
	if (_asyncStep != ASYNC_COMPLETE)
		return;

	_onComplete		= callBack;
	_asyncStep		= ASYNC_NEXT_START;

	ee_readAHandler();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Performs an asynchronous page read
void ee_readBytesHandler(void)
{
	// setting the current page in the device requires setting up the
	// device for a 'write' operation, but aborting the write before
	// sending any data to the chip.  a re-start is initiated with the
	// device which starts the sequential read
	switch (_asyncStep)
	{
		// send start signal
		case ASYNC_MULTI_START:
			_asyncStep = ASYNC_MULTI_DEVICE;
			i2cSendStartAsync(ee_readBytesHandler);
			break;

		// send device address with write
		case ASYNC_MULTI_DEVICE:
			if (i2cGetStatus() != TW_START)
			{
				_asyncError();
				break;
			}

			_asyncStep = ASYNC_MULTI_ADDRMSB;
			i2cSendByteAsync(_device & I2C_WRITE, ee_readBytesHandler);
		break;

		// send address MSB
		case ASYNC_MULTI_ADDRMSB:
			if (i2cGetStatus() != TW_MT_SLA_ACK)
			{
				_asyncError();
				break;
			}

			_asyncStep = ASYNC_MULTI_ADDRLSB;
			i2cSendByteAsync(_address >> 8, ee_readBytesHandler);
		break;

		// send address LSB
		case ASYNC_MULTI_ADDRLSB:
			if (i2cGetStatus() != TW_MT_DATA_ACK)
			{
				_asyncError();
				break;
			}

			_asyncStep = ASYNC_MULTI_READ;
			i2cSendByteAsync(_address & 0xff, ee_readBytesHandler);
		break;

		// prepare appropriate reply for recceived byte(s)
		case ASYNC_MULTI_READ:
			_asyncStep = ASYNC_MULTI_NEXT;
			if (_bufferLen)
				i2cAckA(ee_readBytesHandler);
			else
				i2cNackA(ee_readBytesHandler);
			break;

		// get the byte and
		case ASYNC_MULTI_NEXT:
			if (_bufferLen > 0)
			{
				_bufferLen--;
				*_pbuffer++ = i2cGetReceivedByte();
				_asyncStep = ASYNC_MULTI_READ;

				// increment the address pointer	
				incrementAddress();
			}
			else
				_asyncStep = ASYNC_MULTI_STOP;
			break;

		// end the transaction
		case ASYNC_MULTI_STOP:
			if (i2cGetStatus() != TW_MT_DATA_ACK)
			{
				_asyncError();
				break;
			}

			i2cSendStop();

			_eeComplete(0);
		break;
	}
}
void ee_readBytesA(uint16_t page, uint16_t length, uint8_t * data, fStatusCallback callBack)
{
	if (_asyncStep != ASYNC_COMPLETE)
		return;

	_pbuffer		= data;
	_bufferLen		= length;

	_onComplete		= callBack;
	_asyncStep		= ASYNC_MULTI_START;


	ee_readBytesHandler();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// reads the specified page of data from the eeprom chip
EE_STATUS ee_readBytes(uint16_t page, uint16_t length, uint8_t * data)
{
	EE_STATUS status = __writeActiveAddress(page);
	if (I2C_OK != status)
		return status;

	// perform block read
	status = i2cMasterReceiveNI(AT24C1024_ADDRESS, length, data);
	if (I2C_OK != status)
		return status;

	return I2C_OK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// writes the specified page of data to the eeprom chip
// Page write: Start + device + 2 address bytes + 256 bytes of data
EE_STATUS ee_writePage(uint16_t page, void * data)
{
	// initiate a page write sequence
	if (__writeActiveAddress(page) != I2C_OK)
		return I2C_ERROR;

	// perform block write
	i2cMasterRawWrite(AT24C1024_PAGE_SIZE, (uint8_t*)data);

	// terminate the page write
	ee_putBytesEnd();

	return I2C_OK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Writes an arbitrary block of data to the EEPROM.
EE_STATUS ee_writeBytes(uint16_t address, void * data, uint8_t length)
{
	// initiate a page write sequence
	if (__writeActiveAddress(address) != I2C_OK)
		return I2C_ERROR;

	// perform block write
	i2cMasterRawWrite((uint16_t) length, (uint8_t*)data);

	// terminate the page write
	ee_putBytesEnd();

	return I2C_OK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// setup page on eeprom and leave it open
EE_STATUS ee_putByteStart(uint16_t page)
{

	return __writeActiveAddress(page);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Writes a byte of data to the current page.  Meant to be called once a call to
// ee_setpage has taken place.
void ee_putByte(uint8_t data)
{
	// send the byte
	i2cSendByte(data);
	i2cWaitForComplete();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Closes a page write sequence.  This blocks while the EEPROM is busy writing the data.
void ee_putBytesEnd(void)
{
	// terminate write operation
	i2cSendStop();
	//i2cWaitForComplete();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ee_poll(void)
{
	/*
	ACKNOWLEDGE POLLING: Once the internally timed write cycle has started and the
	EEPROM inputs are disabled, acknowledge polling can be initiated. This involves sending a
	start condition followed by the device address word. The read/write bit is representative of the
	operation desired. Only if the internal write cycle has completed will the EEPROM respond with
	a zero, allowing the read or write sequence to continue.
	*/

	while(1)
	{
		i2cSendStart();
		i2cWaitForComplete();

		i2cSendByte(_device & I2C_WRITE);
		i2cWaitForComplete();

		if (i2cGetStatus() == TW_MT_SLA_ACK)
		{
			i2cSendStop();
			//i2cWaitForComplete();
			break;
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Performs an I2C acknoledge for the EEPROM.  If the device is busy, it will NACK
bool ee_busy(void)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte(_device & I2C_WRITE);
	i2cWaitForComplete();

	if (i2cGetStatus() == TW_MT_SLA_ACK)
	{
		i2cSendStop();
		return false;
	}

	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ee_init(unsigned short bitrateKHz)
{
	_data			= 0;
	_page			= 0;
	_address		= 0;
	_device			= 0;
	_busy			= 0;
	
	_pbuffer		= 0;
	_bufferLen		= 0;

	_onComplete		= 0;
	_asyncStep		= ASYNC_COMPLETE;

	// initialize I2C interface
	i2cInit(bitrateKHz);
}
