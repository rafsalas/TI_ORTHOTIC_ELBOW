
/*
* main.c
-
PWM Lab
*
* Note from the
datasheet
:
* In PWM mode, timers are configured as 24
-
bit or 48
-
bit down
-
cou
nter with assigned
start value (corresponding to period) defined by GPTMTnILR and GPTMTnPR registers.
*/
#include "inc/hw_memmap.h"
#include"inc/hw_types.h"
#include"inc/hw_ints.h"
#include"driverlib/sysctl.h"
#include"driverlib/interrupt.h"
#include"driverlib/pin_map.h"
#include"driverlib/gpio.h"
#include"driverlib/timer.h"

int main(void){
	unsigned long ulPeriod;
// sets the period, and thus frequency, of our PWM
	unsigned long dutyCycle1, dutyCycle2, dutyCycle3, dutyCycle4;
// Configures to drive 400 MHz PLL by sys_clk -> 16 MHz xtal
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
//Set clock speed = 40MHz
/* Configure specific GPIO pins to use the CCP pins associated with certain timers.
*   More info about GPIO pin options in Table 10-2 ofDatasheet.
**/
// Enable the GPIO ports where our CCP GPIO pins are
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
// Enable the GPIO portcontaining the T0 CCP0 and CCP1 pins
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
// Enable port F, containing T1CCP0, CCP1
/* Reference: DataSheet-1.3.4.4 CCP Pins (under Architectural Overview
 	*CCP Pins can be used by GPTM (see Lab 4 in the workbook) to time/count
	external events with CCP pin as input.
	GPTM can also generate a PWM output on CCP
	pin.
 	 *
	For PWM, the GPTM is incremented (or decremented) by system clock. The
	PWM signal (which is output) is generated based on match between counter value and
	value stored in match register.
*/
// Configure pins PB6, PB7 as Timer_0 CCP0, CCP1
// Configure pins PF2, PF3 as Timer_1 CCP0, CCP1
	GPIOPinConfigure(GPIO_PF2_T1CCP0);
// Configure pin PF2 as Timer 1_A output
	GPIOPinConfigure(GPIO_PF3_T1CCP1);
// Configure pin PB3 as Timer 1_B output
	GPIOPinConfigure(GPIO_PB6_T0CCP0);
// Configure pin PB6 as Timer 0_A output
	GPIOPinConfigure(GPIO_PB7_T0CCP1);
// Configure pin PB7 as Timer 0_B output
/* Note that if you get errors here, you may have to replace the named terms
for actual hex address, found in he
ader file. */
	GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_2 );
// Enable pin PF2 as output of timer addressed to it
	GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_3 );
// Enable pin PF3 as output of timer addressed to it
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_6 );
// Enable pin PB6 as output of timer addressed to it
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_7 );
// Enable pin PB7 as output oftimer addressed to it
// SysCtlClockGet() will give you the number of clock cycles in 1 second. This is convenient for
// directly converting to clock frequency division, as one second corresponds to one Hertz.

	ulPeriod = (SysCtlClockGet() / 500)/2;
	dutyCycle1 = (unsigned long)(ulPeriod-1)*0.8;
	dutyCycle2 = (unsigned long)(ulPeriod-1)*0.6;
	dutyCycle3 = (unsigned long)(ulPeriod-1)*0.4;
	dutyCycle4 = (unsigned long)(ulPeriod-1)*0.2;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	// Enable Timer 1
	SysCtlPeripheralEnable
	(SYSCTL_PERIPH_TIMER0);
	// Enable Timer 0
	// Configure the timers as split 16-bit pairs, and set each timer as PWM mode.
	TimerConfigure(TIMER1_BASE,(TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM));
	TimerConfigure(TIMER0_BASE,(TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM));
	TimerControlLevel(TIMER1_BASE, TIMER_BOTH, 0);
	// Timer 1 is trigger low
	TimerControlLevel(TIMER0_BASE, TIMER_BOTH, 0);
	// Timer 0 is trigger low
	// For simplicity, we don't use PWM interrupts, for which we would need to configure the TnEVENT field in GPTMCTL and eanble
	// For simplicity, we don't use prescaler load and match set here.
	// If we do use prescaler, the API functions would write value to GPTM Timer nPrescaleReg(GPTMTnPR)
	// These API functions load the timer start value into Timer n Interval LoadReg(GPTMTnILR)Reg 10, 11
	// Timer 0 Load set
	TimerLoadSet(TIMER1_BASE, TIMER_A, ulPeriod-1);
	TimerLoadSet(TIMER1_BASE, TIMER_B, ulPeriod-1);
	// Timer 0 Load set
	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod-1);
	TimerLoadSet(TIMER0_BASE, TIMER_B, ulPeriod-1);
// These API functions load the match value into Timer n Match register(GPTMTnMATCHR)
// Timer 1 Match set
	TimerMatchSet(TIMER1_BASE, TIMER_A, dutyCycle1);
	TimerMatchSet(TIMER1_BASE, TIMER_B, dutyCycle2);
// Timer 0 Match set
	TimerMatchSet(TIMER0_BASE, TIMER_A, dutyCycle3);
	TimerMatchSet(TIMER0_BASE, TIMER_B, dutyCycle4);
	/* Timers are now configured. */
	// Finally, enable the timers, which will now run (API functions will set TnENbit in Reg 4 (GPTMCTL)
	TimerEnable(TIMER1_BASE, TIMER_BOTH);
	TimerEnable(TIMER0_BASE, TIMER_BOTH);
	// Continuous while loop, and Timers will be counting in PWM mode
	while(1){


	}

}
