/*
 * UART_COMMS.c
 *
 *  Created on: Feb 20, 2016
 *      Author: rafael
 */
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "UART_COMMS.h"
#include <stdint.h>

volatile uint8_t RXData = 0;
volatile uint8_t RxBuff[10];
volatile uint8_t RxBuffSize = 0;
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

    /* Selecting P3.2 and P3.3 in UART mode and P1.0 as output (LED) p2.2 unexpected led */

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
                GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);

    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_3);//set clk to 3M

    MAP_UART_initModule(EUSCI_A2_MODULE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A2_MODULE);
    MAP_UART_enableInterrupt(EUSCI_A2_MODULE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
    MAP_Interrupt_enableMaster();
}



int *read_cal_angles(){
	char low[3];
	char high[3];
	if(RxBuffSize == 6){
		int i =0;
		for(;i<3;++i){
			low[i] = RxBuff[i];
			high[i] = RxBuff[i+3];
		}
	}else{
		//abort
		return 0;
	}
	int *angles= (int *) malloc(2);
	sscanf(low, "%d", &angles[0]);
	sscanf(high, "%d", &angles[1]);
	return angles;
}

void euscia2_isr(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_MODULE);
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_UART_clearInterruptFlag(EUSCI_A2_MODULE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT)
    {
        RXData = MAP_UART_receiveData(EUSCI_A2_MODULE);
        uint8_t itt;
        switch(RXData){

        	case 'a':
        		//memset(RxBuff, 0, 10 );      //clear buffer for limits
        		for(itt = 0;itt<10;itt= itt+1){
        			RxBuff[itt] = 0;
        		}
        		RxBuffSize = 0; //reset iterator
        		Cal_Request = 1; //signal that a calibration is requested and wait for incoming message
        		GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        		MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
        		break;
        	case 'q':
        		Read_flag = 1;//message is complete ready to read
        		MAP_UART_transmitData(EUSCI_A2_MODULE, 'r');//send recieve bit
        		GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        		break;
        	case 's':
        		//submit flex
        		break;
        	default:
        		if(isalnum(RXData)){//load buffer with angle limits
                	RxBuff[RxBuffSize]= RXData;
                	RxBuffSize++;
        		}
        		else{
        			MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
        		}
        		GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        }
            /*    if(RXData != 'a')              // Check value
               {
                   MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                   MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

                   //while(1);                       // Trap CPU
               }else{
            	   MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
            	   MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
               }*/


    }

}
