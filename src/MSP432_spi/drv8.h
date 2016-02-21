/*
 * drv8.h
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#ifndef DRV8_H_
#define DRV8_H_
#include <driverlib.h>


#define PWM1 16000
#define PWM2 16000

Timer_A_PWMConfig pwmConfig0;
Timer_A_PWMConfig pwmConfig1;
bool fault;


void intiallize();
void setup_PWM();
void drive_forward();
void drive_reverse();
void drive_stop();

#endif /* DRV8_H_ */
