/***************************************************************************************
 * MAIN
 *
 *  Created on: Dec, 2015
 *      Author: rafael
***************************************************************************************/
//1.1,1.2,1.3,1.5,1.6,1.7
//3.5
//4.0,4.1,4.2,
//5.4,5.5

#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <QMathLib.h>
#include "printf.h"
#include "RadialEncoder.h"
#include <string.h>
#include "SPI_COMMS.h"
//-----------------------------------------------Variables

uint8_t Drdy = 0x00; //Flag for SPI
uint8_t sample_rdy=0x00; //Flag when window amount of samples read.
uint8_t spi_data[50][24];


volatile uint16_t x = 0;

volatile int32_t raw_position = 0;
volatile int32_t pos_pulse_count = 0;
volatile int32_t neg_pulse_count = 0;

static uint16_t resultsBuffer[2];// used for ADC
volatile uint16_t a,b =0; //used for adc testing
//-----------------------------------------------Filtering

/*void conditionSamples(){
	uint8_t i = 0;
	for(i = 0; i < window;++i){
		samples[i] = _Q1abs(samples[i]);//rectify
		//moving average
		sum = sum + samples[i];
		if(i>window){
			sum = sum - samples[i-window];
			filtered[i] = sum/window;
		}
		else if(i = window){
			filtered[i] = sum/window;
		}
		else{
			filtered[i] = 0;
		}
		MAP_UART_transmitData(EUSCI_A0_MODULE, filtered[i]);
	}
}*/
//-----------------------------------------------ADC

void adc(){

    MAP_PCM_setPowerState(PCM_AM_LDO_VCORE1);
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);

    /* Zero-filling buffer */
    memset(resultsBuffer, 0x00, 2);

    /* Setting reference voltage to 2.5  and enabling reference */
    MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();

    /* Initializing ADC (MCLK/1/1) */
    MAP_ADC14_enableModule();
    //MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,  0);
    /* Configuring GPIOs for Analog In */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,
            GPIO_PIN5 | GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM7 (A0 - A1)  with no repeat)
     * with internal 2.5v reference */

    MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A0, false);
    MAP_ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A1, false);

    /* Enabling the interrupt when a conversion on channel 7 (end of sequence)
     *  is complete and enabling conversions */
    MAP_ADC14_enableInterrupt(ADC_INT1);

    /* Enabling Interrupts */
    MAP_Interrupt_enableInterrupt(INT_ADC14);
    MAP_Interrupt_enableMaster();

    /* Setting up the sample timer to automatically step through the sequence convert.*/
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Triggering the start of the sample */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();

}



//-----------------------------------------------Uart
void uart_setup(){
	const eUSCI_UART_Config uartConfig =
	{
	        EUSCI_A_UART_CLOCKSOURCE_ACLK,          // SMCLK Clock Source
	        26,                                     // BRDIV = 26
	        0,                                       // UCxBRF = 0
	        111,                                       // UCxBRS = 111
	        EUSCI_A_UART_NO_PARITY,                  // No Parity
	        EUSCI_A_UART_LSB_FIRST,                  // LSB First
	        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
	        EUSCI_A_UART_MODE,                       // UART mode
	        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
	};

    /* Selecting P1.2 and P1.3 in UART mode and P1.0 as output (LED) */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    //MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Setting DCO to 48MHz (upping Vcore) */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    //CS_setExternalClockSourceFrequency
    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_MODULE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_MODULE);
    MAP_Interrupt_enableMaster();
}

void main(void)

{
	MAP_WDT_A_holdTimer();
	drdy_setup();
	//adc();
//	uart_setup();
	//encoderInit();
	spi_setup();
	spi_start();

	// READ DATA BY COMMAND
	SPI_transmitData(EUSCI_B0_MODULE, 0x12); // RDATAC
	__delay_cycles(500000);

	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //dummy
	x = SPI_receiveData(EUSCI_B0_MODULE);


    while(1){

    	//SPI_transmitData(EUSCI_B0_MODULE, 0x00); //dummy
    	//x = SPI_receiveData(EUSCI_B0_MODULE);


    	// WHEN DRDY TRANSITIONS
    /*

    SPI_transmitData(__MSP430_BASEADDRESS_USCI_B1__, 0x12); //RDATA
    __delay_cycles(100)

    int j;
    for(j=0; j<(NUM_CHANNELS * 3 + NUM_BOARDS*3); j++)
    {
        SPI_transmitData(__MSP430_BASEADDRESS_USCI_B1__, 0x00); //Dummy data
        __delay_cycles(100);
        spiData[j] = SPI_receiveData(__MSP430_BASEADDRESS_USCI_B1__);
        if((j < 3) || ((j > 26) && (j <= 29))); // Skip status bytes of the 2 ADS1299 data
        else sendDataByte(spiData[j]);
    }
    */


    }
}

//-----------------------------------------------Interrupts

/* This interrupt is fired whenever a conversion is completed and placed in
 * ADC_MEM1. This signals the end of conversion and the results array is
 * grabbed and placed in resultsBuffer */

void adc_isr(void)
{
    uint64_t status;
    status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    /*  if(status & ADC_INT0)
    {
        a = ADC14_getResult(ADC_MEM0);
    }*/
    if(status & ADC_INT1)
    {
        MAP_ADC14_getMultiSequenceResult(resultsBuffer);
        a = resultsBuffer[0];
        b = resultsBuffer[1];
        printf(EUSCI_A0_MODULE,"result1: %i result1: %i",a,b );
    }
}

void gpio_isr4(void)
{
//	uint32_t status;
	uint8_t val[3];
	val[0] = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0);
	val[1] = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN1);
	val[2] = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2);

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

void gpio_isr3(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P3, status);

    /* set Drdy Flag*/
    if(status & GPIO_PIN5)
    {
    //	Drdy = 0x01;
     //   MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //    read_message();
    }
}
