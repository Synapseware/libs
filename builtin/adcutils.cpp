#include "adcutils.h"


// --------------------------------------------------------------------------------
Adc::Adc(void)
{
	// initialize the class
	_lastAdcValue = 0;
}

// --------------------------------------------------------------------------------
void Adc::setAdcChannelAndVRef(uint8_t channel, uint8_t vref)
{
	// 0x30 = 0b00110000, we don't want to keep the REFS0:1 or MUX3:0 bits
	ADMUX = (ADMUX & 0x30) |
			(channel & 0x0F) |
			((vref & 0x03) << 6);

	// read the ADC (discards the result after a channel & vref change)
	readAdcSample();
}

// --------------------------------------------------------------------------------
uint16_t Adc::readAdcSample(void)
{
	sleep_cpu(); // ISR will wake up CPU
	return _lastAdcValue;
}

// --------------------------------------------------------------------------------
uint16_t Adc::getAdcAverage(uint8_t samples)
{
	uint16_t	total		= 0;
	uint8_t		count		= samples;

	while (count > 0)
	{
		total += readAdcSample();
		count--;
	}

	total = total / samples;

	return total;
}

// --------------------------------------------------------------------------------
void Adc::init(void)
{
	power_adc_enable();

	sleep_enable();
	set_sleep_mode(SLEEP_MODE_ADC);

	// atmega168...

	// prepare ADC
	ADMUX =	(1<<REFS1) |
			(1<<REFS0) |
			(0<<ADLAR);		// right adjust result

	// setup
	ADCSRA = (1<<ADEN) |
			(1<<ADSC) |
			(0<<ADATE) |
			(1<<ADIF) |
			(1<<ADIE) |
			(1<<ADPS2) |	// prescale should be ~160 (14.75MHz/125KHz = 118KHz)
			(0<<ADPS1) |
			(1<<ADPS0);

	// setup
	ADCSRB = (0<<ACME) |
			(0<<ADTS2) |
			(0<<ADTS1) |
			(0<<ADTS0);

	// disable digial inputs
	DIDR0 &= ~((1<<ADC1D) | (1<<ADC0D));

	// setup the input channels
	DDRC &= ~((1<<ADC0D) | (1<<ADC1D));
}


// --------------------------------------------------------------------------------
double Adc::scaleAdcReading(uint16_t value, double scale)
{
	
	return value * scale / ADC_SAMPLE_MAX;
}

// --------------------------------------------------------------------------------
void Adc::conversionComplete(uint16_t value)
{
	// save the provided value
	_lastAdcValue = value; 
}
