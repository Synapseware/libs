#include "events.h"

volatile static event_t		_events[MAX_EVENT_RECORDS];
volatile static uint8_t		_total		= 0;
volatile static uint16_t	_timeBase	= 0;


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
			_events[index].interval	= _events[index+1].interval;
			_events[index].flags	= _events[index+1].flags;
			_events[index].next		= _events[index+1].next;
			_events[index].func		= _events[index+1].func;
			_events[index].state	= _events[index+1].state;
		}
	}

	_events[index].func			= 0;
	_events[index].interval		= 0;
	_events[index].flags		= 0;
	_events[index].next			= 0;
	_events[index].state		= 0;

	_total--;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Preserves the time base, which is the number of eventSync calls per second.
void setTimeBase(uint16_t timeBase)
{
	// save the time base
	_timeBase = timeBase;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Returns the timebase value
uint16_t getTimeBase(void)
{
	// time base
	return _timeBase;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Sets the event flags for the main processing thread and runs high priority events.  This
// should be called by the ISR for the timer that is driving the event system.
void eventSync(void)
{
	for (uint8_t i = 0; i < _total; i++)
	{
		event_t * event = &_events[i];

		// decrement events that haven't hit their schedule yet
		if (event->next > 0)
		{
			event->next--;
			continue;
		}

		// reset event interval
		event->next = event->interval;

		// process high-priority events
		if ((event->flags & (1<<EVENT_PRIORITY_HIGH)))
		{
			event->func(event->state);
		}
		else
		{
			event->flags |= (1<<EVENT_FLAG_PROCESS);
		}
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Processes the events that are ready
void eventsDoEvents(void)
{
	for (uint8_t i = 0; i < _total; i++)
	{
		event_t * event = &_events[i];

		// skip null or already processed events
		if (!(event->flags & (1<<EVENT_FLAG_PROCESS)))
			continue;

		// skip high-priority events
		if ((event->flags & (1<<EVENT_PRIORITY_HIGH)))
			continue;

		// process event
		if (event->func)
			event->func(event->state);

		// mark event as processed
		event->flags &= ~(1<<EVENT_FLAG_PROCESS);

		// remove one-shot events
		if (0 != (event->flags & (1<<EVENT_FLAG_ONESHOT)))
			removeEvent(i);
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Register a normal priority event
void registerEvent(fEventCallback fptr, uint16_t interval, eventState_t state)
{
	if (0 == fptr)
		return;

	_events[_total].flags		= (1<<EVENT_FLAG_REPEATING);
	_events[_total].interval	= interval;
	_events[_total].next		= interval;
	_events[_total].func		= fptr;
	_events[_total].state		= state;

	_total++;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// Register a high priority event.  High priority events run on the same thread
// as the eventSync.
void registerHighPriorityEvent(fEventCallback fptr, uint16_t interval, eventState_t state)
{
	if (0 == fptr)
		return;

	_events[_total].flags		= (1<<EVENT_FLAG_REPEATING) | (1<<EVENT_PRIORITY_HIGH);
	_events[_total].interval	= interval;
	_events[_total].next		= interval;
	_events[_total].func		= fptr;
	_events[_total].state		= state;

	_total++;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// One shot events run after a specific interval, and are then removed
void registerOneShot(fEventCallback fptr, uint16_t interval, eventState_t state)
{
	if (0 == fptr)
		return;

	_events[_total].flags		= (1<<EVENT_FLAG_ONESHOT);
	_events[_total].interval	= interval;
	_events[_total].next		= interval;
	_events[_total].func		= fptr;
	_events[_total].state		= state;

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
		_events[i].func			= 0;
		_events[i].interval		= 0;
		_events[i].flags		= 0;
		_events[i].next			= 0;
		_events[i].state		= 0;
	}

	_total = 0;
}