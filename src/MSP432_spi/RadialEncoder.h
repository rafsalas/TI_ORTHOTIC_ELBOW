/*
 * RadialEncoder.h
 *
 *  Created on: Fall 2015
 *      Author:  Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas
 *      Elbow Orthosis
 *		Texas A&M University & Texas Instruments
 *
 *
 */




#ifndef RADIALENCODER_H_
#define RADIALENCODER_H_

#include <driverlib.h>

extern double ANGLE_deg[50]; // 50 Samples of Angle History
extern double ANGLE_max; // Maximum Permitted Angle from Calibration Routine
extern double ANGLE_min; // Minimum Permitted Angle from Calibration Routine
extern int8_t Direction_flag; // + if Elbow Opening, - if Elbow Closing
extern double ANGLE_damp; // Dampening Coefficient

extern int32_t raw_position;
extern int32_t pos_pulse_count;
extern int32_t neg_pulse_count;

void Angle_Dampen();


#endif /* RADIALENCODER_H_ */
