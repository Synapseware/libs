#ifndef __RGB_H__
#define __RGB_H__


#include <avr/wdt.h>
#include <stdint.h>
#include <math.h>

#include <events/events.h>


#define RGB_FADE_STEPS	64


class RGB
{
public:
	// RGB pixel
	struct pixel_t {
	    uint8_t		red;
	    uint8_t		grn;
	    uint8_t		blu;
	};


	RGB(Events* events, pixel_t* ioPixel);

	void fadeIn(uint8_t red, uint8_t green, uint8_t blue, const uint16_t delay);
	void fadeOut(const uint16_t delay);
	void fadeTo(uint8_t red, uint8_t green, uint8_t blue, const uint16_t delay);

private:

	// Pixel for doing effects
	struct factors_t {
	    float		red;
	    float		grn;
	    float		blu;
	};

	// fade effects callback event handler
	static void e_fadeCallback(eventState_t state)
	{
		if (0 == state)
			return;

		RGB* rgb = (RGB*)state;
		rgb->e_fade();
	}

	// performs the fade effects for an individual fade event
	void e_fade(void);
	void fade(pixel_t * fadeFrom, pixel_t * fadeTo, const uint16_t delay);

	// computes the fade steps needed to transition from one color to another
	factors_t getFadeFactor(pixel_t * fadeFrom, pixel_t * fadeTo);
	void calcFade(uint8_t cfrom, uint8_t cto, float* val);

	// computes the minimum number of steps needed to complete the fade
	uint8_t getMinimumSteps(factors_t* effects);

	// private object data
	Events*				_events;
	factors_t			_effectsPixel;
	factors_t			_fadeFactors;
	pixel_t*			_ioPixel;
	uint8_t				_fadeSteps;
};



#endif
