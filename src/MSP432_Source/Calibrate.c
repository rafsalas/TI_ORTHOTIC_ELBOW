/*
 * Calibrate.c
 * ADC_Sensor.c
 *
 * 	Elbow Orthosis
 * 	Texas A&M University & Texas Instruments
 *
 *  Created on: Fall 2015
 *      Author: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas
 */

#ifndef CALIBRATE_C_
#define CALIBRATE_C_
#include "Calibrate.h"
#include "UART_COMMS.h"
#include "SPI_COMMS.h"
#include "ADC_Sensors.h"
#include "drv8.h"

extern uint16_t resultsBuffer[4];
extern uint16_t PWM1;
extern uint16_t PWM2;
extern double ANGLE_deg[50];


volatile low =0;
volatile high= 0;
volatile left_loop = 0;

void reset_position(){
	// Reset Position
	read_adc(resultsBuffer);
	ANGLE_deg[0] = 0.5*(resultsBuffer[0]+resultsBuffer[0]);

	double ANGLE_set = 0.5*(ANGLE_max-ANGLE_min)+ANGLE_min;

	while(ANGLE_deg[0]<ANGLE_set-10 || ANGLE_deg[0]>ANGLE_set+10){
		read_adc(resultsBuffer);
		ANGLE_deg[0] = 0.5*(resultsBuffer[0]+resultsBuffer[0]);
		if(ANGLE_deg[0]<ANGLE_set)
		{
			Direction_flag = 1;
		}else if(ANGLE_deg[0]>ANGLE_set)
		{
			Direction_flag =-1;
		}

		PWM1=400;
		PWM2=400;
		drive_motor();
		__delay_cycles(10000);


	}
	left_loop =1;
}


void calibration(){

	drive_stop(); // Stop Motor


	while(Read_flag != 1); // Trap Until Bluetooth Buffer Read
	int *temp = read_cal_angles(); // Read Buffer Contents
	ANGLE_min = temp[0]; // Read Minimum Angle
	ANGLE_max = temp[1]; // Read Maximum Angle
	low = ANGLE_min;
	high = ANGLE_max;
	Read_flag = 0; // Reset Read Flag

	uint32_t i;
	int j;
	int k;

	uint32_t SMCLK_cycles = CS_getSMCLK(); // System Clock for Delay Cycles
	uint32_t SMCLK_DIV100K = SMCLK_cycles/(100000); // Divide System Clock by 10,000 Full Cycles



	//////
	// EMG
	//////

	left_loop = 0;

	// 5 Second Delay (User Prompt in Android App)
	for(i = 0; i < 5*SMCLK_DIV100K; i++) __delay_cycles(25000); // 5 Second Delay

	left_loop = left_loop+1;

	// 3 Second Delay (Center Calibration Signal in 10 Second Interval)
	for(i = 0; i < 3*SMCLK_DIV100K; i++) __delay_cycles(25000); // 3 Second Delay

	left_loop = left_loop+1;


	raise_clk_rate();

	// Reset Minimum EMG Signal
	for(i = 0; i < NUM_ACTIVE_CHANNELS; i++) EMG_max[i] = 0; // Average EMG Data

	// Average Multiple Windows
	for(k = 0; k < Calibration_History ;++k)
	{
		SPI_Collect_Data(); // Collect EMG Data
		EMG_Condition_Data(); // Condition EMG Data
		for(i = 0; i < NUM_ACTIVE_CHANNELS; ++i) for(j=0;j<EMG_History;j++)	EMG_max[i] = EMG_max[i] + EMG[i][j]; // Sum EMG Data
	}

	for(i = 0; i < NUM_ACTIVE_CHANNELS; i++) EMG_max[i] = EMG_max[i]/(EMG_History*Calibration_History); // Average EMG Data
	lower_clk_rate();

	left_loop = left_loop+1;
	Cal_Request = 0;
}

#endif /* CALIBRATE_C_ */
