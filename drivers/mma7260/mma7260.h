#ifndef _MMA_7260_H
#define _MMA_7260_H


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <math.h>

#include "../../utils.h"
#include "../../events/events.h"



#ifndef ACCEL_AXIS_X_PORT
	#error ACCEL_AXIS_X_PORT undefined
#endif
#ifndef ACCEL_AXIS_Y_PORT
	#error ACCEL_AXIS_Y_PORT undefined
#endif
#ifndef ACCEL_AXIS_Z_PORT
	#error ACCEL_AXIS_Z_PORT undefined
#endif
#ifndef ACCEL_AXIS_X_ADC
	#error ACCEL_AXIS_X_ADC undefined
#endif
#ifndef ACCEL_AXIS_Y_ADC
	#error ACCEL_AXIS_Y_ADC undefined
#endif
#ifndef ACCEL_AXIS_Z_ADC
	#error ACCEL_AXIS_Z_ADC undefined
#endif
#ifndef ACCEL_CTL_DDR
	#error ACCEL_CTL_DDR undefined
#endif
#ifndef ACCEL_CTL_PORT
	#error ACCEL_CTL_PORT undefined
#endif
#ifndef ACCEL_CTL_GAIN1
	#error ACCEL_CTL_GAIN1 undefined
#endif
#ifndef ACCEL_CTL_GAIN2
	#error ACCEL_CTL_GAIN2 undefined
#endif
#ifndef ACCEL_CTL_SLEEP
	#error ACCEL_CTL_SLEEP undefined
#endif
#ifndef ACCEL_GAIN_MODE
	#error ACCEL_GAIN_MODE undefined
#endif



#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega48__)
	#define ADMUX_PORT		ADMUX
	#define AD_PORT			PORTC
	#define AD_DDR			DDRC
	const static uint8_t	ADMUX_MASK =
		(1<<MUX3) |
		(1<<MUX2) |
		(1<<MUX1) |
		(1<<MUX0);
#elif defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__)
	#error Not yet supported
	const static uint8_t	ADMUX_MASK =
		(1<<ADMUX5) |
		(1<<ADMUX4) |
		(1<<ADMUX3) |
		(1<<ADMUX2) |
		(1<<ADMUX1) |
		(1<<ADMUX0);
#else
	#error Unsupported MCU
#endif




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Zero offsets required to zero the outputs
#define ZERO_X		4
#define ZERO_Y		1
#define ZERO_Z		4



// Returns the acceleration of the device as a set of angles
void accel_readAngles(float * ax, float * ay, float * az);

// Returns the delta's in the raw data for the current sample and the previous
// delta request.
void accel_readDeltas(float * px, float * py, float * pz);

// Takes a pointer a configuration struct, which is expected to
// reside in PROGMEM!
void accel_init(void);



const static uint8_t AccelLookupTableMinSample = 65;
const static uint8_t AccelLookupTableMaxSample = 191;


typedef struct
{
	uint8_t			adc;
	float			arcsine;
} PROGMEM const LookupTableEntry_t;

// define the lookup table for the accelerometer
const static LookupTableEntry_t AccelLookupTable[] PROGMEM =
{
	{65, -90.00},
	{66, -87.47},
	{67, -79.39},
	{68, -75.19},
	{69, -71.93},
	{70, -69.16},
	{71, -66.7},
	{72, -64.47},
	{73, -62.4},
	{74, -60.47},
	{75, -58.65},
	{76, -56.92},
	{77, -55.26},
	{78, -53.67},
	{79, -52.14},
	{80, -50.66},
	{81, -49.23},
	{82, -47.83},
	{83, -46.48},
	{84, -45.15},
	{85, -43.86},
	{86, -42.59},
	{87, -41.35},
	{88, -40.13},
	{89, -38.93},
	{90, -37.76},
	{91, -36.6},
	{92, -35.46},
	{93, -34.33},
	{94, -33.22},
	{95, -32.12},
	{96, -31.04},
	{97, -29.97},
	{98, -28.91},
	{99, -27.86},
	{100, -26.82},
	{101, -25.79},
	{102, -24.77},
	{103, -23.76},
	{104, -22.75},
	{105, -21.75},
	{106, -20.76},
	{107, -19.78},
	{108, -18.8},
	{109, -17.83},
	{110, -16.86},
	{111, -15.9},
	{112, -14.94},
	{113, -13.99},
	{114, -13.04},
	{115, -12.09},
	{116, -11.15},
	{117, -10.21},
	{118, -9.27},
	{119, -8.34},
	{120, -7.41},
	{121, -6.48},
	{122, -5.55},
	{123, -4.62},
	{124, -3.7},
	{125, -2.77},
	{126, -1.85},
	{127, -0.92},
	{128, 0},
	{129, 0.92},
	{130, 1.85},
	{131, 2.77},
	{132, 3.7},
	{133, 4.62},
	{134, 5.55},
	{135, 6.48},
	{136, 7.41},
	{137, 8.34},
	{138, 9.27},
	{139, 10.21},
	{140, 11.15},
	{141, 12.09},
	{142, 13.04},
	{143, 13.99},
	{144, 14.94},
	{145, 15.9},
	{146, 16.86},
	{147, 17.83},
	{148, 18.8},
	{149, 19.78},
	{150, 20.76},
	{151, 21.75},
	{152, 22.75},
	{153, 23.76},
	{154, 24.77},
	{155, 25.79},
	{156, 26.82},
	{157, 27.86},
	{158, 28.91},
	{159, 29.97},
	{160, 31.04},
	{161, 32.12},
	{162, 33.22},
	{163, 34.33},
	{164, 35.46},
	{165, 36.6},
	{166, 37.76},
	{167, 38.93},
	{168, 40.13},
	{169, 41.35},
	{170, 42.59},
	{171, 43.86},
	{172, 45.15},
	{173, 46.48},
	{174, 47.83},
	{175, 49.23},
	{176, 50.66},
	{177, 52.14},
	{178, 53.67},
	{179, 55.26},
	{180, 56.92},
	{181, 58.65},
	{182, 60.47},
	{183, 62.4},
	{184, 64.47},
	{185, 66.7},
	{186, 69.16},
	{187, 71.93},
	{188, 75.19},
	{189, 79.39},
	{190, 87.47},
	{191, 90.00}
};

static const uint16_t AccelTableLen = sizeof(AccelLookupTable)/sizeof(LookupTableEntry_t);


#endif
