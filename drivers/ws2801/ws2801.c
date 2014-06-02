#include "ws2801.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initialize RGB
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RGBInit(void)
{
	// enable the SPI interface
	SPI_MasterInit();

	// set RGB output pin	
	RGB_DDR	|= (1<<RGB_DRVR_POL);
	RGB_PORT &= ~(1<<RGB_DRVR_POL);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Updates the RGB LED with the latest color value for the specified SOC value
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SetSOCColor(BYTE soc)
{
	BYTE red		= pgm_read_byte(&COLOR_MAP[(soc * 3) + 0]);
	BYTE green		= pgm_read_byte(&COLOR_MAP[(soc * 3) + 1]);
	BYTE blue		= pgm_read_byte(&COLOR_MAP[(soc * 3) + 2]);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		SPI_MasterTransmit(red);
		SPI_MasterTransmit(green);
		SPI_MasterTransmit(blue);
	}
}
