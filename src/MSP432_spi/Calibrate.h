/*
 * Calibrate.h
 *
 *  Created on: Apr 3, 2016
 *      Author: rafael
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


#endif /* CALIBRATE_H_ */
