/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Button debounce utility functions.

Uses a polling method with timeouts to process button debounce
actions and invoke call-backs 
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/


#ifndef __BUTTON_H__
#define __BUTTON_H__



#include <inttypes.h>



//------------------------------------------------------------------------------------------
// button debounce delay
#define BUTTON_TIMEOUT			250


// button states
#define STATE_PROCESSED			0
#define STATE_DEBOUNCE			1
#define STATE_TIMEOUT			2
#define STATE_RELEASED			3


typedef void (*fButtonOnClick)(void);
typedef void (*fButtonOnRelease)(void);
typedef void (*fButtonComplete)(void);
typedef uint8_t (*fButtonIsUp)(void);

//------------------------------------------------------------------------------------------
// Processes button press events
class ButtonClickDebounce
{
public:
	ButtonClickDebounce(fButtonOnClick onclick, fButtonComplete oncomplete, fButtonIsUp isbuttonup)
	{
		_onclick	= onclick;
		_oncomplete	= oncomplete;
		_isbuttonup	= isbuttonup;
	}

	void poll(void);
	void eventRaised(void);

private:

	fButtonOnClick		_onclick;
	fButtonComplete		_oncomplete;
	fButtonIsUp			_isbuttonup;

	uint8_t	_button;				// button debounce counter/delay
	uint8_t	_state;					// button debounce state
};





//------------------------------------------------------------------------------------------





#endif
