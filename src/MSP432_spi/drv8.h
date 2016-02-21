/*
 * drv8.h
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#ifndef DRV8_H_
#define DRV8_H_
#include <driverlib.h>

// GPIO Port 1 Definitions
#define VREF    BIT3    // pot //P3.0
// GPIO Port 2 Definitions
#define BIN1    BIT1    // P2.6
#define BIN2    BIT2    // P2.7
#define AIN2    BIT4    // P2.5
#define	AIN1    BIT5    // P2.4

Timer_A_PWMConfig pwmConfig0;
Timer_A_PWMConfig pwmConfig1;

void intiallize();
void setup_PWM();
void drive_forward();
void drive_reverse();
void drive_stop();

#endif /* DRV8_H_ */
