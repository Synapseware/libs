#ifndef _UTILS_H_
#define _UTILS_H_


#include <avr/io.h>
#include <types.h>
#include <stdlib.h>
#include <stdint.h>



#ifdef __cplusplus
extern "C" {
#endif


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Set and clear bits in a register
void setBitsInRegister(uint8_t port, uint8_t mask);
void clearBitsInRegister(uint8_t port, uint8_t mask);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Calculates the minimum of two numbers.
int8_t min(int8_t a, int8_t b);
int16_t min_l(int16_t a, int16_t b);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Calculates the maximum of two numbers.
int8_t max(int8_t a, int8_t b);
int16_t max_l(int16_t a, int16_t b);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Constrains a number to be within a range.
int8_t constrain(int8_t x, int8_t a, int8_t b);
int16_t constrain_l(int16_t x, int16_t a, int16_t b);

/*
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Computes the absolute value of a number.
int8_t abs(int8_t a);
int16_t abs_l(int16_t a);
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Remaps a value from one range to another
int8_t map(int8_t x, int8_t in_min, int8_t in_max, int8_t out_min, int8_t out_max);
int16_t map_16(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max);
int32_t map_32(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
double mapd(double x, double in_min, double in_max, double out_min, double out_max);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Converts an uint8_t to a signed int8_t
int8_t to_int8(uint8_t ux);
int16_t to_int16(uint16_t ux);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Converts a signed int8_t to an uint8_t
uint8_t to_uint8(int8_t sx);
uint16_t to_uint16(int16_t sx);


#ifdef  __cplusplus
}
#endif

#endif
