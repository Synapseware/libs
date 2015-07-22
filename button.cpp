#include "button.h"





//------------------------------------------------------------------------------------------
// Processes a button click with proper debounce
void ButtonClickDebounce::poll(void)
{
	switch (_state)
	{
		case STATE_DEBOUNCE:
			if (0 != _onclick)
				_onclick();

			// after change modes, ignore button presses for a timeout
			_button	= BUTTON_TIMEOUT;
			_state	= STATE_TIMEOUT;

			break;

		case STATE_TIMEOUT:
			// make sure button is up for at least the timeout period
			if (0 != _isbuttonup && 0 != _isbuttonup())
			{
				if (_button-- == 0)
				{
					_state	= STATE_PROCESSED;

					// call the onrelease method
					if (0 != _oncomplete)
						_oncomplete();
				}
			}
			else
				_button = BUTTON_TIMEOUT;
			break;
		case STATE_PROCESSED:
			//NO-OP
			break;
	}
}


//------------------------------------------------------------------------------------------
// Called by an ISR when a button event is raised
void ButtonClickDebounce::eventRaised(void)
{
	// only start button state processing if it's been processed completely
	if (_state == STATE_PROCESSED)
		_state = STATE_DEBOUNCE;
}
