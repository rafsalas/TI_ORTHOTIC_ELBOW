/*
 * drv8.h
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#ifndef DRV8_H_
#define DRV8_H_
#include <driverlib.h>



#define CLK_PERIOD 500


Timer_A_PWMConfig pwmConfig0;
Timer_A_PWMConfig pwmConfig1;
Timer_A_PWMConfig pwmConfig2;
Timer_A_PWMConfig pwmConfig3;
extern uint16_t PWM1;
extern uint16_t PWM2;
extern double Upper_Arm_Intention;
extern double ANGLE_damp;
extern int8_t Direction_flag;
bool fault;


void setup_Motor_Driver();
void setup_PWM();
void drive_forward();
void drive_reverse();
void drive_motor();
void drive_stop();

#endif /* DRV8_H_ */
