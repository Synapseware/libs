#include "utils.h"






// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Set and clear bits in a register
void setBitsInRegister(uint8_t port, uint8_t mask)
{
	asm volatile
    (
    	"\n"
        "L_setBitsInRegisterl1%=:"	"\n\t"
    	"mov r30, %0"				"\n\t"
    	"mov r31, __zero_reg__"		"\n\t"
    	"adiw r30, 0x20"			"\n\t"
    	"ld  r16, Z"				"\n\t"
    	"or	 r16, %1"				"\n\t"
    	"st	 Z, r16"
    	: /* no outouts */
    	: "r" (_SFR_IO_ADDR(port)),
    	  "r" (mask)
    	: "r16", "r30", "r31"
    );
}
void clearBitsInRegister(uint8_t port, uint8_t mask)
{
	asm volatile
    (
    	"\n"
        "L_clearBitsInRegisterl1%=:" "\n\t"
    	"mov r30, %0"				"\n\t"
    	"mov r31, __zero_reg__"		"\n\t"
    	"adiw r30, 0x20"			"\n\t"
    	"ld  r16, Z"				"\n\t"
    	"and r16, %1"				"\n\t"
    	"st	 Z, r16"
    	: /* no outouts */
    	: "r" (_SFR_IO_ADDR(port)),
    	  "r" (mask)
    	: "r16", "r30", "r31"
    );
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Calculates the minimum of two numbers.
int8_t min(int8_t a, int8_t b)
{
	return a < b ? a : b;
}
int16_t min_l(int16_t a, int16_t b)
{
	return a < b ? a : b;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Calculates the maximum of two numbers.
int8_t max(int8_t a, int8_t b)
{
	return a > b ? a : b;
}
int16_t max_l(int16_t a, int16_t b)
{
	return a > b ? a : b;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Constrains a number to be within a range.
int8_t constrain(int8_t x, int8_t a, int8_t b)
{
	if (x < a)
		return a;

	if (x > b)
		return b;

	return x;
}
int16_t constrain_l(int16_t x, int16_t a, int16_t b)
{
	if (x < a)
		return a;

	if (x > b)
		return b;

	return x;
}

/*
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Computes the absolute value of a number.
uint8_t abs(uint8_t a)
{
	return a > -1
		? a
		: (a ^ 0xFF) + 1;
}
int16_t abs_l(int16_t a)
{
	return a > -1
		? a
		: (a ^ 0xFFFFUL) + 1;
}
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Remaps a value from one range to another
int8_t map(int8_t x, int8_t in_min, int8_t in_max, int8_t out_min, int8_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
int16_t map_16(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
int32_t map_32(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
double mapd(double x, double in_min, double in_max, double out_min, double out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Converts an uint8_t to a int8_t.  That is:
// [128 to 255] => [0 to 127]
// [0 to 127] => [-128 to -1]
int8_t to_int8(uint8_t ux)
{
	return ux > 127
		? ux - 128
		: ((128 - ux) ^ 0xFF) + 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Converts an uint16_t to a int16_t.  That is:
// [32768 to 65535] => [0 to 32767]
// [0 to 32767] => [-32768 to -1]
int16_t to_int16(uint16_t ux)
{
	return ux > 32767UL
		? ux - 32768UL
		: ((32768UL -ux) ^ 0xFFFF) + 1;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Converts a int8_t to an uint8_t
// [0 to 127] => [128 to 255]
// [-128 to -1] => [0 to 127]
uint8_t to_uint8(int8_t sx)
{
	return sx >= 0
		? ((uint8_t) sx) + 128
		: (sx ^ 0xFF) + 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Converts an int16_t to a uint16_t.  That is:
// [0 to 32767] => [32768 to 65535]
// [-32768 to -1] => [0 to 32767]
uint16_t to_uint16(int16_t sx)
{
	return sx >= 0
		? ((uint16_t) sx) + 32768UL
		: (sx ^ 0xFFFFUL) + 1;
}
