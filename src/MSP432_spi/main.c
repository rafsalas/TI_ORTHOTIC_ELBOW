/*
 * main.c
 *
 * 	Elbow Orthosis
 * 	Texas A&M University & Texas Instruments
 *
 *  Created on: Fall 2015
 *      Author: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas
 *
 * Pins used:
 *	1.1,1.2,1.3,1.5,1.6,1.7
 *	2.0,2.2,2.4,2.5
 *	3.2,3.3,3.5
 *	4.0,4.1,4.2,4.5,4.6
 *	5.0,5.1,5.2,5.3,5.4,5.5
 *	7.5,7.7
 *
 * Clock table (normal values)
 *	|MCLK  |SMCLK  |ACLK  |SPICLK  |TIMER3  |
 *	|3MHz  |3MHz   |32KHz |1MHz    |1000Hz  |
 *
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
#include "RadialEncoder.h"

//-----------------------------------------------Variables

///////////////////////////
// MAIN ROUTINE
///////////////////////////

// SPI     -> EMG Voltage -> EMG Coefficient
// ADC POT -> Angle       -> Dampen Coefficient
// ADC FSR -> Pressure    -> Safty Threshold Emergency Stop
// MOTOR   -> PWM Duty Cycle * (EMG) * (DAMP)

uint8_t Main_Routine_Rate_Flag = 0x00; // Flag for Main Routine Interrupt


// SPI

uint8_t Drdy = 0x00; //Flag for DRDY on SPI Channel
uint8_t SPI_Cleared = 1; // Flag to Wait Until SPI Channel Clears
uint8_t SPI_Connected = 0; // Flag to Wait Until SPI Initialiation Complete
uint8_t Cal_Request = 0;
uint8_t Read_flag = 0;
uint16_t Calibration_History = 100;

// EMG
double EMG[8][50]; // 8 Channel History (Filtered, Rectified, Averaged)

// NORMALIZATION ROUTINE
double EMG_max[8]; // Maximum EMG Signal
double EMG_min[8];// = {100, 100, 100, 100, 100, 100, 100, 100}; // Minimum EMG Signal
double EMG_min_i[8];// = {51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51}; // Minimum EMG Signal Index

// ANGLE
double ANGLE_deg[50]; // 50 Samples of Angle History
double ANGLE_max; // Maximum Permitted Angle from Calibration Routine
double ANGLE_min; // Minimum Permitted Angle from Calibration Routine
int8_t Direction_flag= 0; // + if Elbow Opening, - if Elbow Closing
double ANGLE_damp; // Dampening Coefficient
double ANGLE_ave = 0;


// MOTOR
double MOTOR[50]; // 50 Samples of Motor Control History
double Upper_Arm_Intention=0;
double Lower_Arm_Intention=0;
uint16_t PWM1 = 500;
uint16_t PWM2 = 500;
//////
// END
//////

//-----------------------------------------------ADC
static uint16_t resultsBuffer[4];// used for ADC
volatile uint16_t a,b,c,d =0; //used for adc testing
volatile double clk = 0;
volatile int32_t aux = 0;

void raise_clk_rate(){//Used to change to high speed calculation crunch
	MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
	CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
}

void lower_clk_rate(){//Used to conserve energy
	CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_3);
	MAP_PCM_setCoreVoltageLevel(PCM_VCORE0);
}

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

void motor_test_setup(){
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);//push buttons pin1 push = toggle direction flag
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);//pin 4 push does nothing
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
    MAP_Interrupt_enableInterrupt(INT_PORT1);
    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
	Direction_flag = 1;
	Upper_Arm_Intention = 0.5;
	ANGLE_min = 0;
	ANGLE_max = 180;
	PWM1=1000;//*Upper_Arm_Intention;//ANGLE_damp;
	PWM2=1000;//*Upper_Arm_Intention;//ANGLE_damp;
	setup_Motor_Driver();

}

void motor_test(){//this goes in the loop

	//read_adc(resultsBuffer);
	//a = resultsBuffer[0];
	//ANGLE_deg[0] = resultsBuffer[0];
	//Angle_Dampen();
	//PWM1=500;//*ANGLE_damp;
	//PWM2=500;//*ANGLE_damp;
	drive_motor();
	__delay_cycles(100000);
	aux = Direction_flag;
}

void main(void)
{
	int i, j;
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

		// ADC
			//setup_adc();

		// MOTOR SETUP
			//setup_Motor_Driver();


	/*while(1){
		__delay_cycles(100);


		MAP_Interrupt_enableInterrupt(INT_TA3_N);

		SPI_Collect_Data();

		MAP_Interrupt_disableInterrupt(INT_TA3_N);

		EMG_Condition_Data();



	}*/

	motor_test_setup();
	drive_forward();
    while(1){
		//motor_test();
    	//aux = CS_getSMCLK();
    	clk = CS_getSMCLK();
		//MAP_Interrupt_enableInterrupt(INT_TA3_N);

		//SPI_Collect_Data();

		//MAP_Interrupt_disableInterrupt(INT_TA3_N);

		//EMG_Condition_Data();


/*
		///////////////////////////////////////////////////////////////////////
		// SPI Read
		///////////////////////////////////////////////////////////////////////
		// Read from ADS1299 through SPI Channel
		// INPUT: 216 Bit Stream, 9 24 Bit Channels
		// INTERFACES: MSB|MID|LSB -> 2's Complement
		// OUTPUT: ADC Data
		///////////////////////////////////////////////////////////////////////
		 *  raise_clk_rate();
			MAP_Interrupt_enableInterrupt(INT_TA3_N);
			SPI_Collect_Data();
			MAP_Interrupt_disableInterrupt(INT_TA3_N);

		///////////////////////////////////////////////////////////////////////
		// Condition EMG Data
		///////////////////////////////////////////////////////////////////////
		// High Pass Filter, Rectification, and Dynamic Normalization
		// INPUT: Raw ADC Data (+/- 24 Bit) (2's Complemented)
		//     -- EMG_max[CHANNEL], EMG_min[CHANNEL]
		// INTERNAL: Raw ADC -> Voltage -> Filter -> Rectify -> Average -> Normalize
		// OUTPUT: EMG Coefficient
		//     -- EMG[CHANNEL][0]
		///////////////////////////////////////////////////////////////////////
			EMG_Condition_Data();
			 lower_clk_rate();
		///////////////////////////////////////////////////////////////////////
		// Read Potentiometer and FSR
		///////////////////////////////////////////////////////////////////////
		// Reads ADC Value for Potentiometer
		// INPUT: ADC Data (14 Bit)
		// INTERNAL:
		// OUTPUT: Angle
		//      -- ANGLE_deg[0])
		///////////////////////////////////////////////////////////////////////
			read_adc(resultsBuffer);
        	ANGLE_deg[0] = resultsBuffer[0];//pin5.5, Pot
        	b = resultsBuffer[1];//pin5.4 //ALTERNATE Pot
        	c = resultsBuffer[2];//pin5.3//FSR
        	d = resultsBuffer[3];//pin5.2//FSR
        	int it;
        	for(it = 0; it< 5 ;++it){
        		ANGLE_ave = ANGLE_ave + ANGLE_deg[it];
			}
			ANGLE_ave = ANGLE_ave/5;
		///////////////////////////////////////////////////////////////////////
		// Angle Limit Dampening
		///////////////////////////////////////////////////////////////////////
		// Dampens EMG Coefficient based on Current Angle State
		// INPUT: Potentiometer Angle (0*,180*)
		//     -- ANGLE_deg[0], ANGLE_max, ANGLE_min, ANGLE_dir
		// INTERFACES: Angle, Direction -> Transfer Function Lookup Table
		// OUTPUT: Damper Coefficient
		//      -- ANGLE_damp
		///////////////////////////////////////////////////////////////////////
			Angle_Dampen();

		///////////////////////////////////////////////////////////////////////
		// Direction Comparator
		///////////////////////////////////////////////////////////////////////
		// Multiplexes Counteracting EMG Signals
		// INPUT:
		//     --
		// INTERFACES:
		// OUTPUT:
		//      -- DIR_COMP
		///////////////////////////////////////////////////////////////////////
			Direction_Compare();

		//Read FSR(get adc value)

		//threshold determination

		// Coefficient Multiplication
			MOTOR[0]=PWM_max*(EMG_norm[0]*DAMP_norm[0])*(DIR_COMP);

		//actuate motor


		// HISTORY BUFFER (QUEUE)
			// EMG History Buffer
			for(i=0;j<8;j++) for(j=EMG_History-1;j>0;j--) EMG[i][j]=EMG[i][j-1];
			// ANGLE History Buffer
			for(i=ANGLE_History-1;i>0;i--) ANGLE_deg[i]=ANGLE_deg[i-1];
			// MOTOR History Buffer
			for(i=MOTOR_History-1;j>0;j--) EMG[i][j]=EMG[i][j-1];
*/
    }


}
//-----------------------------------------------Interrupts

void SPI_DATA_RATE_ISR(void)
{

//used later for power consumption

}

void gpio_isr1(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    /* Toggling the output on the LED */
    if(status & GPIO_PIN1 )
    {
    	lower_clk_rate();
    	if(Direction_flag == -1)
    		Direction_flag =1;
    	else
    		Direction_flag =-1;

    }else{
    	raise_clk_rate();
    }


}


