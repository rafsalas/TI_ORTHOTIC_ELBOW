/*
 * drv8.c
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#include "drv8.h"

void setup_Motor_Driver(){
	MAP_WDT_A_holdTimer();
	fault = false;

    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);//nfault
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1);//nsleep
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);

    setup_PWM();
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0);
    MAP_Interrupt_enableInterrupt(INT_PORT5);

    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();


    setup_PWM();

}

void setup_PWM(){
	//PWM1 = 500*Upper_Arm_Intention*ANGLE_damp;
	//PWM2 = 500*Upper_Arm_Intention*ANGLE_damp;
    pwmConfig0.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    pwmConfig0.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    pwmConfig0.timerPeriod = CLK_PERIOD;
    pwmConfig0.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    pwmConfig0.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    pwmConfig0.dutyCycle = PWM1;

    pwmConfig1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    pwmConfig1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    pwmConfig1.timerPeriod = CLK_PERIOD;
    pwmConfig1.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    pwmConfig1.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    pwmConfig1.dutyCycle = PWM1;
//----------------------------------------------------------------------------------
    pwmConfig2.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    pwmConfig2.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    pwmConfig2.timerPeriod = CLK_PERIOD;
    pwmConfig2.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    pwmConfig2.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    pwmConfig2.dutyCycle = PWM2;

    pwmConfig3.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    pwmConfig3.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    pwmConfig3.timerPeriod = CLK_PERIOD;
    pwmConfig3.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    pwmConfig3.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    pwmConfig3.dutyCycle = PWM2;


    MAP_Timer_A_generatePWM(TIMER_A0_MODULE, &pwmConfig0);
    MAP_Timer_A_generatePWM(TIMER_A0_MODULE, &pwmConfig1);
    MAP_Timer_A_generatePWM(TIMER_A1_MODULE, &pwmConfig2);
    MAP_Timer_A_generatePWM(TIMER_A1_MODULE, &pwmConfig3);
}

void drive_forward(){
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);
	//PWM-A2
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);// AIN2 = PWM
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);// AIN1 = Low

	//PWM-B2
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);//BIN2 = PWM
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN5);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN5);

}

void drive_reverse(){
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);
	//PWM-A1 = pwm
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);//pwmA2 = low
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);

	//PWM-B1 =pwm
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
	//PWM-B2 = low
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN7);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN7);
}

void drive_motor(){
	setup_PWM();
	if(Direction_flag == -1){
		drive_reverse();
	}else{
		drive_forward();
	}
}

void drive_stop(){
	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);
	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4|GPIO_PIN5);
	 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN5|GPIO_PIN7);
}

//----------------------interrupts----------------------------------------------------------------------
void interrupt_helper(){// in the event of motor fault
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
	GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
	while(1){
		GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
		__delay_cycles(100000);
		GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
	}
}

void gpio_isr5(void)//fault
{
    uint32_t status;
    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, status);


    if(status & GPIO_PIN0)
    {
    	drive_stop();
    	interrupt_helper();
    }
}
