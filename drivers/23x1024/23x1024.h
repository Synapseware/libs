#ifndef __23x1024_H__
#define __23x1024_H__




#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


// operational modes
const uint8_t	MODE_BYTE		= (0<<6);
const uint8_t	MODE_PAGE		= (2<<6);
const uint8_t	MODE_SEQ		= (1<<6);

// commands
const uint8_t	CMD_READ		= 0x03;
const uint8_t	CMD_WRITE		= 0x02;
const uint8_t	CMD_EDIO		= 0x3B;
const uint8_t	CMD_EQIO		= 0x38;
const uint8_t	CMD_RSTIO		= 0xFF;
const uint8_t	CMD_RDMR		= 0x05;
const uint8_t	CMD_WRMR		= 0x01;


class SerialSRam
{
public:
	SerialSRam(void);

	void SetAddress(long address);

	int Read(void);
	int Read(int count, void* data);
	int Read(int count, long address, void* data);

	int Write(uint8_t data);
	int Write(int count, void* data);
	int Write(int count, long address, void* data);

	void SetByteMode(void);
	void SetPageMode(void);
	void SetSequentialMode(void);
private:
	uint8_t		_mode			= 0;
	long		_address		= 0;

	inline void write(uint8_t byte)
	{

	}

	// sets the operational mode 9byte/page/sequential)
	void SetMode(uint8_t mode)
	{
		mode 
	}
};






#endif