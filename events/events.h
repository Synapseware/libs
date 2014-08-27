#ifndef __EVENTS__
#define __EVENTS__


#include "../asyncTypes.h"

#ifndef MAX_EVENT_RECORDS
	#warning MAX_EVENT_RECORDS not specified, defaulting to 8.
	#define MAX_EVENT_RECORDS	8
#endif


#define EVENT_FLAG_PROCESS		0
#define EVENT_FLAG_REPEATING	1
#define EVENT_FLAG_ONESHOT		2
#define EVENT_PRIORITY_HIGH		7

#define EVENT_STATE_NONE		0


#ifdef __cplusplus
extern "C" {
#endif


typedef void*			eventState_t;
typedef uint8_t			flags_t;


// Events event handler signature
typedef void (*fEventCallback)(eventState_t);


typedef struct
{
	fEventCallback	func;
	uint16_t		interval;
	uint16_t		next;
	flags_t			flags;
	eventState_t	state;
} event_t;


// set the time base once at startup.  this value should reflect
// the number of times the eventSync() method is called-per-second
void setTimeBase(uint16_t timeBase);

// get the time base value that is currently set
uint16_t getTimeBase(void);

// the event sync should be called by a timer event handler.
// it's job is to flag registered events that need to be executed
// events are co-operative
void eventSync(void);


// this method should be called whenever there are spare
// cycles to execute pending events
void eventsDoEvents(void);


// updates the specified event with new state information
void eventsUpdateState(uint8_t eventIndex, eventState_t newState);


// register a callback function using this method.  the delay
// is a multiple of the eventSync period
void registerEvent(fEventCallback fptr, uint16_t delay, eventState_t state);


// register a callback function with high priority.  high priority events
// are called by the eventSync method, instead of in the normal way
// by the eventsDoEvents() method
void registerHighPriorityEvent(fEventCallback fptr, uint16_t delay, eventState_t state);


// register a one-shot callback function.  one-shot callbacks will
// only be executed once, and will be removed after execution.
// the callback delay is basically time from registration
void registerOneShot(fEventCallback fptr, uint16_t delay, eventState_t state);


// Removes an event from the event list and frees up it's allocation
void eventsUnregisterEvent(fEventCallback fprt);


// Unregisters all events
void eventsUnregisterAll(void);

#ifdef  __cplusplus
}
#endif

#endif
