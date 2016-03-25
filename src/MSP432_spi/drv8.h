/*
 * drv8.h
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#ifndef DRV8_H_
#define DRV8_H_
#include <driverlib.h>


#define PWM1 500
#define PWM2 500
#define CLK_PERIOD 500

Timer_A_PWMConfig pwmConfig0;
Timer_A_PWMConfig pwmConfig1;
Timer_A_PWMConfig pwmConfig2;
Timer_A_PWMConfig pwmConfig3;
bool fault;


void setup_Motor_Driver();
void setup_PWM();
void drive_forward();
void drive_reverse();
void drive_stop();

#endif /* DRV8_H_ */
