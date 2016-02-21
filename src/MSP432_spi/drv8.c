/*
 * drv8.c
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#include "drv8.h"

void intiallize(){
	MAP_WDT_A_holdTimer();


	// initialize pins
	//pwm pins
   // MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    //MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
	//sleep and fault
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);//nfault
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1);//nsleep
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);

    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0);
    MAP_Interrupt_enableInterrupt(INT_PORT5);

    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();


}

void setup_PWM(){
    pwmConfig0.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    pwmConfig0.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    pwmConfig0.timerPeriod = 32000;
    pwmConfig0.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    pwmConfig0.compareOutputMode = TIMER_A_OUTPUTMODE_TOGGLE;
    pwmConfig0.dutyCycle = 3200;

    pwmConfig1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    pwmConfig1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    pwmConfig1.timerPeriod = 32000;
    pwmConfig1.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    pwmConfig1.compareOutputMode = TIMER_A_OUTPUTMODE_TOGGLE;
    pwmConfig1.dutyCycle = 3200;
    MAP_Timer_A_generatePWM(TIMER_A0_MODULE, &pwmConfig0);
    MAP_Timer_A_generatePWM(TIMER_A1_MODULE, &pwmConfig1);
}

void drive_forward(){
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);
	//PWM-A2
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);// AIN2 = PWM
	//HWREG16(0x40004C01 + OFS_PASEL1) &= ~GPIO_PIN4;// deselect pin
	//PWM-A1
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);// AIN1 = HIGH

	//PWM-B2
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);
	//HWREG16(0x40004C60 + OFS_PASEL1) &= ~GPIO_PIN5;// deselect pin
	//PWM-B1
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN5);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN5);


}

void drive_reverse(){
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);
	//PWM-A1
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
	//HWREG16(0x40004C01 + OFS_PASEL1) &= ~GPIO_PIN5;// deselect pin
	//PWM-B2
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
	//PWM-B1
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
	//HWREG16(0x40004C60 + OFS_PASEL1) &= ~GPIO_PIN7;// deselect pin
	//PWM-B2
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN7);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN7);

}

void drive_stop(){
	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);
	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4|GPIO_PIN5);
	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN5|GPIO_PIN7);
}

