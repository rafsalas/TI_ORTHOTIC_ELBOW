/***************************************************************************************
 * MAIN
 *
 *  Created on: Dec, 2015
 *      Author: rafael
***************************************************************************************/
//1.1,1.2,1.3,1.5,1.6,1.7
//2.4,2.5
//3.5
//4.0,4.1,4.2,4.5,4.6
//5.0,5.1,5.4,5.5
//7.5,7.7
#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <QMathLib.h>
#include "printf.h"
#include "RadialEncoder.h"
#include <string.h>
#include "SPI_COMMS.h"
#include "UART_COMMS.h"
#include "drv8.h"

//-----------------------------------------------Variables

uint8_t Drdy = 0x00; //Flag for SPI
uint8_t sample_rdy=0x00; //Flag when window amount of samples read.
uint8_t spi_data[50][24];
volatile uint16_t x = 0;

static uint16_t resultsBuffer[2];// used for ADC
volatile uint16_t a,b =0; //used for adc testing
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

void main(void)
{
	MAP_WDT_A_holdTimer();
	//drdy_setup();
	//adc();
	//uart_setup();
	//encoderInit();
	spi_setup();
	//spi_start();

	// READ DATA BY COMMAND
/*	SPI_transmitData(EUSCI_B0_MODULE, 0x12); // RDATAC
	__delay_cycles(5000);

	// READ DATA CONTINUOUSLY
	SPI_transmitData(EUSCI_B0_MODULE, 0x10); // RDATAC
	__delay_cycles(5000);
*/
	//MAP_CS_initClockSignal(CS_SMCLK , CS_DCOCLK_SELECT , CS_CLOCK_DIVIDER_1);// clock source CS_ACLK, Use external clock, no clock divisions

	intiallize();
    //setup_PWM();
    drive_forward();
    //drive_reverse();

    while(1){

    x = CS_getSMCLK();
    //	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //dummy
    //	x = SPI_receiveData(EUSCI_B0_MODULE);
    	//__delay_cycles(1000000);
    //	printf(EUSCI_A0_MODULE,"hello\n");
    	//drive_stop();
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

