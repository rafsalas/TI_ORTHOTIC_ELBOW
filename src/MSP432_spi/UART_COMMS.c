/*
 * UART_COMMS.c
 *
 *  Created on: Feb 20, 2016
 *      Author: rafael
 */

#include "UART_COMMS.h"
volatile uint8_t RXData = 0;
volatile uint8_t rx= 0;
void uart_setup(){
	const eUSCI_UART_Config uartConfig =
	{
	        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
	        19,                                     // BRDIV = 26
	        8,                                       // UCxBRF = 0
	        85,                                       // UCxBRS = 111
	        EUSCI_A_UART_NO_PARITY,                  // No Parity
	        EUSCI_A_UART_LSB_FIRST,                  // LSB First
	        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
	        EUSCI_A_UART_MODE,                       // UART mode
	        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
	};

    /* Selecting P1.2 and P1.3 in UART mode and P1.0 as output (LED) */
    //MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
                GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);


    /* Setting DCO to 48MHz (upping Vcore) */
//    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
//    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    //CS_setExternalClockSourceFrequency
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_3);
    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A2_MODULE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A2_MODULE);

    MAP_UART_enableInterrupt(EUSCI_A2_MODULE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);

    MAP_Interrupt_enableMaster();
}

void uart_setup2(){
	const eUSCI_UART_Config uartConfig =
	{
	    EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
	    78,                                      // BRDIV = 78
	    2,                                       // UCxBRF = 2
	    0,                                       // UCxBRS = 0
	    EUSCI_A_UART_NO_PARITY,                  // No Parity
	    EUSCI_A_UART_LSB_FIRST,                  // MSB First
	    EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
	    EUSCI_A_UART_MODE,                       // UART mode
	    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
	};

    /* Configure pins P3.2 and P3.3 in UART mode.
     * Doesn't matter if they are set to inputs or outputs
     */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
                GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Setting DCO (clock) to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A2_MODULE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A2_MODULE);


}

/* EUSCI A0 UART ISR - Echos data back to PC host */
void euscia2_isr(void)
{
	rx = 1;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_MODULE);

    MAP_UART_clearInterruptFlag(EUSCI_A2_MODULE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT)
    {
        RXData = MAP_UART_receiveData(EUSCI_A2_MODULE);
        if(RXData != 'a')              // Check value
        {
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            while(1);                       // Trap CPU
        }
      	//	  RXData = 0;
        MAP_UART_transmitData(EUSCI_A2_MODULE, 'r');
        Cal_Request = 1;
        //TXData++;
        MAP_Interrupt_disableSleepOnIsrExit();
    }

}
