#ifndef __BLINK_TABLES__
#define __BLINK_TABLES__

#ifndef BLINK_TABLE_FACTOR
	#err BLINK_TABLE_FACTOR is required and was not declared.
#endif


#ifdef __cplusplus
extern "C" {
#endif



//------------------------------------------------------------------------------------------
// each "delay" value is in units of 10ms
// So, if a delay is set for 12, then the total time is 12 * 10ms, or 120ms.
#define BLINK_TABLE_DELAY 125	// 3 fast blinks and 1 slow blink
static const uint8_t BLINK_TABLE[] PROGMEM = {
	// brightness	delay
	(uint8_t)(0.10 * BLINK_TABLE_FACTOR),
	0,
	(uint8_t)(0.20 * BLINK_TABLE_FACTOR),
	0,
	(uint8_t)(0.30 * BLINK_TABLE_FACTOR),
	0,
	(uint8_t)(1.00 * BLINK_TABLE_FACTOR),
	0,
	(uint8_t)(1.00 * BLINK_TABLE_FACTOR),
	0
};
static const uint8_t BLINK_TABLE_LEN = sizeof(BLINK_TABLE)/sizeof(uint8_t);

#define SLEEP_TABLE_DELAY 80 // sleepy eye effect
static const uint8_t SLEEP_TABLE[] PROGMEM = {
	(uint8_t)(0.10 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.18 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.25 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.33 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.50 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.70 * BLINK_TABLE_FACTOR),
	(uint8_t)(1.00 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.70 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.50 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.33 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.25 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.18 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.10 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.07 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.04 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.02 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.01 * BLINK_TABLE_FACTOR)
};
static const uint8_t SLEEP_TABLE_LEN = sizeof(SLEEP_TABLE)/sizeof(uint8_t);

#define FADE_TABLE_DELAY 80
static const uint8_t FADE_TABLE[] PROGMEM = {
	(uint8_t)(1.00 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.30 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.15 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.10 * BLINK_TABLE_FACTOR),
	(uint8_t)(0.07 * BLINK_TABLE_FACTOR),
	1,
	1,
	1
};
static const uint8_t FADE_TABLE_LEN = sizeof(FADE_TABLE)/sizeof(uint8_t);

#ifdef  __cplusplus
}
#endif


#endif
