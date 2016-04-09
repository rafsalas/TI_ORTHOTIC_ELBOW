/*
 * Calibrate.c
 *
 *  Created on: Apr 3, 2016
 *      Author: rafael
 */

#ifndef CALIBRATE_C_
#define CALIBRATE_C_
#include "Calibrate.h"
#include "UART_COMMS.h"

void calibration(){
	while(Read_flag != 1);
	int *temp = read_cal_angles();
	ANGLE_min = temp[0];
	ANGLE_max = temp[1];

}

#endif /* CALIBRATE_C_ */
