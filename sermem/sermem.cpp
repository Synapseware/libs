#include "sermem.h"



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sermem::Sermem(Uart* uart)
{
	_uart = uart;

	_transferPageComplete	= 0;
	_bytesTransfered		= 0;
	_transferSize			= 0;

	_autoMode				= 0;

	last_rx					= 0;
	last_tx					= 0;
	rx_complete				= 0;
	tx_complete				= 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Retrieves a file from the user.  Host to client transfer works as follows:
// HOST:  The PC
// CLIENT: This device
// 1: Host sends ACK byte when ready
// 2: Client responds with transfer block size
// 3: Host responds with total transfer size (must be a multiple of the block size)
// 4: Client response with ack if size OK, or nack if too large
// 5: Host starts sending data, 1 block at a time
//		5a.  When block done, host waits for ACK/NACK before sending next block
// 6: Client responds with ACK when block write is complete
uint8_t Sermem::putFile()
{
	// Step 0:
	// Client sent a 'W' to enter write mode
	
	// Step 1:
	// Let host know we are in WRITE func
	_uart->write(TRANSFER_ACK);

	// Step 2:
	// Let host know we are ready by sending the transfer block size
	// this is byte size * 8.  So for 256 byte blocks, we'd send
	// 32. 32 * 8 = 256, 256/8 = 32
	uint16_t page = AT24C1024_PAGE_SIZE;
	_uart->sendBuff((const char*)&page, sizeof(uint16_t));
	page = 0;

	// Step 3:
	// Host will now give us 4 bytes, indicating the size of the transfer.
	_uart->receiveBuff((char*)&_transferSize, sizeof(uint32_t));

	// Step 4:
	// Check that we can accept the transfer!
	if (_transferSize > AT24C1024_MAX_DATA)
	{
		// transfer is too large - abort!
		_uart->write(TRANSFER_NACK);
		return TRANSFER_ERR;
	}

	// begin the page write, starting with page 0
	ee_putByteStart(page++);

	// transfer size is good (less than AT24C1024_MAX_DATA bytes)
	// host will now begin to send data
	// setup for async receive

	// ACK the size of the transfer
	_uart->write(TRANSFER_ACK);

	// get data!
	_bytesTransfered = 0;
	while (0 != _transferSize)
	{
		// increment the byte counter and save the byte to the EEPROM
		uint8_t data = _uart->read();
		ee_putByte(data);

		// update transfer info
		_bytesTransfered++;
		_transferSize--;

		// wait for a pages worth of data, or the end of the write transmission of data
		if (_bytesTransfered != AT24C1024_PAGE_SIZE)
			continue;

		// end the write cycle
		ee_putBytesEnd();

		// poll the device for write-complete
		ee_poll();

		// close the current page
		_bytesTransfered = 0;
		ee_putByteStart(page++);

		// ack the page
		_uart->write(TRANSFER_ACK);
	}

	if (_bytesTransfered != 0)
	{
		ee_putBytesEnd();
		ee_poll();
	}

	// nack the end of the transfer
	_uart->write(TRANSFER_NACK);

	// return success
	return TRANSFER_SUCCESS;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Reads all the data from the chip and sends it to the host
// HOST:  This device
// CLIENT: The PC
// 1: Host sends ACK byte when ready
// 2: Client responds with transfer block size
// 3: Host responds with total transfer size (must be a multiple of the block size)
// 4: Client response with ack if size OK, or nack if too large
// 5: Host starts sending data, 1 block at a time
//		5a.  When block done, host waits for ACK/NACK before sending next block
// 6: Client responds with ACK when block write is complete
uint8_t Sermem::getFile(void)
{
	// Step 1:
	// Wait for ACK from host to let us know they are ready to send
	_uart->write(TRANSFER_ACK);

	// Step 2:
	// Get block size for transfer from client
	uint8_t transferSizeMultiple = 0;
	_uart->receiveBuff((char*)&transferSizeMultiple, sizeof(uint8_t));
	if (0 == transferSizeMultiple)
		return TRANSFER_ERR;
	_transferSize = transferSizeMultiple * 8;

	// Step 3:
	// Let client know how large of a transfer this will be
	uint32_t totalTransferSize = AT24C1024_MAX_DATA;
	_uart->sendBuff((char*)&totalTransferSize, sizeof(uint32_t));

	// Step 4:
	// Make sure client ACK'd our size
	if (TRANSFER_ACK != _uart->read())
		return TRANSFER_ERR;

	// Step 5:
	// Send entire file to client!  :)

	// spin while transfer in progress
	uint16_t page = 0;
	while(_transferSize && page < AT24C1024_PAGE_COUNT)
	{
		// setup for sequential read from EEPROM chip
		ee_setpage(page++);

		// enable async transmit
		//TODO: Rewire the callback function...
		//_uart->beginTransmit(&Sermem::getFileCallback);

		// send first byte to start async transmit routine
		_uart->write(ee_read());
		_bytesTransfered++;

		// wait for page to finish transmitting
		while (!_transferPageComplete)
		{}
		_transferPageComplete = 0;

		// get ACK code from client
		if (TRANSFER_ACK != _uart->read())
			break;
	}

	// make sure async mode is disabled
	_uart->endTransmit();

	// send final NACK
	_uart->write(TRANSFER_NACK);

	// return success
	return TRANSFER_SUCCESS;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// formats the chip by writing 0xff to all cells
void Sermem::format(void)
{
	uint16_t length = 0;
	uint16_t page = 0;
	while (page < AT24C1024_PAGE_COUNT)
	{
		ee_setpage(page++);

		while (length++ < AT24C1024_PAGE_SIZE)
			ee_putByte(0xFF);
		length = 0;

		ee_putBytesEnd();

		putstr(PSTR("."));
	}
	putstr(PSTR("\r\n"));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sermem::putstr(const char * pstr)
{
	if (_autoMode)
		return;
	_uart->putstrAM(pstr, 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sermem::showHelp(void)
{
	putstr(PSTR("Sound Effects EEPROM Commands:\r\n\
  A: Auto-mode for scripted interfacing\r\n\
  M: Manual-mode for non-scripted interfacing\r\n\
  R: Retrieves the entire contents of the EEPROM\r\n\
  W: Stores a file on the EEPROM\r\n\
  F: Formats the EEPROM\r\n\
  I: Interactive mode\r\n\
  H: Display help (this text)\r\n"));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Processes serial data
void Sermem::process(char data)
{
	switch (data & 0x5F) // mask all characters to upper-case ASCII :)
	{
		// auto-mode
		case 'A':
			_uart->write(TRANSFER_ACK);
			_autoMode = 1;
			break;

		// manual mode
		case 'M':
			_autoMode = 0;
			putstr(PSTR("Manual mode now selected.\r\n"));
			break;

		// retrieve EEPROM contents
		case 'R':
			putstr(PSTR("Transfer starting...\r\n"));
			getFile();
			putstr(PSTR("Done!\r\n"));
			break;

		// write new EEPROM data
		case 'W':
			putstr(PSTR("Waiting for file transfer.\r\n"));
			if (putFile())
				putstr(PSTR("File successfully transfered to EEPROM.\r\n"));
			else
				putstr(PSTR("File transfer timed out.  Please send file within 30 seconds.\r\n"));
			break;

		// format the EEPROM
		case 'F':
			putstr(PSTR("Formatting...\r\n"));
			format();
			putstr(PSTR("Done!\r\n"));
			break;
		
		// show the help text
		case 'H':
			showHelp();
			break;
	}
}

