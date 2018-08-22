#include "23x1024.h"


//----------------------------------------------------------
// Constructor
SerialSRam::SerialSRam(void)
{
	// set a default mode (sequential read)
	_mode = MODE_SEQ | CMD_READ;
}


//----------------------------------------------------------
void SerialSRam::SetByteMode(void)
{}


//----------------------------------------------------------
void SerialSRam::SetPageMode(void)
{}


//----------------------------------------------------------
void SerialSRam::SetSequentialMode(void)
{}


//----------------------------------------------------------
void SerialSRam::SetAddress(long address)
{
	_address = address & 0x000001FFFF;
}


//----------------------------------------------------------
int SerialSRam::Read(void)
{
	return -1;
}


//----------------------------------------------------------
int SerialSRam::Read(int count, void* data)
{
	return -1;
}


//----------------------------------------------------------
int SerialSRam::Read(int count, long address, void* data)
{
	SetAddress(address);
	return -1;
}


//----------------------------------------------------------
int SerialSRam::Write(uint8_t data)
{
	return -1;
}


//----------------------------------------------------------
int SerialSRam::Write(int count, void* data)
{
	return -1;
}


//----------------------------------------------------------
int SerialSRam::Write(int count, long address, void* data)
{
	return -1;
}

