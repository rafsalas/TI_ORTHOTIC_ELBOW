// RadialEncoder.h

// Elbow Orthosis
// Texas A&M University & Texas Instruments
// Fall 2015 - Spring 2016
// Authors: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas


#ifndef RADIALENCODER_H_
#define RADIALENCODER_H_

#include <driverlib.h>
#include "printf.h"

extern double ANGLE_deg[50]; // 50 Samples of Angle History
extern double ANGLE_max; // Maximum Permitted Angle from Calibration Routine
extern double ANGLE_min; // Minimum Permitted Angle from Calibration Routine
extern int8_t ANGLE_dir; // + if Elbow Opening, - if Elbow Closing
extern double ANGLE_damp; // Dampening Coefficient

extern int32_t raw_position;
extern int32_t pos_pulse_count;
extern int32_t neg_pulse_count;

void Angle_Dampen();


#endif /* RADIALENCODER_H_ */
