#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
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
/*
 * main.c
 */

#ifdef DEBUG
void__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif
unsigned long ulADC0Value[1];

void ADC0IntHandler(void){

	ADCSequenceDataGet(ADC0_BASE, 3, ulADC0Value);
	UARTprintf("ADC VALUE: %d\r\n", ulADC0Value[0]);
	ADCIntClear(ADC0_BASE,3);
}




int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);


    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    GPIOPinTypeUART(GPIO_PORTA_BASE,  GPIO_PIN_0 | GPIO_PIN_1);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);


	UARTprintf("\n1Ready\n\n");


    GPIOPinTypeADC (GPIO_PORTD_BASE, GPIO_PIN_0);


	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);

	ADCSequenceDisable(ADC0_BASE, 3);
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 3);
	ADCIntEnable(ADC0_BASE, 3);

	IntMasterEnable();
	ADCIntClear(ADC0_BASE,3);
	ADCIntRegister(ADC0_BASE,3,ADC0IntHandler);
	ADCSequenceEnable(ADC0_BASE, 3);
	ADCIntEnable(ADC0_BASE, 3);

	while(1){
		ADCProcessorTrigger(ADC0_BASE, 3);
		SysCtlDelay(7000000);
	}
}
