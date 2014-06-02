#include "ads7818.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initializes the ADC
void ADS7818::initialize(void)
{

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// reads a sample value from the ADC
uint16_t ADS7818::read(void)
{
	cli();

	// bring conversion line low
	

	uint8_t clk = 12;
	while (--clk)
	{

	}

	sei();
}
