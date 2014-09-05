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
const uint8_t VREF_AVCC				= 0x00;
const uint8_t VREF_AREF				= 0x01;
const uint8_t VREF_11				= 0x03;

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
