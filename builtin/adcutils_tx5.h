#ifndef __ADCUTILS_TX5_H__
#define __ADCUTILS_TX5_H__


const uint8_t VREF_VCC				= 0x00;		// VCC, disconnected from PB0
const uint8_t VREF_AREF				= 0x01;		// External voltage on AREF pin
const uint8_t VREF_11				= 0x02;		// Internal 1.1v reference
const uint8_t VREF_256				= 0x06;		// No bypass cap, PB0 disconnected from AREF
const uint8_t VREF_256_BYPASS		= 0x07;		// Bypass cap on PB0 (AREF pin)


#endif
