#include "rgb.h"




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initialize the RGB Effects with required data values
RGB::RGB(Events* events, pixel_t* pixel)
{
	_events = events;
	_ioPixel = pixel;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Calculates the fading step that each color needs to make
void RGB::calcFade(uint8_t cfrom, uint8_t cto, float* val)
{
	// compute floating point difference
	*val = ((float)(cto - cfrom)) / RGB_FADE_STEPS;
}
RGB::factors_t RGB::getFadeFactor(pixel_t * fadeFrom, pixel_t * fadeTo)
{
	factors_t factor;

	calcFade(fadeFrom->red, fadeTo->red, &factor.red);
	calcFade(fadeFrom->grn, fadeTo->grn, &factor.grn);
	calcFade(fadeFrom->blu, fadeTo->blu, &factor.blu);

	return factor;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Performs the fade in steps
void RGB::e_fade(void)
{
	if (_fadeSteps == 0)
		return;

	_effectsPixel.red += _fadeFactors.red;
	_effectsPixel.grn += _fadeFactors.grn;
	_effectsPixel.blu += _fadeFactors.blu;

	// set the RGB values on the IO pixel
	_ioPixel->red = _effectsPixel.red;
	_ioPixel->grn = _effectsPixel.grn;
	_ioPixel->blu = _effectsPixel.blu;

	if (_fadeSteps > 0)
		_fadeSteps--;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from current pixel to specified pixel at the specified event rate
void RGB::fade(pixel_t * fadeFrom, pixel_t * fadeTo, const uint16_t delay)
{
	// compute the fade or blend steps for each color
	_fadeFactors = getFadeFactor(fadeFrom, fadeTo);

	// the delay value is the delay needed to complete the animation.  Based on the fade step constant,
	// determine the step delay.
	uint16_t stepDelay = delay / RGB_FADE_STEPS;

	// set the fade step counter
	_fadeSteps = RGB_FADE_STEPS;

	// copy current IO pixel
	_effectsPixel.red = _ioPixel->red;
	_effectsPixel.grn = _ioPixel->grn;
	_effectsPixel.blu = _ioPixel->blu;

	// wait for fade to complete
	_events->registerEvent(e_fadeCallback, stepDelay, this);
	while (_fadeSteps > 0)
	{
		wdt_reset();
		_events->doEvents();
	}
	_events->eventsUnregisterEvent(e_fadeCallback);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from black to the specified pixel
void RGB::fadeIn(uint8_t red, uint8_t green, uint8_t blue, const uint16_t step)
{
	pixel_t fadeFrom;
	fadeFrom.red = 0;
	fadeFrom.grn = 0;
	fadeFrom.blu = 0;

	pixel_t fadeTo;
	fadeTo.red = red;
	fadeTo.grn = green;
	fadeTo.blu = blue;

	fade(&fadeFrom, &fadeTo, step);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from current color to specified color
void RGB::fadeTo(uint8_t red, uint8_t green, uint8_t blue, const uint16_t delay)
{
	pixel_t fadeTo;
	fadeTo.red = red;
	fadeTo.grn = green;
	fadeTo.blu = blue;

	fade(_ioPixel, &fadeTo, delay);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Fade from current color to black
void RGB::fadeOut(const uint16_t delay)
{
	fadeTo(0, 0, 0, delay);
}
