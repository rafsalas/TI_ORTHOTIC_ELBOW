/*
 * Calibrate.h
 *
 *  Created on: Apr 3, 2016
 *      Author: rafael
 */

#ifndef CALIBRATE_H_
#define CALIBRATE_H_
#include <driverlib.h>

extern int ANGLE_max;
extern int ANGLE_min;
extern uint8_t Cal_Request;
extern uint8_t Read_flag;
void calibration();


#endif /* CALIBRATE_H_ */
