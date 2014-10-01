#ifndef _ADC_H_
#define _ADC_H_


#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

#define SLEEP_FOR_ADC
#define SAMPLES_TO_AVERAGE          8
#define ADC_SAMPLE_MAX				1024

// define ADC analog reference constants
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__)
#  include "adcutils_m168.h"
#elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
#  include "adcutils_tx5.h"
#endif


class Adc
{
public:
	
	// Default constructor
	Adc(void);
	
	// Sets the ADC's channel and VREF source
	void setAdcChannelAndVRef(uint8_t channel, uint8_t vref);
	
	// Reads the ADC value for the specified channel
	uint16_t readAdcSample(void);
	
	// Reads a fixed number of samples from the ADC and averages the result
	uint16_t getAdcAverage(uint8_t samples);

	// helper to rescale a value	
	double scaleAdcReading(uint16_t value, double scale);

	// Prepare the ADC for sampling
	void init(void);

	// should be called by the ADC conversion complete ISR	
	void conversionComplete(uint16_t value);

private:

	uint16_t _lastAdcValue;
};


#endif
