// RadialEncoder.c

// Elbow Orthosis
// Texas A&M University & Texas Instruments
// Fall 2015 - Spring 2016
// Authors: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas

#include "RadialEncoder.h"


int32_t raw_position = 0;
int32_t pos_pulse_count = 0;
int32_t neg_pulse_count = 0;

// ANGLE DAMPER LOOK-UP TABLES
// NORMAL CDF BASIS
const int N_DAMP = 20; // Order of Damping Look-Up Table
double h_DAMP[20] = { // Coefficients in Damping Look-Up Table
		0,
		0.0637646354428060,
		0.166161647646428,
		0.306579295220414,
		0.471011823050417,
		0.635444350880420,
		0.775861998454406,
		0.878259010658028,
		0.942023646100834,
		0.975930899347034,
		0.991327201858212,
		0.997296733567729,
		0.999273025352170,
		0.999831667917365,
		0.999966493618762,
		0.999994274700306,
		0.999999161721359,
		0.999999895622512,
		0.999999989704829,
		1
};



void Angle_Dampen(){
	uint32_t ANGLE_i;

	// Bound Angle Value between Minimum and Maximum Values
	if(ANGLE_deg[0]>=ANGLE_max) ANGLE_deg[0] = ANGLE_max-0.5*(ANGLE_max-ANGLE_min)/N_DAMP;
	if(ANGLE_deg[0]<=ANGLE_min) ANGLE_deg[0] = ANGLE_min+0.5*(ANGLE_max-ANGLE_min)/N_DAMP;

	// Convert Measured Angle into Index for Lookup Table
	ANGLE_i=(N_DAMP*((uint32_t)(ANGLE_deg[0]-ANGLE_min))/((uint32_t)(ANGLE_max-ANGLE_min)));

	// Produce Damping Coefficient from Normal CDF Curve
	if(Direction_flag<0) ANGLE_damp=h_DAMP[ANGLE_i]; // Decreasing Angle (ELBOW CLOSING)
	else            ANGLE_damp=h_DAMP[N_DAMP-ANGLE_i-1]; // Increasing Angle (ELBOW OPENING)

}



