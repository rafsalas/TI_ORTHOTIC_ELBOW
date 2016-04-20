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
 *	|3MHz  |3MHz   |32KHz |2MHz    |1000Hz  |
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
#include "Calibrate.h"

//-----------------------------------------------Variables

///////////////////////////
// MAIN ROUTINE
///////////////////////////

// GLOBAL DEFINITIONS
const uint8_t BICEPS = 0; // Biceps Electrode Number
const uint8_t TRICEPS = 1; // Triceps Electrode Number
const uint8_t FOREARM_I = 2; // Inner Forearm Electrode Number
const uint8_t FOREARM_O = 3; // Outer Forearm Electrode Number

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
uint16_t Calibration_History = 50;

// SENSORS
// FSRs
int16_t FSR1_ADC_Threshold = 12000; // Force Sensitive Resistor 1 ADC Maximum Value
int16_t FSR2_ADC_Threshold = 12000; // Force Sensitive Resistor 2 ADC Maximum Value


// EMG
double EMG[8][100+11-1]; // 8 Channel History (Filtered, Rectified, Averaged)
int EMG_History = 100+11-1; // EMG Data History

// NORMALIZATION ROUTINE
double EMG_max[8]; // Maximum EMG Signal
double EMG_min[8];// = {100, 100, 100, 100, 100, 100, 100, 100}; // Minimum EMG Signal
double EMG_min_i[8];// = {51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51}; // Minimum EMG Signal Index

// ANGLE
double ANGLE_deg[50]; // 50 Samples of Angle History
int ANGLE_History = 50; // ANGLE Data History
double ANGLE_max = 170; // Maximum Permitted Angle from Calibration Routine (Default)
double ANGLE_min = 10; // Minimum Permitted Angle from Calibration Routine (Default)
int8_t Direction_flag= 0; // + if Elbow Opening, - if Elbow Closing
double ANGLE_damp = 0; // Dampening Coefficient


// MOTOR
double MOTOR[50]; // 50 Samples of Motor Control History
int MOTOR_History = 50; // Motor Data History
double Upper_Arm_Intention=0;
double Lower_Arm_Intention=0;
uint16_t PWM1 = 500;
uint16_t PWM2 = 500;
uint16_t PWM_max = 1000;

//////
// END
//////

//-----------------------------------------------ADC
uint16_t resultsBuffer[4];// used for ADC
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



void main(void)
{
	int i, j;
	double sum;
	MAP_WDT_A_holdTimer();

	// INITIALIZATION

		// SPI SETUP (ADS1299)
			raise_clk_rate(); // 48 MHz
			spi_setup(); // Setup SPI Communication
			spi_start(); // Setup ADS1299 Registers
			drdy_setup(); // Setup
			lower_clk_rate(); // 12 MHz


		// ADC SETUP (Potentiometers and Force Sensitive Resistors)
			setup_adc();


		// MOTOR SETUP (High Torque DC Motors)
			PWM1=100; // Initialize Motor Speed to 10%
			PWM2=100; // Initialize Motor Speed to 10%
			Direction_flag=1; // Initialize Motors to Open Direction
			setup_Motor_Driver();


		// PUSH BUTTON SETUP (Direction Change)
		    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);//push buttons pin1 push = toggle direction flag
		    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);//pin 4 push does nothing
		    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
		    MAP_Interrupt_enableInterrupt(INT_PORT1);
		    // Enabling MASTER interrupts
		    MAP_Interrupt_enableMaster();


		// UART (Bluetooth)
			lower_clk_rate();
			uart_setup();


		// Calibration Routine
		    while(Cal_Request!=1);
			lower_clk_rate();
			calibration();

		__delay_cycles(5000000);


	// DEMO ROUTINE (WITH EMG ONLY)
	// SHOW EMG OUTPUT AND PROCESSING GRAPHS
	while(0)
	{
		__delay_cycles(10000);


		// RAISE CLOCK RATE (12 MHz -> 48 MHz)
		raise_clk_rate();

		// ENABLE DRDY INTERRUPT
		MAP_Interrupt_enableInterrupt(INT_TA3_N);

		// COLLECT SPI DATA
		SPI_Collect_Data();

		// DISABLE DRDY INTERRUPT
		MAP_Interrupt_disableInterrupt(INT_TA3_N);

		// CONDITION DATA
		EMG_Condition_Data();

		// COMPARATOR
		Comparator();

		// LOWER CLOCK RATE (48 MHz -> 12 MHz)
		//lower_clk_rate();
	}


	// DEMO ROUTINE (WITHOUT EMG)
	// OSCILLATE BETWEEN DYNAMIC ANGLE LIMITS
	while(0)
	{

		__delay_cycles(100000);

		// READ DYNAMIC ANGLE LIMITS AND CENTER ORTHOSIS
		if(Cal_Request==1){
			calibration();
		}
		// READ ANGLE FROM POTENTIOMETERS
		read_adc(resultsBuffer);
		ANGLE_deg[0] = resultsBuffer[0];
		d = ANGLE_deg[0];

		// EMERGENCY STOP WITH FSRs
		/*
		if((resultsBuffer[2] < FSR1_ADC_Threshold)) // PIN5.3 (FSR)
		{
			drive_stop();
			while(1);
		}
		*/

		// DAMPEN MOTOR SPEED BASED ON ANGLE
		Angle_Dampen();

		// FLIP DIRECTION AT LIMITS
		if(ANGLE_damp<0.2){
			Direction_flag=Direction_flag*(-1);
		}


		// DRIVE MOTOR
		PWM1=0.4*PWM_max*ANGLE_damp;
		PWM2=0.4*PWM_max*ANGLE_damp;
		b = PWM1;
		c = PWM2;
		drive_motor();

	}

	// TEST LOOP
	while(0){
		// Check for Calibration Signal
		if(Cal_Request==1)	calibration();

		__delay_cycles(100000);

		// Clock Test
		clk=CS_getSMCLK();
		aux=CS_getACLK();

		/*
		raise_clk_rate();
		MAP_Interrupt_enableInterrupt(INT_TA3_N);
		SPI_Collect_Data();
		MAP_Interrupt_disableInterrupt(INT_TA3_N);
		EMG_Condition_Data();
		lower_clk_rate();
		*/

		// Comparator Test
		//Comparator();

		read_adc(resultsBuffer);
		ANGLE_deg[0] = resultsBuffer[0];//0.5*(resultsBuffer[0]+resultsBuffer[1]); // PIN5.5 + PIN5.4 (Potentiometers)


		// ANGLE DAMPEN COEFFICIENT
		Angle_Dampen();



		//Direction_flag=1;
		PWM1=100*ANGLE_damp;
		PWM2=100*ANGLE_damp;
		drive_motor();


	}




	// DEMO ROUTINE (FULL INTEGRATION)
	// SHOW EMG CONTROLLING MOTOR WITH POTENTIOMETER FEEDBACK
    while(1){
		///////////////////////////////////////////////////////////////////////
		// Calibration Routine
		///////////////////////////////////////////////////////////////////////
		// Read Angle Limits from Bluetooth Stream, Start EMG Calibration
		// INPUT: Bluetooth Interrupt
		// INTERNAL:
		// OUTPUT: ANGLE_max, ANGLE_min, EMG_max, EMG_min
		///////////////////////////////////////////////////////////////////////
			if(Cal_Request==1)	calibration();


		///////////////////////////////////////////////////////////////////////
		// SPI Read
		///////////////////////////////////////////////////////////////////////
		// Read from ADS1299 through SPI Channel
		// INPUT: 216 Bit Stream, 9 24 Bit Channels
		// INTERNAL: MSB|MID|LSB -> 2's Complement
		// OUTPUT: ADC Data
		///////////////////////////////////////////////////////////////////////
		    raise_clk_rate();
			MAP_Interrupt_enableInterrupt(INT_TA3_N);
			MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
			SPI_Collect_Data();
			MAP_Interrupt_disableInterrupt(INT_TA3_N);
			MAP_Interrupt_disableInterrupt(INT_EUSCIA2);

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


		///////////////////////////////////////////////////////////////////////
		// Direction Comparator
		///////////////////////////////////////////////////////////////////////
		// Multiplexes Counteracting EMG Signals
		// INPUT:
		//     -- EMG[BICEPS][0], EMG[TRICEPS][0]
		// INTERFACES:
		// OUTPUT: EMG Magnitude and Direction Coefficients
		//     -- Upper_Arm_Intention (0,1), Direction_flag (-1 or 1)
		///////////////////////////////////////////////////////////////////////
			Comparator();
			lower_clk_rate();

		///////////////////////////////////////////////////////////////////////
		// Read Potentiometer and FSR
		///////////////////////////////////////////////////////////////////////
		// Reads ADC Value for Potentiometer and FSR
		// INPUT: ADC Data (14 Bit)
		// INTERNAL:
		// OUTPUT: Angle, Pressure
		//      -- ANGLE_deg[0]
		///////////////////////////////////////////////////////////////////////

		// FSR Threshold (EMERGENCY STOP)
			// TWO FSRs
			//if((resultsBuffer[2] < FSR1_ADC_Threshold) || (resultsBuffer[3] < FSR2_ADC_Threshold)) // PIN5.3 + PIN5.2 (FSR)
			if((resultsBuffer[2] < FSR1_ADC_Threshold)) // PIN5.3 (FSR)
			//if((resultsBuffer[3] < FSR1_ADC_Threshold)) // PIN5.2 (FSR)
			{
				drive_stop();
				while(1);
			}

		// POTENTIOMETER (Angle State)
			read_adc(resultsBuffer);
			ANGLE_deg[0] = 0.5*(resultsBuffer[0]+resultsBuffer[1]); // PIN5.5 + PIN5.4 (Potentiometers)

		///////////////////////////////////////////////////////////////////////
		// Angle Limit Dampening
		///////////////////////////////////////////////////////////////////////
		// Dampens EMG Coefficient based on Current Angle State
		// INPUT: Potentiometer Angle (0*,180*)
		//     -- ANGLE_deg[0], ANGLE_max, ANGLE_min, ANGLE_dir
		// INTERNAL: Angle, Direction -> Transfer Function Lookup Table
		// OUTPUT: Damper Coefficient
		//      -- ANGLE_damp
		///////////////////////////////////////////////////////////////////////
			Angle_Dampen();


		///////////////////////////////////////////////////////////////////////
		// Motor Logic
		///////////////////////////////////////////////////////////////////////
		// Scales Maximum PWM Signal Based on State, Intention, Interrupts
		// INPUT:
		//     -- PWM_max, Upper_Arm_Intention, ANGLE_damp
		// INTERNAL:
		// OUTPUT:
		//      --
		///////////////////////////////////////////////////////////////////////
			MOTOR[0]=0.3*PWM_max*Upper_Arm_Intention*ANGLE_damp;

			PWM1=MOTOR[0];
			PWM2=MOTOR[0];
			drive_motor();


		// HISTORY BUFFER (QUEUE)
			// EMG History Buffer
			//for(i=0;j<8;j++) for(j=EMG_History-1;j>0;j--) EMG[i][j]=EMG[i][j-1];
			// ANGLE History Buffer
			for(i=ANGLE_History-1;i>0;i--) ANGLE_deg[i]=ANGLE_deg[i-1];
			// MOTOR History Buffer
			for(i=MOTOR_History-1;j>0;j--) MOTOR[i][j]=MOTOR[i][j-1];
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
    	//lower_clk_rate();
    	Direction_flag=Direction_flag*(-1);
		__delay_cycles(1000);


    }else{
    	;
    	//raise_clk_rate();
    }




}


