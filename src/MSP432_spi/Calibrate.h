/*
 * Calibrate.h
 *
 * 	Elbow Orthosis
 * 	Texas A&M University & Texas Instruments
 *
 *  Created on: Fall 2015
 *      Author: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas
 */

#ifndef CALIBRATE_H_
#define CALIBRATE_H_
#include <driverlib.h>

extern double ANGLE_max;
extern double ANGLE_min;
extern uint8_t Cal_Request;
extern uint8_t Read_flag;
extern uint16_t Calibration_History;
void reset_position();
void calibration();

extern void	raise_clk_rate();
extern void	lower_clk_rate();



#endif /* CALIBRATE_H_ */
