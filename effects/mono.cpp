#include "rgb.h"




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initialize the RGB Effects with required data values
void Mono::initialize(dot_t* dot)
{
	_ioDot = dot;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Calculates the fading step that each color needs to make
Mono::factors_t Mono::getFadeFactor(dot_t * fadeFrom, dot_t * fadeTo)
{
	factors_t factor;

	factor.delta = ((float)(to - from)) / MONO_FADE_STEPS;
	return factor;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Performs the fade in steps
void Mono::e_fade(void)
{
	if (_fadeSteps == 0)
		return;

	_effectsDot.intensity += _fadeFactors.delta;

	// set the RGB values on the IO pixel
	_ioDot->intensity = _effectsDot.intensity;

	if (_fadeSteps > 0)
		_fadeSteps--;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from current pixel to specified pixel at the specified event rate
void Mono::fade(dot_t * fadeFrom, dot_t * fadeTo, const uint16_t delay)
{
	// compute the fade or blend steps for each color
	_fadeFactors = getFadeFactor(fadeFrom, fadeTo);

	// the delay value is the delay needed to complete the animation.  Based on the fade step constant,
	// determine the step delay.
	uint16_t stepDelay = delay / MONO_FADE_STEPS;

	// set the fade step counter
	_fadeSteps = MONO_FADE_STEPS;

	// copy current IO pixel
	_effectsDot.delta = _ioDot->intensity;

	// wait for fade to complete
	registerEvent(e_fadeCallback, stepDelay, this);
	while (_fadeSteps > 0)
	{
		wdt_reset();
		eventsDoEvents();
	}
	eventsUnregisterEvent(e_fadeCallback);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from black to the specified pixel
void Mono::fadeIn(uint8_t intensity, const uint16_t step)
{
	dot_t fadeFrom;
	fadeFrom.intensity = 0;

	dot_t fadeTo;
	fadeTo.intensity = intensity;

	fade(&fadeFrom, &fadeTo, step);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from current color to specified color
void Mono::fadeTo(uint8_t intensity, const uint16_t delay)
{
	dot_t fadeTo;
	fadeTo.intensity = intensity;

	fade(_ioDot, &fadeTo, delay);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from current color to black
void Mono::fadeOut(const uint16_t delay)
{
	fadeTo(0, delay);
}
