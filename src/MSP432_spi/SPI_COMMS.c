/*
 * SPI_COMMS.c
 *
 *  Created on: Feb 17, 2016
 *      Author: rafael
 */

#include "SPI_COMMS.h"

volatile uint16_t ID_reg = 0;
volatile uint16_t CONFIG1_reg = 0;
volatile uint16_t CONFIG2_reg = 0;
volatile uint16_t CONFIG3_reg = 0;
volatile uint16_t CONFIG4_reg = 0;
volatile uint16_t LOFF_reg = 0;
volatile uint16_t CHSET[8];
volatile uint16_t CH1SET_reg = 0;
volatile uint16_t CH2SET_reg = 0;
volatile uint16_t CH3SET_reg = 0;
volatile uint16_t CH4SET_reg = 0;
volatile uint16_t CH5SET_reg = 0;
volatile uint16_t CH6SET_reg = 0;
volatile uint16_t CH7SET_reg = 0;
volatile uint16_t CH8SET_reg = 0;
volatile uint16_t BIAS_SENSP_reg = 0;
volatile uint16_t BIAS_SENSN_reg = 0;
volatile uint16_t LOFF_SENSP_reg = 0;
volatile uint16_t LOFF_SENSN_reg = 0;
volatile uint16_t LOFF_FLIP_reg = 0;
volatile uint16_t LOFF_STATP_reg = 0;
volatile uint16_t LOFF_STATN_reg = 0;
volatile uint16_t GPIO_reg = 0;
volatile uint16_t MISC1_reg = 0;
volatile uint16_t MISC2_reg = 0;

volatile uint8_t SPI_Raw_Data[54];
volatile uint16_t DATA = 0;


int32_t SPI_Data_Window[50][8];


//-------------------------------------------------------
void spi_setup(){
/* SPI Master Configuration Parameter */
	msp_clk_rate = 48000;//48Mhz
	ads_clk_rate = 2048; //2.048 MHz
	spi_index = 0;
	Drdy = 0x00;
    /* Starting and enabling LFXT (32kHz) */
	//MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
	//CS_setExternalClockSourceFrequency(32768, 0);//32.768 KHz
	//MAP_CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);// clock source CS_ACLK, Use external clock, no clock divisions
	MAP_CS_initClockSignal(CS_SMCLK , CS_DCOCLK_SELECT , CS_CLOCK_DIVIDER_1);// clock source CS_ACLK, Use external clock, no clock divisions
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION );
	//MAP_CS_startLFXT(CS_LFXT_DRIVE0); //LFXTDRIVE_0 give the lowest current consumption according to the msp432 header file for this board
	//MAP_CS_startHFXT(0);
	//selecting pins for spi mode
	//pin 5 = clk  pin 6 = mosi pin 7 =miso
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1 , GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7,
			GPIO_PRIMARY_MODULE_FUNCTION );

	//configuring SPI for 3wire master mode
//	x = CS_getSMCLK();
	const eUSCI_SPI_MasterConfig spiMasterConfig =
	{
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,              // ACLK Clock Source
		CS_getSMCLK(),//32768,                                     // ACLK = LFXT = 32.768khz
		1000000,//500000,                                    // SPICLK = 500khz (110k)
        EUSCI_B_SPI_MSB_FIRST,                     // MSB First
        EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,    // Phaseb
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW, // low polarity
        EUSCI_B_SPI_3PIN                           // 3Wire SPI Mode
	};
	MAP_SPI_initMaster(EUSCI_B0_MODULE,&spiMasterConfig);//EUSCI_B0_MODULE = Base address of module registers
	MAP_SPI_enableModule(EUSCI_B0_MODULE);
	MAP_Interrupt_enableInterrupt(INT_EUSCIB0);
	MAP_Interrupt_enableSleepOnIsrExit();

   /* Delaying waiting for the module to initialize */
	__delay_cycles(500);

}

void conditionSamples(int32_t samples [50], uint32_t filtered[50]){
	uint8_t i = 0;
	for(i = 0; i < window;++i){
		samples[i] = _Q1abs(samples[i]);//rectify
		//moving average
		sum = sum + samples[i];
		if(i>window){
			sum = sum - samples[i-window];
			filtered[i] = sum/window;
		}
		else if(i == window){
			filtered[i] = sum/window;
		}
		else{
			filtered[i] = 0;
		}
		MAP_UART_transmitData(EUSCI_A0_MODULE, filtered[i]);
	}
}

int32_t twos_to_signed (uint32_t msb, uint32_t mid, uint32_t lsb){
//	uint32_t num = (msb<<24)|(mid<<16)|(lsb<<8);// concatonate bytes

	// Cast Inputs as 8 Bit Unsigned Integer
	msb = msb & 0xFF;
	mid = mid & 0xFF;
	lsb = lsb & 0xFF;

	uint32_t num = (msb<<16)|(mid<<8)|(lsb);  // Concatenate Bytes

	int32_t num2;

	num = num&(0x0FFF); // Cast num as 24 Bit Integer

	if((num>>23)%2==1)
	{
		num2=(1<<24)-num;
		num2=-num2;
	}
	else
	{
		num2=num;
	}


	/*
    if(((int)num)<0){
            num2=-((~num)+1); //2's complement
    }
    else{
            num2 = num;
    }
    num2 = num2>>2; //shift to correct for 2 bit offset needed for 2's complement
    */
    return num2;
}

void read_message(int32_t sample[50], uint32_t filtered[50]){
	unsigned int cycle = (msp_clk_rate/ads_clk_rate)+1; // cycle ratio needed for delays between reading bytes
	unsigned int delay = 4*cycle;
	unsigned int iterator = 0;
	unsigned int iter = 0;
	uint32_t container[3];

	if(spi_index == window){//condition sample and reset  spi_index
		conditionSamples(sample , filtered);
		spi_index = 0;
	}
	uint8_t it = 0;
	for(it=0;it<3;++it){
		SPI_receiveData(EUSCI_B0_MODULE); //read status byte and throw away
	}
	for(iterator = 0;iterator < 24;++iterator){//24 bytes
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

void spi_start(int32_t sample[50]){

	__delay_cycles(5000); //Wait 150ms

	// RESET
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x06); //RESET 0000 0110
	__delay_cycles(5000); //Wait 150ms

	// WRITE REGISTERS
	spi_write_registers();

	// READ REGISTERS
	spi_read_registers();

	// START COMMAND
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
	__delay_cycles(5000);

	// READ DATA CONTINUOUSLY
	//SPI_transmitData(EUSCI_B0_MODULE, 0x10); // RDATAC
	//__delay_cycles(5000);

	// READ DATA BY COMMAND
	//SPI_transmitData(EUSCI_B0_MODULE, 0x12); // RDATAC
	//__delay_cycles(5000);


	// COMPLETE FLAG
	SPI_Connected=0x01; //Flag for SPI Initialize Complete
}

void spi_register_setting(){
	// Power Up sequencing - Single ended Input
	SPI_transmitData(EUSCI_B0_MODULE, 0x11); //SDATAC  0001 0001
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x43); //CONFIG3 address 0100 0011
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x6C); //Register data 0110 1100
	__delay_cycles(960);
	__delay_cycles(10000);

	SPI_transmitData(EUSCI_B0_MODULE, 0x41); //CONFIG1 address 0100 0001
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00);// Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x96); //Register data - 500Hz data rate
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE, 0x42); //CONFIG2 address
//	 __delay_cycles(960);
	 __delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0xC0); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE, 0x45); //CH1SET address
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x07); //Write to 8 registers
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data - normal electrode
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE, 0x55); //MISC1 address is 15h
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE, 0x20); //Register data - Set SRB1
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE,0x44); //LOFF address
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	SPI_transmitData(EUSCI_B0_MODULE,0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	SPI_transmitData(EUSCI_B0_MODULE,0x06); //Register data - 24nA, 31.2Hz
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	__delay_cycles(1920);

	SPI_transmitData(EUSCI_B0_MODULE, 0x23); //config4
	__delay_cycles(5000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //read one reg
	__delay_cycles(5000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); //read one reg

	__delay_cycles(5000);

	SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
//	__delay_cycles(960);
	__delay_cycles(5000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x10); // read data cont
	__delay_cycles(5000);
}

void spi_write_registers(){
	// Write ADS1299 Firmware Registers

	// Stop Continuous Data Flow for Register Setting
	SPI_transmitData(EUSCI_B0_MODULE, 0x11); //SDATAC
	__delay_cycles(100);


	// ID Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x40); // Write Starting at ID Address
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x0C); // Write 13 Registers
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x3E); // ID Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x96); // CONFIG1 Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0xC0); // CONFIG2 Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x60); // CONFIG3 Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // LOFF Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH1SET Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH2SET Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH3SET Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH4SET Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH5SET Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH6SET Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH7SET Register
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH8SET Register
	__delay_cycles(1000);

}

void spi_read_registers(){
	// Read ADS1299 Firmware Registers

	// Stop Continuous Data Flow for Register Setting
	SPI_transmitData(EUSCI_B0_MODULE, 0x11); //SDATAC
	__delay_cycles(1000);


	// Read Address Command
	SPI_transmitData(EUSCI_B0_MODULE, 0x20); // Read Starting at ID Address
	__delay_cycles(1000);
	SPI_transmitData(EUSCI_B0_MODULE, 0x17); // Read 24 Registers
	__delay_cycles(1000);


	//1 ID Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	ID_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record ID Register
	__delay_cycles(1000);

	//2 CONFIG1 Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CONFIG1_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG1 Register
	__delay_cycles(1000);

	//3 CONFIG2 Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CONFIG2_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG2 Register
	__delay_cycles(1000);

	//4 CONFIG3 Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CONFIG3_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG3 Register
	__delay_cycles(1000);

	//5 LOFF Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF Register
	__delay_cycles(1000);

	//6 CH1SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH1SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH1SET Register
	__delay_cycles(1000);

	//7 CH2SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH2SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH2SET Register
	__delay_cycles(1000);

	//8 CH3SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH3SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH3SET Register
	__delay_cycles(1000);

	//9 CH4SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH4SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH4SET Register
	__delay_cycles(1000);

	//10 CH5SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH5SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH5SET Register
	__delay_cycles(1000);

	//11 CH6SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH6SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH6SET Register
	__delay_cycles(1000);

	//12 CH7SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH7SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH7SET Register
	__delay_cycles(1000);

	//13 CH8SET Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH8SET_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CH8SET Register
	__delay_cycles(1000);

	//14 BIAS_SENSP Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	BIAS_SENSP_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record BIAS_SENSP Register
	__delay_cycles(1000);

	//15 BIAS_SENSN Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	BIAS_SENSN_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record BIAS_SENSN Register
	__delay_cycles(1000);

	//16 LOFF_SENSP Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_SENSP_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_SENSP Register
	__delay_cycles(1000);

	//17 LOFF_SENSN Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_SENSN_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_SENSN Register
	__delay_cycles(1000);

	//18 LOFF_FLIP Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_FLIP_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_FLIP Register
	__delay_cycles(1000);

	//19 LOFF_STATP Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_STATP_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_STATP Register
	__delay_cycles(1000);

	//20 LOFF_STATN Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_STATN_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_STATN Register
	__delay_cycles(1000);

	//21 GPIO Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	GPIO_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record GPIO Register
	__delay_cycles(1000);

	//22 MISC1 Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	MISC1_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record MISC1 Register
	__delay_cycles(1000);

	//23 MISC2 Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	MISC2_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record MISC2 Register
	__delay_cycles(1000);

	//24 CONFIG4 Register
	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_SENSN_reg = SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG4 Register
	__delay_cycles(1000);

}

void drdy_setup(){
    /* Configuring P3.5 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN5);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN5);
    MAP_Interrupt_enableInterrupt(INT_PORT3);

    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
}


//----------------------interrupts----------------------------------------------------------------------

void gpio_isr3(void)//drdy intrpt
{
    uint32_t status;
    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);

    // DRDY Toggle
    //if(Drdy==0x01) Drdy=0x00;
    //else Drdy=0x01;

    //Drdy=Drdy+1;


    if(SPI_Connected && SPI_Cleared)//&& Drdy==0x01)
    {
       	SPI_Cleared=0;
       	SPI_Collect_Data();
       	SPI_Cleared=1;
    }




    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P3, status);

    /* set Drdy Flag*/
    if(status & GPIO_PIN5)
    {
    	//Drdy = Drdy+1;
    	//	Drdy = 0x01;
     //   MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //    read_message();
    }
}

void SPI_Collect_Data(void)
{
	unsigned int cycle = (msp_clk_rate/ads_clk_rate)+1; // cycle ratio needed for delays between reading bytes

	if(spi_index == window){//condition sample and reset  spi_index
		conditionSamples(sample , filtered);
		spi_index = 0;
	}


	// READ DATA BY COMMAND
	SPI_transmitData(EUSCI_B0_MODULE, 0x12); // RDATAC
	__delay_cycles(100);

	// PROMPT DATA STREAM
	// Issue 24 + CHANNELS*24 SCLK Signals (216 Clock Signals)
	int i;
	for(i=0;i<(NUM_CHANNELS+1)*3;i++)
	{
    	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // DUMMY Data Signal; 8 SCLK Signals
    	__delay_cycles(10); // Read Delay
    	SPI_Raw_Data[i] = SPI_receiveData(EUSCI_B0_MODULE);
	}

	for(i=0;i<NUM_CHANNELS;i++) // Iterate over (8) Channels
	{
		// 3 (8 Bit Status Signal) + i * 3 (8 Bit Channel Signals)
		SPI_Data_Window[spi_index][i]=twos_to_signed(SPI_Raw_Data[3+i*3],SPI_Raw_Data[3+i*3+1],SPI_Raw_Data[3+i*3+2]);
	}

	sample[spi_index]=SPI_Data_Window[spi_index][0];




	//Drdy = 0x01;
	spi_index++; //increment first dimension of spi_data index

}

