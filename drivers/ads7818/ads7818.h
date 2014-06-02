#ifndef __ADS7818_H__
#define __ADS7818_H__


#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

class ADS7818
{
public:
	void initialize(void);


	uint16_t read(void);


private:
};


#endif
