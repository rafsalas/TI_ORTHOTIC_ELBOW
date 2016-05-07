/*
 * drv8.h
 *
 * 	Elbow Orthosis
 * 	Texas A&M University & Texas Instruments
 *
 *  Created on: Fall 2015
 *      Author: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas
 *
 *      Two PWMs are set up and ran to control dual motors (PWM A and B). Each PWM requires two pins.
 *      When one pin is set to PWM signal the other is set low. Direction of the motor is dependant on
 *      which pin is set to pwm. An enable pin is used to enable motor movement and a fault pin is used to indicated failure
 *      Pin map
 *      2.4 = AIN1
 *      2.5 = AIN2
 *      7.5 = BIN1
 *      7.7 = BIN2
 *      5.1 = enable
 *      5.0 = fault
 *
 *      CLK = SMCLK
 */

#ifndef DRV8_H_
#define DRV8_H_
#include <driverlib.h>



#define CLK_PERIOD 1000


Timer_A_PWMConfig pwmConfig0;
Timer_A_PWMConfig pwmConfig1;
Timer_A_PWMConfig pwmConfig2;
Timer_A_PWMConfig pwmConfig3;
extern uint16_t PWM1;
extern uint16_t PWM2;

// motor calculation constants
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
