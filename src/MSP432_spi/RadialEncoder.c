/*
 * RadialEncoder.c
 *
 *  Created on: Nov 30, 2015
 *      Author: rafael
 */
#include "RadialEncoder.h"

void encoderInit(){
    /* Configuring P3.5 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0 |GPIO_PIN1|GPIO_PIN2);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);//A,B,Z
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1);
    MAP_Interrupt_enableInterrupt(INT_PORT4);
    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
}





