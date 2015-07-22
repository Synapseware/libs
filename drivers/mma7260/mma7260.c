#include "mma7260.h"



volatile static float		_samples[3];
volatile static uint8_t		_latest[3];
volatile static uint8_t		_lastChannel;



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// maps a sample to a table computed value
void mapSampleToTableIndex(uint8_t sample, float* angle)
{
	// constrain sample value
	if (sample < AccelLookupTableMinSample)
		sample = AccelLookupTableMinSample;
	else if (sample > AccelLookupTableMaxSample)
		sample = AccelLookupTableMaxSample;

	// convert sample value to an index
	sample -= AccelLookupTableMinSample;

	// fill the entry structure with data
	*angle = pgm_read_float(&AccelLookupTable[sample].arcsine);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Returns the acceleration of the device as a set of angles
void accel_readAngles(float * ax, float * ay, float * az)
{
	*ax = _samples[0];
	*ay = _samples[1];
	*az = _samples[2];
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// returns a delta between this request and the last request
void accel_readDeltas(float * px, float * py, float * pz)
{
	static float lx, ly, lz;
	float x, y, z;

	accel_readAngles(&x, &y, &z);

	// get the delta between this request and the last request
	*px = x - lx;
	*py = y - ly;
	*pz = z - lz;

	// save the sampled values
	lx = x;
	ly = y;
	lz = z;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Simply starts a conversion
void nextSampleHandler(eventState_t state)
{
	// enable ADC and start a conversion
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// initialize the accelerometer
void initAccelerometer(eventState_t state)
{
	// set the wake bit
	ACCEL_CTL_PORT |= (1<<ACCEL_CTL_SLEEP);

	// mark pins as output (wakes up the device & sets the gain value)
	DDRD |= (1<<ACCEL_CTL_SLEEP) | (1<<ACCEL_CTL_GAIN1) | (1<<ACCEL_CTL_GAIN2);	

	// set the gain modes
#if (ACCEL_GAIN_MODE == G1_5)
	ACCEL_CTL_PORT &= ~((1<<ACCEL_CTL_GAIN1) | (1<<ACCEL_CTL_GAIN2));
#elif (ACCEL_GAIN_MODE == G2)
	ACCEL_CTL_PORT |= (1<<ACCEL_CTL_GAIN1);
	ACCEL_CTL_PORT &= ~(1<<ACCEL_CTL_GAIN2);
#elif (ACCEL_GAIN_MODE == G4)
	ACCEL_CTL_PORT &= ~(1<<ACCEL_CTL_GAIN1);
	ACCEL_CTL_PORT |= (1<<ACCEL_CTL_GAIN2);
#elif (ACCEL_GAIN_MODE == G6)
	ACCEL_CTL_PORT |= (1<<ACCEL_CTL_GAIN1) | (1<<ACCEL_CTL_GAIN2);
#else
	#error ACCEL_GAIN_MODE is invalid.  Must be G1_5, G2, G4 or G6
#endif

	// clear any pending interrupt flag
	ADCSRA |= (1<<ADIF);

	// setup to sample accelerometer data @ 48Hz (48/3 channels is 16 samples a channel, per second)
	uint16_t timeBase = getTimeBase();
	//registerEvent(nextSampleHandler, (timeBase/16/3), 0);

	// let's go with a slower sample model of 10 samples/second
	// quick analysis has revealed that the chip can only do 10 conversions
	// a second anyway.
	registerEvent(nextSampleHandler, (timeBase/10), 0);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// initialize the system
void accel_init(void)
{
	// make sure the ADC power is enabled
	power_adc_enable();

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega88__)
	// setup MUX
	ADMUX =		(0<<REFS1) |	// AVCC with external capacitor at AREF pin
				(1<<REFS0) |
				(1<<ADLAR);		// Left-adjust result

	// setup ADCSRB
	ADCSRB =	(0<<ACME) |
				(0<<ADTS2) |	// free-running mode
				(0<<ADTS1) |
				(0<<ADTS0);

#elif defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__)
	// setup MUX
	ADMUX =		(0<<REFS1) |	// AVCC with external capacitor at AREF pin
				(1<<REFS0);

	// setup ADCSRB
	ADCSRB =	(0<<BIN) |
				(0<<ACME) |
				(1<<ADLAR) |	// Left-adjust result
				(0<<ADTS2) |	// free-running mode
				(0<<ADTS1) |
				(0<<ADTS0);
#endif
	// setup ADCSRA
	ADCSRA =	(1<<ADEN) |		// enable the ADC
				(0<<ADSC) |		// don't start a conversion
				(0<<ADATE) |	// no auto trigger
				(1<<ADIF) |		// clear interrupt flag
				(1<<ADIE) |		// enable interrupts
				(1<<ADPS2) |	// prescaler set @ 128 (14.7456mHz/128 = 115,200samples/sec)
				(1<<ADPS1) |	// @ 25 clock cycles per conversion (we change channels on every conversion)
				(0<<ADPS0);		// the ADC is able to get a new sample every 

	// disable digital inputs on ADC ports
	DIDR0 =		(1<<ACCEL_AXIS_X_PORT) |
				(1<<ACCEL_AXIS_Y_PORT) |
				(1<<ACCEL_AXIS_Z_PORT);

	_lastChannel = 0;
	_samples[0] = 0;
	_samples[1] = 0;
	_samples[2] = 0;

	uint16_t timeBase = getTimeBase();

	// initialize the accelerometer after 500ms
	registerOneShot(initAccelerometer, timeBase / 2, 0);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Conversion complete interrupt handler
ISR(ADC_vect)
{
	// get sample value
	uint8_t sample = ADCH;

	// disable the ADC
	ADCSRA &= ~(1<<ADEN);

	// convert raw sample value to signed 8 bit int
	_latest[_lastChannel] = sample;

	// setup the next channel
	_lastChannel++;
	if (_lastChannel > 2)
	{
		// copy latest sample data
		mapSampleToTableIndex(_latest[0] + ZERO_X, &_samples[0]);
		mapSampleToTableIndex(_latest[1] + ZERO_Y, &_samples[1]);
		mapSampleToTableIndex(_latest[2] + ZERO_Z, &_samples[2]);

		// reset the channel
		_lastChannel = 0;
	}

	// change channels
	switch (_lastChannel)
	{
		case 0:
			ADMUX_PORT = (ADMUX_PORT & ~ADMUX_MASK) | ACCEL_AXIS_X_ADC;
			break;
		case 1:
			ADMUX_PORT = (ADMUX_PORT & ~ADMUX_MASK) | ACCEL_AXIS_Y_ADC;
			break;
		case 2:
			ADMUX_PORT = (ADMUX_PORT & ~ADMUX_MASK) | ACCEL_AXIS_Z_ADC;
			break;
	}
}
