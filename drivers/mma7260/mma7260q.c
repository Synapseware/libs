#include "mma7260q.h"


// initialize global vars for motion library
MOTION_DATA motion_data[ACCEL_CHANNELS];
unsigned char adc_channel = 0;


//------------------------------------------------------------------------------------------
// initialize the motion sensor hardware
//------------------------------------------------------------------------------------------
void init_motion(void)
{
	// enable ADC
	PRR &= ~(1<<PRADC);

	// setup ADC...
	// enable ADC and interrupts
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADSC);

	// set ADC clock to be CLK/128 (20000000/128 = 156250Hz or 156kHz)
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

	// auto trigger for timer0 overflow (76Hz)
	ADCSRA |= (1<<ADATE);

	// we'll use external Vref, left-adjust
	ADMUX = ADC_MUX_BITS;

	// default sample port
	ADC_enable_axis(ACCEL_X);

	// set for auto trigger on timer0 overflow
	ADCSRB = (1<<ADTS1) | (1<<ADTS0);

	// disable digital input on PORTC0/ADC0, adc1 and adc2
	DIDR0 |= (1<<ACCEL_X) | (1<<ACCEL_Y) | (1<<ACCEL_Z);
}


//------------------------------------------------------------------------------------------
// ADC interrupt handler.  Read ADC registers and set delay
// routine will switch between ports, so for each interrupt we'll sample 1 axis value
//------------------------------------------------------------------------------------------
volatile unsigned char _discard = 1;
ISR(ADC_vect)
{
	if (_discard)
		_discard = 0;
	else
	{
		// get new sample value
		unsigned char sample = ADCH;

		// set our sample value and update the moving average
		updateMotionAverage(sample, &motion_data[adc_channel]);

		// move to next axis
		adc_channel++;
		if (adc_channel >= ACCEL_CHANNELS)
			adc_channel = 0;

		// change ADC channel
		switch(adc_channel)
		{
			case 0:
				ADMUX = ADC_MUX_BITS | ACCEL_X;
				break;
			case 1:
				ADMUX = ADC_MUX_BITS | ACCEL_Y;
				break;
			case 2:
				ADMUX = ADC_MUX_BITS | ACCEL_Z;
				break;
		}

		_discard = 1;
	}
}


//------------------------------------------------------------------------------------------
// Updates the motion data with a new sample and a new average
//------------------------------------------------------------------------------------------
void updateMotionAverage(unsigned char sample, MOTION_DATA* data)
{
	if (!data)
		return;

	// set new sample value and update index
	data->values[data->index++] = sample;
	data->index = data->index % MOTION_BUFFER_LEN;

	// sum sample values
	unsigned int average = 0;
	unsigned char i;
	for (i = 0; i < MOTION_BUFFER_LEN; i++)
		average += data->values[i];

	// compute new average
	data->average = average / MOTION_BUFFER_LEN;
}
