#ifndef MMA7260Q_H
#define MMA7260Q_H

#include "../board.h"
#include <avr/io.h>
#include <avr/interrupt.h>


#define ADC_MUX_BITS	(1<<ADLAR)
#define ADC_enable_axis(index) (ADMUX = ADC_MUX_BITS | index)


#define MOTION_BUFFER_LEN	2
#define ACCEL_CHANNELS		3


typedef struct
{
	unsigned char values[MOTION_BUFFER_LEN];
	unsigned char index;
	unsigned char average;
} MOTION_DATA;


// allow the motion data object to be shared
extern MOTION_DATA motion_data[ACCEL_CHANNELS];


void init_motion(void);
void updateMotionAverage(unsigned char sample, MOTION_DATA* data);


#endif
