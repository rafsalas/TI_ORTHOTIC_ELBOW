#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.c"
//#include "float.h"
/*
 * main.c
 */

#ifdef DEBUG
void__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif
unsigned long ulADC0Value[1];
unsigned long dutyCycle,ulPeriod;
double ADC_Per;
void ADC0IntHandler(void){

	ADCSequenceDataGet(ADC0_BASE, 3, ulADC0Value);
	ADC_Per = (double)(ulADC0Value[0])/4096.00;
	if (ADC_Per<0.25)
		ADC_Per = .25;
	if (ADC_Per>0.75)
		ADC_Per = .75;


	dutyCycle = (unsigned long)((ulPeriod-1)*(ADC_Per));


	/*TimerLoadSet(TIMER1_BASE, TIMER_A, ulPeriod-1);
	TimerMatchSet(TIMER1_BASE, TIMER_A, dutyCycle);
	TimerEnable(TIMER1_BASE, TIMER_A );*/
	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod-1);
	TimerMatchSet(TIMER0_BASE, TIMER_A, dutyCycle);
	TimerEnable(TIMER0_BASE, TIMER_A );

	UARTprintf("ADC VALUE: %d, Period: %d ,Duty Cycle: %d\n", ulADC0Value[0], (ulPeriod-1),dutyCycle);
	ADCIntClear(ADC0_BASE,3);
}




int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //set up Uart comms
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE,  GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
	UARTprintf("\nUart Ready\n\n");

    //ADC set up using ADC0 sewuencer 3
    GPIOPinTypeADC (GPIO_PORTE_BASE, GPIO_PIN_0);
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	ADCSequenceDisable(ADC0_BASE, 3);
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 3);
	ADCIntEnable(ADC0_BASE, 3);
	
	//Setup PWM
	/*GPIOPinConfigure(GPIO_PF2_T1CCP0);
	GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_2 );
	ulPeriod = (SysCtlClockGet() / 500)/2;
	//TimerConfigure(TIMER1_BASE,TIMER_CFG_A_PWM);
	TimerConfigure(TIMER1_BASE,(TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM));*/
	ulPeriod = (SysCtlClockGet() / 500)/2;
	GPIOPinConfigure(GPIO_PB6_T0CCP0);
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_6 );
	TimerConfigure(TIMER0_BASE,(TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM));

	//enabling interupts
	IntMasterEnable();
	ADCIntClear(ADC0_BASE,3);
	ADCIntRegister(ADC0_BASE,3,ADC0IntHandler);
	ADCSequenceEnable(ADC0_BASE, 3);
	ADCIntEnable(ADC0_BASE, 3);

	while(1){
		ADCProcessorTrigger(ADC0_BASE, 3);//trigger
		SysCtlDelay(5000000);//delay
	}
}
