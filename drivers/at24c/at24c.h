#ifndef AT24C_H
#define AT24C_H

#include "../../twi/i2c.h"


#ifndef TWI_SPEED
	#warning TWI_SPEED not defined.  Defaulting to 100kHz
	#define TWI_SPEED			100
#endif


#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// Setup AT24C1024B info
#define AT24C1024_ADDRESS		0b10100000
#define AT24C1024_DEVICE_PAGES	512
#define AT24C1024_DEVICE_SIZE	131072
#define AT24C1024_PAGE_SIZE		256
#define AT24C1024_DEVICE_COUNT	2
#define AT24C1024_PAGE_COUNT	AT24C1024_DEVICE_PAGES * AT24C1024_DEVICE_COUNT
#define AT24C1024_MAX_DATA		AT24C1024_DEVICE_SIZE * AT24C1024_DEVICE_COUNT

// async enumerations
#define ASYNC_COMPLETE			0x00
#define ASYNC_IDLE				0xFF

// send enum values
#define ASYNC_SEND_START		1
#define ASYNC_SEND_DEVICE		2
#define ASYNC_SEND_ADDRMSB		3
#define ASYNC_SEND_ADDRLSB		4
#define ASYNC_SEND_STOP			5

// read next enum values
#define ASYNC_NEXT_START		1
#define ASYNC_NEXT_DEVICE		2
#define ASYNC_NEXT_NACK			3
#define ASYNC_NEXT_READ			4
#define ASYNC_NEXT_STOP			5

#define ASYNC_MULTI_START		1
#define ASYNC_MULTI_DEVICE		2
#define ASYNC_MULTI_ADDRMSB		3
#define ASYNC_MULTI_ADDRLSB		4
#define ASYNC_MULTI_READ		5
#define ASYNC_MULTI_NEXT		6
#define ASYNC_MULTI_STOP		7


typedef	uint8_t EE_STATUS;


//---------------------------------------------------------------------------


void ee_mapdevice(uint16_t page);

EE_STATUS ee_setpage(uint16_t page);
void ee_setpageA(uint16_t page, fStatusCallback callBack);

uint8_t ee_read(void);
void ee_readEnd(void);
void ee_readA(fStatusCallback callBack);

EE_STATUS ee_readBytes(uint16_t page, uint16_t length, uint8_t * data);
void ee_readBytesA(uint16_t page, uint16_t length, uint8_t * data, fStatusCallback callBack);

EE_STATUS ee_writeBytes(uint16_t page, uint8_t * data);

EE_STATUS ee_putByteStart(uint16_t page);
void ee_putByte(uint8_t data);
void ee_poll(void);
void ee_putBytesEnd(void);

void ee_init(unsigned short bitrateKHz);



#ifdef  __cplusplus
}
#endif

#endif
