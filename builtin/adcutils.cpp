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
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__)
	// 0x30 = 0b00110000, we don't want to keep the REFS0:1 or MUX3:0 bits
	ADMUX = (ADMUX & 0x30) |
			(channel & 0x0F) |
			((vref & 0x03) << 6);

	// set DIRD0 here!!
#elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
	ADMUX = (ADMUX & 0x20) |			// clear REFS2:0
			(channel & 0x0F) |			// set the channel #
			((vref & 0x04) << 4) |		// set REFS2
			((vref & 0x03) << 6);		// set REFS1:0

	//attinyx5 has crazy stupid digital input disable mappings...  :(
	if (channel >= 0x00 && channel <= 0x03)
	{
		uint8_t didr = 0;
		switch (channel)
		{
			case 0x00:
				didr = (1<<ADC0D);
				break;
			case 0x01:
				didr = (1<<ADC1D);
				break;
			case 0x02:
				didr = (1<<ADC2D);
				break;
			case 0x03:
				didr = (1<<ADC3D);
				break;
		}

		// preserve the non-ADC channel pin values
		DIDR0 = (DIDR0 & 0x03) |
				didr;

		// just mark the requested channel as input
		DDRB |= didr;
	}
#endif
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
// Initialize the ADC to use specific channels
void Adc::init(void)
{
	power_adc_enable();

	sleep_enable();
	set_sleep_mode(SLEEP_MODE_ADC);

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__)
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
#elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
	ADMUX	=	(0<<REFS1)	|
				(0<<REFS0)	|
				(0<<ADLAR)	|
				(0<<REFS2)	|
				(0<<MUX3)	|
				(0<<MUX2)	|
				(0<<MUX1)	|
				(0<<MUX0);

	ADCSRA	=	(1<<ADEN)	|
				(1<<ADSC)	|
				(0<<ADATE)	|
				(1<<ADIF)	|
				(1<<ADIE)	|
				(1<<ADPS2)	|	// set prescaler to 128 (16MHz/128 = 125KHz ADC clock)
				(1<<ADPS1)	|
				(1<<ADPS0);

	ADCSRB	=	(0<<BIN)	|
				(0<<IPR)	|
				(0<<ADTS2)	|
				(0<<ADTS1)	|
				(0<<ADTS0);
#endif
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
