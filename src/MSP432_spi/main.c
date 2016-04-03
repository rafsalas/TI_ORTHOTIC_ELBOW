/***************************************************************************************
 * MAIN
 *
 *  Created on: Dec, 2015
 *      Author: rafael
***************************************************************************************/
//1.1,1.2,1.3,1.5,1.6,1.7
//2.4,2.5
//3.2,3.3,3.5
//4.0,4.1,4.2,4.5,4.6
//5.0,5.1,5.2,5.3,5.4,5.5
//7.5,7.7
/*
*clock_table
*|MCLK  |SMCLK  |ACLK  |SPICLK  |TIMER3  |
*|3MHz  |3MHz   |32KHz |1MHz    |1000Hz  |
*/
#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <QMathLib.h>
#include "printf.h"
#include <string.h>
#include "SPI_COMMS.h"
#include "UART_COMMS.h"
#include "drv8.h"
#include "ADC_Sensors.h"

//-----------------------------------------------Variables

///////////////////////////
// MAIN ROUTINE
///////////////////////////
uint8_t Main_Routine_Rate_Flag = 0x00; // Flag for Main Routine Interrupt


///////////////////////////
// BODY-TO-SENSOR INTERFACE
///////////////////////////
// SPI

uint8_t Drdy = 0x00; //Flag for DRDY on SPI Channel
uint8_t SPI_Cleared = 1; // Flag to Wait Until SPI Channel Clears
uint8_t SPI_Connected = 0; // Flag to Wait Until SPI Initialiation Complete
uint8_t Cal_Request = 0;
// EMG
double EMG[8][50]; // 8 Channel History (Filtered, Rectified, Averaged)

// NORMALIZATION ROUTINE
double EMG_max[8]; // Maximum EMG Signal
double EMG_min[8];// = {100, 100, 100, 100, 100, 100, 100, 100}; // Minimum EMG Signal
double EMG_min_i[8];// = {51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51}; // Minimum EMG Signal Index

//////
// END
//////

//-----------------------------------------------ADC
static uint16_t resultsBuffer[4];// used for ADC
volatile uint16_t a,b,c,d =0; //used for adc testing

void timersetup(){

	const Timer_A_ContinuousModeConfig continuousModeConfig =
	{
	        TIMER_A_CLOCKSOURCE_ACLK,           // ACLK Clock Source
	        TIMER_A_CLOCKSOURCE_DIVIDER_32,      // ACLK/4 = 250
	        TIMER_A_TAIE_INTERRUPT_ENABLE,      // Enable Overflow ISR
	        TIMER_A_DO_CLEAR                    // Clear Counter
	};

    /* Configuring P1.0 as output */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Starting and enabling ACLK (128 kHz) */
    MAP_CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);

    /* ACLK Divided (128/4 kHz) */
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_4);//aux clk div

    /* Configuring Continuous Mode */
    MAP_Timer_A_configureContinuousMode(TIMER_A3_MODULE, &continuousModeConfig);

    /* Enabling interrupts and going to sleep */
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableInterrupt(INT_TA3_N);
    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
    /* Starting the Timer_A0 in continuous mode */
    MAP_Timer_A_startCounter(TIMER_A3_MODULE, TIMER_A_CONTINUOUS_MODE);
}

volatile uint32_t clk = 0;
volatile uint32_t aux = 0;

void main(void)
{
	MAP_WDT_A_holdTimer();

	// INITIALIZATION


		// BLUETOOTH ROUTINE
			//Bluetooth
			//Bluetooth


		// SPI SETUP
			//spi_setup();
			//spi_start();
			//drdy_setup();

		// UART
			uart_setup();

		// MOTOR SETUP
			//setup_Motor_Driver();

	/*while(1){
		__delay_cycles(100);


		MAP_Interrupt_enableInterrupt(INT_TA3_N);

		SPI_Collect_Data();

		MAP_Interrupt_disableInterrupt(INT_TA3_N);

		EMG_Condition_Data();



	}*/


	// LOOP
	setup_adc();
    while(1){

    	/*_delay_cycles(10000);
    	clk = CS_getMCLK();
    	aux = CS_getSMCLK();
    	read_adc(resultsBuffer);
        a = resultsBuffer[0];
        b = resultsBuffer[1];
        c = resultsBuffer[2];
        d = resultsBuffer[3];*/

/*
		// SPI Read
		SPI_Collect_Data();
		// Condition EMG Data
		EMG_Condition_Data();

		//Read Pot(output angle value

		//Normalize pot coefficient

		//direction comparator(outputs direction coefficient)

		//Read FSR(get adc value)

		//threshold determination

		//Motor coeficient multiplication(also check calibration values here)

		//actuate motor
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
        printf(EUSCI_A2_MODULE,"result1: %i result1: %i",a,b );
    }
}

void SPI_DATA_RATE_ISR(void)
{

//used later for power consumption

}




