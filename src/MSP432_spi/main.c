
//***************************************************************************************

//***************************************************************************************

#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>

 volatile uint8_t AllData[100][9][3];
static uint8_t iter = 0;

static volatile uint8_t RXData[9][3];//BITS must match array size
static uint8_t TXData = 0;
volatile uint8_t Drdy;


/* SPI Master Configuration Parameter */
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
        EUSCI_B_SPI_CLOCKSOURCE_ACLK,              // ACLK Clock Source
        32768,                                     // ACLK = LFXT = 32.768khz
        500000,                                    // SPICLK = 500khz
        EUSCI_B_SPI_MSB_FIRST,                     // MSB First
        EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,    // Phase
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW, // low polarity
        EUSCI_B_SPI_3PIN                           // 3Wire SPI Mode
};



void spi_setup(){

    /* Starting and enabling LFXT (32kHz) */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    CS_setExternalClockSourceFrequency(32768, 0);//32.768 KHz
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);// clock source CS_ACLK, Use external clock, no clock divisions
    CS_startLFXT(CS_LFXT_DRIVE0); //LFXTDRIVE_0 give the lowest current consumption according to the msp432 header file for this board

	//selecting pins for spi mode
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1 , GPIO_PIN5 |GPIO_PIN6|GPIO_PIN7,
			GPIO_PRIMARY_MODULE_FUNCTION );

	//selecting gpio pin for CS
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN2);//set direction
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2); //set high to turn off

    //Drdy
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P5, GPIO_PIN1);//set direction
   // Drdy = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1);

	//configuring SPI for 3wire master mode
	SPI_initMaster(EUSCI_B0_MODULE,&spiMasterConfig);//EUSCI_B0_MODULE = Base address of module registers
	SPI_enableModule(EUSCI_B0_MODULE);
	SPI_enableInterrupt(EUSCI_B0_MODULE, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIB0);
    Interrupt_enableSleepOnIsrExit();

    /* Delaying waiting for the module to initialize */
    uint8_t i;
    for(i=0; i<100;i++);

}



void main(void)
{
	WDT_A_holdTimer();
	spi_setup();

    /* SPI, put CS high P5.2 and polling to see if the TX buffer is ready or busy */
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);//set low to turn on

    /*Not sure I need to send a transmission*/
    TXData = 0x40;
    while (!(SPI_getInterruptStatus(EUSCI_B0_MODULE,EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
    SPI_transmitData(EUSCI_B0_MODULE, TXData);

    TXData = 0x00;
    while (!(SPI_getInterruptStatus(EUSCI_B0_MODULE,EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
    SPI_transmitData(EUSCI_B0_MODULE, TXData);

    //start loop
    while(1){}

}
volatile uint8_t val;
//talk to nathan about the bit length
void euscib0_isr(void)
{


	volatile uint8_t value = SPI_receiveData(EUSCI_B0_MODULE);

  /*  uint32_t status = SPI_getEnabledInterruptStatus(EUSCI_B0_MODULE);
    SPI_clearInterruptFlag(EUSCI_B0_MODULE, status);

    if(status & EUSCI_B_SPI_RECEIVE_INTERRUPT)
    {
    	uint8_t p = 0;
    	uint8_t j = 0;
    	for(p = 0;p<9;++p){
    		for(j = 0;j<3;++j){
    			RXData[p][j] = SPI_receiveData(EUSCI_B0_MODULE);
    			val = RXData[p][j];
    		}
    	}

    	if(iter < 100){
    		AllData[iter++] = RXData;

    	}else{
    		iter = 0;
    		AllData[iter++] = RXData;
    	}
    }*/
}


