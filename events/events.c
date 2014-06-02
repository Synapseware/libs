#include "events.h"

volatile static event_t		_events[MAX_EVENT_RECORDS];
volatile static uint8_t		_total		= 0;
volatile static uint16_t	_timeBase	= 0;
volatile static uint8_t		_eventTick	= 0;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Removes an event from the list
void removeEvent(uint8_t index)
{
	// bounds check
	if (index >= _total || _total == 0)
		return;

	// slide events or zero the record
	if (index < MAX_EVENT_RECORDS - 1)
	{
		// copy events & slide them down
		for (; index <_total - 1; index++)
		{
			_events[index].delay	= _events[index+1].delay;
			_events[index].flags	= _events[index+1].flags;
			_events[index].next		= _events[index+1].next;
			_events[index].func		= _events[index+1].func;
			_events[index].state	= _events[index+1].state;
		}
	}

	_events[index].func		= 0;
	_events[index].delay	= 0;
	_events[index].flags	= 0;
	_events[index].next		= 0;
	_events[index].state	= 0;

	_total--;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Preserves the time base, which is the number of eventSync calls per second.
void setTimeBase(uint16_t timeBase)
{
	_timeBase = timeBase;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Returns the timebase value
uint16_t getTimeBase(void)
{
	return _timeBase;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Sets the event flags for the main processing thread
void eventSync(void)
{
	_eventTick = 1;
	for (uint8_t i = 0; i < _total; i++)
	{
		// skip any event that doesn't have a function pointer
		if (!_events[i].func)
			continue;

		// decrement events that haven't hit their schedule yet
		if (_events[i].next > 1)
		{
			_events[i].next--;
			continue;
		}

		// process high-priority events
		if ((_events[i].flags & (1<<EVENT_PRIORITY_HIGH)))
		{
			_events[i].func(_events[i].state);
		}
		else
		{
			_events[i].flags |= (1<<EVENT_FLAG_PROCESS);
		}

		// reset event delay
		_events[i].next = _events[i].delay;
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Processes the events that are ready
void eventsDoEvents(void)
{
	if (0 == _eventTick)
	return;

	for (uint8_t i = 0; i < _total; i++)
	{
		// skip null-events
		if (!_events[i].func)
			continue;

		// skip processed events
		if (!(_events[i].flags & (1<<EVENT_FLAG_PROCESS)))
			continue;

		// skip high-priority events
		if ((_events[i].flags & (1<<EVENT_PRIORITY_HIGH)))
			continue;

		// process event!  :)
		_events[i].flags &= ~(1<<EVENT_FLAG_PROCESS);
		_events[i].func(_events[i].state);

		// remove one-shot events
		if (0 != (_events[i].flags & (1<<EVENT_FLAG_ONESHOT)))
			removeEvent(i);
	}

	_eventTick = 0;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Register a normal priority event
void registerEvent(fEventCallback fptr, uint16_t delay, eventState_t state)
{
	_events[_total].flags	= (1<<EVENT_FLAG_REPEATING);
	_events[_total].delay	= delay;
	_events[_total].next	= delay;
	_events[_total].func	= fptr;
	_events[_total].state	= state;

	_total++;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Register a high priority event.  High priority events run on the same thread
// as the eventSync.
void registerHighPriorityEvent(fEventCallback fptr, uint16_t delay, eventState_t state)
{
	_events[_total].flags	= (1<<EVENT_FLAG_REPEATING) | (1<<EVENT_PRIORITY_HIGH);
	_events[_total].delay	= delay;
	_events[_total].next	= delay;
	_events[_total].func	= fptr;
	_events[_total].state	= state;

	_total++;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// One shot events run after a specific delay, and are then removed
void registerOneShot(fEventCallback fptr, uint16_t delay, eventState_t state)
{
	_events[_total].flags	= (1<<EVENT_FLAG_ONESHOT);
	_events[_total].delay	= delay;
	_events[_total].next	= delay;
	_events[_total].func	= fptr;
	_events[_total].state	= state;

	_total++;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Removes an event from the event list and frees up it's allocation
void eventsUnregisterEvent(fEventCallback fprt)
{
	for (uint8_t i = 0; i < _total; i++)
	{
		if (_events[i].func == fprt)
			removeEvent(i);
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Unregisters all the events
void eventsUnregisterAll(void)
{
	for (uint8_t i = 0; i < MAX_EVENT_RECORDS; i++)
	{
		_events[i].func		= 0;
		_events[i].delay	= 0;
		_events[i].flags	= 0;
		_events[i].next		= 0;
		_events[i].state	= 0;
	}

	_total = 0;
}