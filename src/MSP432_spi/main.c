/***************************************************************************************
 * MAIN
 *
 *  Created on: Dec, 2015
 *      Author: rafael
***************************************************************************************/

#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <QMathLib.h>
#include "printf.h"
#include "RadialEncoder.h"
#include <string.h>

//-----------------------------------------------Variables
const int bit_stream_length = 24; // length in bytes, data + status byte
const int msp_clk_rate = 48000;//48Mhz
const int ads_clk_rate = 2048; //2.048 MHz
uint8_t Drdy = 0x00; //Flag for SPI
uint8_t sample_rdy=0x00; //Flag when window amount of samples read.
uint8_t spi_data[50][24];
int32_t sample[50];
uint8_t spi_index = 0; // used to keep track of the first diminsion of the spi_dta matrix

const uint8_t window = 50; //moving average window
volatile int16_t samples [50]; //raw signal
volatile int16_t filtered [50]; //filtered signal
static uint32_t sum = 0; //sum for moving average

volatile int32_t raw_position = 0;
volatile int32_t pos_pulse_count = 0;
volatile int32_t neg_pulse_count = 0;

static uint16_t resultsBuffer[2];// used for ADC
volatile uint16_t a,b =0; //used for adc testing
//-----------------------------------------------Filtering

void conditionSamples(){
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
}
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
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,
            0);
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
//-----------------------------------------------SPI

int32_t twos_to_signed (uint32_t msb, uint32_t mid, uint32_t lsb){
	uint32_t num = (msb<<24)|(mid<<16)|(lsb<<8);// concatonate bytes
    int32_t num2;
    if(((int)num)<0){
            num2=-((~num)+1); //2's complement
    }
    else{
            num2 = num;
    }
    num2 = num2>>2; //shift to correct for 2 bit offset needed for 2's complement
    return num2;
}


void spi_setup(){
/* SPI Master Configuration Parameter */
	const eUSCI_SPI_MasterConfig spiMasterConfig =
	{
        EUSCI_B_SPI_CLOCKSOURCE_ACLK,              // ACLK Clock Source
        32768,                                     // ACLK = LFXT = 32.768khz
        500000,                                    // SPICLK = 500khz (110k)
        EUSCI_B_SPI_MSB_FIRST,                     // MSB First
        EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,    // Phaseb
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW, // low polarity
        EUSCI_B_SPI_3PIN                           // 3Wire SPI Mode
	};

    /* Starting and enabling LFXT (32kHz) */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_CS_setExternalClockSourceFrequency(32768, 0);//32.768 KHz
	MAP_CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);// clock source CS_ACLK, Use external clock, no clock divisions
	MAP_CS_startLFXT(CS_LFXT_DRIVE0); //LFXTDRIVE_0 give the lowest current consumption according to the msp432 header file for this board

	//selecting pins for spi mode
	//pin 1 = clk  pin 6 = mosi pin 7 =miso
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1 , GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7,
			GPIO_PRIMARY_MODULE_FUNCTION );

	//configuring SPI for 3wire master mode
	MAP_SPI_initMaster(EUSCI_B0_MODULE,&spiMasterConfig);//EUSCI_B0_MODULE = Base address of module registers
	MAP_SPI_enableModule(EUSCI_B0_MODULE);
    Interrupt_enableInterrupt(INT_EUSCIB0);
    Interrupt_enableSleepOnIsrExit();

   /* Delaying waiting for the module to initialize */
    __delay_cycles(500);

}

void read_message(){
	unsigned int cycle = (msp_clk_rate/ads_clk_rate)+1; // cycle ratio needed for delays between reading bytes
	unsigned int delay = 4*cycle;
	unsigned int iterator = 0;
	unsigned int iter = 0;
	uint32_t container[3];

	if(spi_index == window){//condition sample and reset  spi_index
		conditionSamples();
		spi_index = 0;
	}
	uint8_t it = 0;
	for(it=0;it<3;++it){
		SPI_receiveData(EUSCI_B0_MODULE); //read status byte and throw away
	}
	for(iterator = 0;iterator < bit_stream_length;++iterator){

			for(iter = 0; iter<delay ;++iter){}// delay for 4 cycles
			container[iterator%3] = SPI_receiveData(EUSCI_B0_MODULE); //read a byte
			if(iterator%3 == 2){
				sample[spi_index]= twos_to_signed (container[0],container[1],container[2]);// convert twos complement and store in buffer
				printf(EUSCI_A0_MODULE, "value: %i \r\n", sample[spi_index]);
			}
	}
	/*if(spi_index==49){
		sample_rdy = 0x01;
	}*/
	Drdy = 0x01;
	spi_index++; //increment first dimension of spi_data index
}

void spi_start(){
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x06); //RESET
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x06); //RESET
	__delay_cycles(100000); //Wait 150ms

	// Power Up sequencing - Single ended Input
	SPI_transmitData(EUSCI_B0_MODULE, 0x11); //SDATAC
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x43); //CONFIG3 address
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x6C); //Register data
	__delay_cycles(960);
	__delay_cycles(100000);

	SPI_transmitData(EUSCI_B0_MODULE, 0x41); //CONFIG1 address
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00);// Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x96); //Register data - 500Hz data rate
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE, 0x42); //CONFIG2 address
//	 __delay_cycles(960);
	 __delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0xC0); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE, 0x45); //CH1SET address
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x07); //Write to 8 registers
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data - normal electrode
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE, 0x55); //MISC1 address is 15h
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x20); //Register data - Set SRB1
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE,0x44); //LOFF address
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE,0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE,0x06); //Register data - 24nA, 31.2Hz
//	__delay_cycles(960);
	__delay_cycles(10000); //Wait 150ms
	__delay_cycles(1920);
	SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
//	__delay_cycles(960);
	__delay_cycles(10000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x10); // read data cont
	__delay_cycles(10000);
}

//-----------------------------------------------Drdy
void drdy_setup(){
    /* Configuring P3.5 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN5);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN5);
    MAP_Interrupt_enableInterrupt(INT_PORT3);

    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
}

//-----------------------------------------------Uart
void uart_setup(){
	const eUSCI_UART_Config uartConfig =
	{
	        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
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


volatile int css = 0;
void main(void)
{
	MAP_WDT_A_holdTimer();
	drdy_setup();
	adc();
	uart_setup();
	encoderInit();
	spi_setup();
	spi_start();
    while(1){}
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
	uint8_t val[3];
	val[0] = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0);
	val[1] = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN1);
	val[2] = GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2);
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, status);

  //  printf(EUSCI_A0_MODULE,"gpio1 %i", GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0));

    printf(EUSCI_A0_MODULE,"\r\n\r\n");
    if(val[0]==val[1]==val[2] ){ //one step CW
		printf(EUSCI_A0_MODULE,"in neg\r\n");
		raw_position++;
	}else{ //one step CCW
		printf(EUSCI_A0_MODULE,"in pos\r\n");
		//temp_pos++;
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
