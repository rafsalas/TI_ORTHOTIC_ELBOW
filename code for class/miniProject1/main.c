#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"

unsigned long ulPeriod2;
int main(void)
{

	//unsigned long ulPeriod;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);

	GPIOPadConfigSet( GPIO_PORTF_BASE ,  GPIO_PIN_0|GPIO_PIN_4 , GPIO_STRENGTH_2MA , GPIO_PIN_TYPE_STD_WPU );

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);

	ulPeriod2 = (SysCtlClockGet() / 10)/2 ;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod2 -1);

	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);
	//long pind =0;
	while(1)
	{
	//	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)){
	//		pind = 4;
	//	}
	//	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)){
	//		pind =8;
	//	}
	//	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, pind);
	//	SysCtlDelay(500000);
	}



}

void Timer0IntHandler(void)
{
// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
// Read the current state of the GPIO pin and
// write back the opposite state

	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)){
			//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
			//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
		ulPeriod2=ulPeriod2*2;
		TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod2);
	}
	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)){
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
	}
	else if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1)){
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
	}
	else if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2)){
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);
	}
	else if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3)){
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
	}
	else
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 2);
}
