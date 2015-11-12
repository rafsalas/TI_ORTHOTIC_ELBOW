
//***************************************************************************************

//***************************************************************************************

#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t Drdy = 0x00;

static volatile uint16_t curADCResult;
static volatile float normalizedADCRes;

static uint16_t resultsBuffer[2];
volatile uint16_t a,b =0;

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
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1 , GPIO_PIN5 |GPIO_PIN6|GPIO_PIN7,
			GPIO_PRIMARY_MODULE_FUNCTION );

	//selecting gpio pin for CS
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN2);//set direction
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2); //set high to turn off

    //Drdy
	MAP_GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P5, GPIO_PIN1);//set direction
   // Drdy = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1);

	//configuring SPI for 3wire master mode
	MAP_SPI_initMaster(EUSCI_B0_MODULE,&spiMasterConfig);//EUSCI_B0_MODULE = Base address of module registers
	MAP_SPI_enableModule(EUSCI_B0_MODULE);

   /* Delaying waiting for the module to initialize */
    uint8_t i;
    for(i=0; i<100;i++);

}

void drdy_setup(){
    /* Configuring P1.0 as output and P1.1 (switch) as input */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Configuring P1.1 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    MAP_Interrupt_enableInterrupt(INT_PORT1);

    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
}

void uart_setup(){
	const eUSCI_UART_Config uartConfig =
	{
	        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
	        26,                                     // BRDIV = 26
	        0,                                       // UCxBRF = 0
	        111,                                       // UCxBRS = 111
	        EUSCI_A_UART_NO_PARITY,                  // No Parity
	        EUSCI_A_UART_MSB_FIRST,                  // MSB First
	        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
	        EUSCI_A_UART_MODE,                       // UART mode
	        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
	};

    /* Selecting P1.2 and P1.3 in UART mode and P1.0 as output (LED) */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Setting DCO to 48MHz (upping Vcore) */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_MODULE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_MODULE);

    //MAP_UART_transmitData(EUSCI_A0_MODULE, Data);
    //Data = MAP_UART_receiveData(EUSCI_A0_MODULE);
}

void main(void)
{
	//void Interrupt_setPriority(uint32_t interruptNumber, uint8_t priority);
	MAP_WDT_A_holdTimer();
	/* Enabling SRAM Bank Retention */
	MAP_SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);
	//spi_setup();

	drdy_setup();
	adc();

    // SPI, put CS high P5.2 and polling to see if the TX buffer is ready or busy
  /*  GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);//set low to turn on

   //Not sure I need to send a transmission
    TXData = 0x40;
    while (!(SPI_getInterruptStatus(EUSCI_B0_MODULE,EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
    SPI_transmitData(EUSCI_B0_MODULE, TXData);

    TXData = 0x00;
    while (!(SPI_getInterruptStatus(EUSCI_B0_MODULE,EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
    SPI_transmitData(EUSCI_B0_MODULE, TXData);
	*/
    //start loop
	//RXData[p][j] = SPI_receiveData(EUSCI_B0_MODULE);
    while(1){
    	MAP_PCM_gotoLPM3();
    }
}

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
    }

}

void gpio_isr(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    /* set Drdy Flag*/
    if(status & GPIO_PIN1)
    {
    	Drdy = 0x01;
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }

}
