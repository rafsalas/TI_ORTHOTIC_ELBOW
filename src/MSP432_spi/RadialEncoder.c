/*
 * RadialEncoder.c
 *
 *  Created on: Nov 30, 2015
 *      Author: rafael
 */
#include "RadialEncoder.h"


int32_t raw_position = 0;
int32_t pos_pulse_count = 0;
int32_t neg_pulse_count = 0;

void encoderInit(){
    /* Configuring P3.5 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0 |GPIO_PIN1|GPIO_PIN2);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);//A,B,Z
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1);
    MAP_Interrupt_enableInterrupt(INT_PORT4);
    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
}

//----------------------interrupts----------------------------------------------------------------------

void gpio_isr4(void)
{
//	uint32_t status;
	uint8_t val[3];
	val[0] = MAP_GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0);
	val[1] = MAP_GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN1);
	val[2] = MAP_GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2);

//    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);
//    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, status);
    printf(EUSCI_A0_MODULE,"\r\n\r\n");
    if(val[0]==val[1]==val[2] ){ //one step CW
		printf(EUSCI_A0_MODULE,"in neg\r\n");
		raw_position++;
	}else{ //one step CCW
		printf(EUSCI_A0_MODULE,"in pos\r\n");
		raw_position--;
	}
}



