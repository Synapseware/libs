#ifndef __RGB_H__
#define __RGB_H__


#include <avr/wdt.h>
#include <stdint.h>
#include <math.h>

#include <events/events.h>


#define MONO_FADE_STEPS	64


class Mono
{
public:

	// Monochrome pixel
	struct dot_t {
	    uint8_t		intensity;
	};


	void initialize(dot_t* ioPixel);
	void fadeIn(uint8_t intensity, const uint16_t delay);
	void fadeOut(const uint16_t delay);
	void fadeTo(uint8_t intensity, const uint16_t delay);

private:

	// Pixel for doing effects
	struct factors_t {
	    float		delta;
	};

	// fade effects callback event handler
	static void e_fadeCallback(eventState_t state)
	{
		if (0 == state)
			return;

		Mono* mono = (Mono*)state;
		mono->e_fade();
	}

	// performs the fade effects for an individual fade event
	void e_fade(void);
	void fade(dot_t * fadeFrom, dot_t * fadeTo, const uint16_t delay);

	// computes the fade steps needed to transition from one color to another
	factors_t getFadeFactor(dot_t * fadeFrom, dot_t * fadeTo);

	// private object data
	factors_t			_effectsDot;
	factors_t			_fadeFactor;
	dot_t*				_ioDot;
	uint8_t				_fadeSteps;
};



#endif
