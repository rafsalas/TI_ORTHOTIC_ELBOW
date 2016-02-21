/*
 * RadialEncoder.h
 *
 *  Created on: Nov 30, 2015
 *      Author: rafael
 */

#ifndef RADIALENCODER_H_
#define RADIALENCODER_H_

#include <driverlib.h>
#include "printf.h"
extern int32_t raw_position;
extern int32_t pos_pulse_count;
extern int32_t neg_pulse_count;
void encoderInit();


#endif /* RADIALENCODER_H_ */
