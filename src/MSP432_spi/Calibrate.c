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
#include "SPI_COMMS.h"

void calibration(){
	while(Read_flag != 1);
	int *temp = read_cal_angles();
	ANGLE_min = temp[0];
	ANGLE_max = temp[1];
	Read_flag = 0;
	int i;
	int j;

	for(i = 0;i <Calibration_History ;++i){
		SPI_Collect_Data();
		EMG_Condition_Data();
		for(j=0;j<8;++j){
			EMG_max[i] = EMG_max[i] + EMG[i][0];
		}
	}
	for(j=0;j<8;++j){
		EMG_max[i] = EMG_max[i]/Calibration_History;
	}

	Cal_Request = 0;
}

#endif /* CALIBRATE_C_ */
