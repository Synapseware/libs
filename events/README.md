Events
======
The Events class is for managing timed or periodic events.  It's meant to simplify the process of polling and other periodic, asynchronous activity.  It can be used for generating sound effects, lighting effects, etc.

## Usage
Typical setup:
* Define the period size with Events::setTimeBase(unsigned short)
* Setting up the Events::sync(void) method in an ISR
* Calling the Events::doEvents(void) method
* Registering event handlers

### Example scenario
**Call a function every 0.5 seconds.**
* Theory: Using an ATMEGA48/88/etc uC on a 20MHz CPU clock with Timer1 configured in CTC mode to reset when TCNT1 = OCR0A, and OCR0A set to F_CPU/1000-1, the ISR below will be called once every millisecond.  The Events::sync(void) method allows the Events class to count the number of events each cycle.  Any registered event that has a trigger count lower than the current cycle count and that hasn't been processed for the current cycle, will be called on the next Events::doEvents(void) call.  Since the counting is separated from the processing of registered events, event handlers that run long, etc, can be easily handled by the Events class.  So for this example, 

### Code
```c++
#include <events.h>
#include <io.h>

// Declare an instance of the Events class that has room for 4 event
Events events(4);

void ledToggle(eventState_t state)
{
    // assuming the LED is connected to PORTC1
    PORTC ^= (1<<PORTC1);
}

int main()
{
    // set LED pin as output
    DDRC |= (1<<PORTC1);

    // let the Events instance know what the period size is
    events.setTimeBase(1000);

    // Register a function to toggle an I/O pin to turn an LED on and off
    events.registerEvent(ledToggle, 500, EVENT_STATE_NONE);

    // Process the events by calling the doEvents() method
    while(1)
    {
        events.doEvents();
    }
}

ISR(TIMER1_OVF_vect)
{
    // assuming TIMER1 is setup to overflow at 1ms intervals
    events.sync();
}
```

## Event registrations
There are 3 event types, each for a specific event model/use case:

### Events::registerEvent(handler, count, state)
Registers a callback back to be handled each time the count value is reached.  These callbacks are invoked from the Events::doEvents(void) method which means they are disconnected from the Events::sync(void) method (and outside of any ISR that might have called the Events::sync method).

### Events::registerOneShot(handler, count, state)
Same as the Events::registerEvent except this event will be called once before it's removed from the list of events handlers to process.

### Events::registerHighPriority(handler, count, state)
Similar to the Events::registerEvent method, this registration method registers a function to be called on a recurring schedule.  The exception here is that high priority events are called within the Events::sync method (usually making them called by the ISR when the timer interrupt occurs).
High priority events should be kept short since they delay all other event processing.
