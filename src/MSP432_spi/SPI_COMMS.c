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

volatile double HP_Data[8][50] = {0};
volatile double BS_Data[8][50] = {0};
volatile double AVG_Data[8][50] = {0};
volatile double Check_Data = 0;

// SPI DATA
uint8_t SPI_Raw_Data[54]; // 54 Packets * 8 Bits per Packet = 216 Bits
double SPI_Data[8]; // 8 Channels (Unconditioned, 2's Complemented)
double SPI_Data_Window[8][100]; // 8 Channels, 100 Sample Window


// EMG DATA
uint32_t EMG_i; // EMG Sample Number
double EMG_Voltage_Window[100]; // Raw Data / ADC Resolution
const double ADS1299_Resolution = 0x1000000; // 24-Bit Resolution


// DSP PARAMETERS
const int N_WIN = 100; // Window Size

// HIGH PASS FILTER
const int N_FIR_HP = 11; // Order of High Pass Filter
double h_HP[11] = { // Filter Coefficients for High Pass Filter
	    -0.2455987326682,  -0.2602154869553,   0.1160770995248,  -0.1397783727177,
	    0.04711656383727,   0.8898372317322,  0.04711656383727,  -0.1397783727177,
	     0.1160770995248,  -0.2602154869553,  -0.2455987326682
};


// NORMALIZATION ROUTINE
double EMG_max[8]; // Maximum EMG Signal
double EMG_min[8];// = {100, 100, 100, 100, 100, 100, 100, 100}; // Minimum EMG Signal
double EMG_min_i[8];// = {51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51}; // Minimum EMG Signal Index

//-------------------------------------------------------
void spi_setup(){
/* SPI Master Configuration Parameter */
	msp_clk_rate = 48000;//48Mhz
	ads_clk_rate = 2048; //2.048 MHz

	EMG_i = 0;
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
		1000000,//1000000,                                    // SPICLK = 500khz (110k)
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



int32_t twos_to_signed (uint32_t msb, uint32_t mid, uint32_t lsb){

	// Cast Inputs as 8 Bit Unsigned Integer
	msb = msb & 0xFF; mid = mid & 0xFF; lsb = lsb & 0xFF;

	uint32_t num = (msb<<16)|(mid<<8)|(lsb); 	// Concatenate Bytes

	int32_t num2; 	// Signed Result

	num = num&(0xFFFFFF); // Cast Number as 24 Bit Integer

	if((num>>23)%2==1) num2=-((1<<24)-num); // If 24th Bit is a 1, then use 2's Complement
	else num2=num; // Otherwise, no need for 2's complement

    return num2;
}


void spi_start(int32_t sample[50]){

	__delay_cycles(5000); //Wait 150ms

	// RESET
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x06); //RESET 0000 0110
	__delay_cycles(5000); //Wait 150ms

	// WRITE REGISTERS
	spi_write_registers();
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


void SPI_Collect_Data(void)
{
	int win_i,i;
	if(SPI_Connected)
	{

		for(win_i=0;win_i<N_WIN;win_i++)
		{

			SPI_Rate_Flag=0;
			while(SPI_Rate_Flag);


			// DELAY IF DRDY NOT READY
			//probe drdy___________________________________________________________________________________________
			while(Drdy==0) __delay_cycles(10);


			// READ DATA BY COMMAND
			SPI_transmitData(EUSCI_B0_MODULE, 0x12); // RDATAC
			__delay_cycles(100);

			// PROMPT DATA STREAM
			// Issue 24 + CHANNELS*24 SCLK Signals (216 Clock Signals)
			for(i=0;i<(NUM_CHANNELS+1)*3;i++)
			{
		    	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // DUMMY Data Signal; 8 SCLK Signals
		    	__delay_cycles(100); // Read Delay
		    	SPI_Raw_Data[i] = SPI_receiveData(EUSCI_B0_MODULE);
			}

			// FORMAT DATA STORAGE
			for(i=0;i<NUM_CHANNELS;i++) // Iterate over (8) Channels
			{
				// Concatenate and 2's Complement
				// 3 (8 Bit Status Signal) + i * 3 (8 Bit Channel Signals)
				SPI_Data[i] = twos_to_signed( SPI_Raw_Data[3+i*3],SPI_Raw_Data[3+i*3+1],SPI_Raw_Data[3+i*3+2]);
				SPI_Data_Window[i][win_i]=SPI_Data[i];
			}

		}
	}
}

void EMG_Condition_Data(void)
{
	int i,j;
	double *p = malloc(N_WIN+N_FIR_HP-1);
	double sum = 0;
	double average;
	// CONDITION DATA
	for(i=0;i<NUM_ACTIVE_CHANNELS;i++) // Iterate over (4) Active Channels
	{
		for(j=0;j<N_WIN;j++)
		{
			EMG_Voltage_Window[j]=SPI_Data_Window[i][j]/ADS1299_Resolution;
		}

		// Convolution
		Convolution(N_FIR_HP,EMG_Voltage_Window,N_WIN,&h_HP[0],N_FIR_HP,p);

		for(j=N_FIR_HP;j<N_WIN;j++)
		{
			sum = sum + _Q1abs(*(p+i)); // Sum Middle of Convolution
		}

		average = sum/(N_WIN-N_FIR_HP); // Average Middle of Convolution

		if(average>EMG_max[i]) EMG_max[i]=average;

		if(average<EMG_min[i]) EMG_min[i]=average;

		// EMG History Buffer
		for(j=EMG_History-1;j>0;j--)
		{
			EMG[i][j]=EMG[i][j-1];
		}

		EMG[i][0]=(average-EMG_min[i])/(EMG_max[i]-EMG_min[i]);
	}

}

void Convolution(int trim, double* a, int N_a, double* b, int N_b, double* Result)
{
	//Result[N_a+N_b-1];
	uint32_t n;
	for (n=(0)+trim;n<(N_a+N_b-1)-trim;n++)
	{
		uint32_t kmin, kmax, k;

		*((double*)(Result+n)) = 0;

		if(n>=N_b-1) kmin=n-(N_b-1);
		else kmin=0;

		if(n<N_a-1) kmax=n;
		else kmax=N_a-1;

		for(k=kmin;k<=kmax;k++)
		{
			*((double*)(Result+n))=*((double*)(a+n)) * (*(double*)(b+(n-k)));
		}
	}
}


//----------------------interrupts----------------------------------------------------------------------

void gpio_isr3(void)//drdy intrpt
{
    uint32_t status;
    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);

    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P3, status);

    /* set Drdy Flag*/
    if(status & GPIO_PIN5)
    {
    	Drdy = GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN5);
    }
}




// Old Code
/*
void SPI_Collect_Data(void)
{

	// READ DATA BY COMMAND
	SPI_transmitData(EUSCI_B0_MODULE, 0x12); // RDATAC
	__delay_cycles(100);

	// PROMPT DATA STREAM
	// Issue 24 + CHANNELS*24 SCLK Signals (216 Clock Signals)
	int i,j;
	for(i=0;i<(NUM_CHANNELS+1)*3;i++)
	{
    	SPI_transmitData(EUSCI_B0_MODULE, 0x00); // DUMMY Data Signal; 8 SCLK Signals
    	__delay_cycles(100); // Read Delay
    	SPI_Raw_Data[i] = SPI_receiveData(EUSCI_B0_MODULE);
	}

	// CONDITION DATA
	for(i=0;i<NUM_CHANNELS;i++) // Iterate over (8) Channels
	{
		// Concatenate and 2's Complement
		// 3 (8 Bit Status Signal) + i * 3 (8 Bit Channel Signals)
		SPI_Data[i] = twos_to_signed( SPI_Raw_Data[3+i*3],SPI_Raw_Data[3+i*3+1],SPI_Raw_Data[3+i*3+2]);
		SPI_Data[i] = SPI_Data[i]/ADS1299_Resolution;

		// EMG History Buffer
		for(j=EMG_History-1;j>0;j--)
		{
			HP_Data[i][j]=HP_Data[i][j-1];
			BS_Data[i][j]=BS_Data[i][j-1];
			AVG_Data[i][j]=AVG_Data[i][j-1];
			EMG[i][j]=EMG[i][j-1];
		}
		
		// Filter, Average, Normalize
		//HP_Data[i][0]  =High_Pass(     i,          (double)SPI_Data[i],N_FIR_HP); // High Pass Filter, High Pass Overhead
		//BS_Data[i][0]  =Band_Stop(     i,                HP_Data[i][0],N_FIR_HP + N_FIR_BS ); // Band Stop Filter, High Pass + Band Stop Overhead


		//Check_Data     =Error_Check(   i,BS_Data[i][0], AVG_Data[i][0],N_FIR_HP + N_FIR_BS + N_AVG);
		//AVG_Data[i][0] =Moving_Average(i,                   Check_Data,N_FIR_HP + N_FIR_BS + N_AVG); // Moving Average, High Pass + Band Stop + Moving Average Overhead


		//AVG_Data[i][0] =Moving_Average(i,                BS_Data[i][0],N_FIR_HP + N_FIR_BS + N_AVG); // Moving Average, High Pass + Band Stop + Moving Average Overhead

		//EMG[i][0]      =Normalize(     i,AVG_Data[i][0],AVG_Data[i][1],N_FIR_HP + N_FIR_BS + N_AVG); // Normalize, High Pass + Band Stop + Moving Average Overhead

		EMG[i][0] = SPI_Data[0];



		MAP_UART_transmitData(EUSCI_A0_MODULE, EMG[i][0]);


	}
	EMG_i = EMG_i + 1; // Increment EMG Data Sample Number
	__delay_cycles(100); // Read Delay


}

double High_Pass(int channel, double data_in, uint32_t overhead)
{
	uint32_t i;
	double data_out = 0;

	// Shift History Buffer Back
	for(i=N_FIR_HP-1;i>0;i--)
	{
		Buffer_HP[channel][i] = Buffer_HP[channel][i-1];
	}

	// Add New Sample
	Buffer_HP[channel][0] = data_in;

	// Filter
	for(i=0;i<N_FIR_HP;i++)
	{
		data_out = data_out + Buffer_HP[channel][i]*h_HP[i];
	}

	if(EMG_i>overhead) return data_out;
	else return 0;
}

double Band_Stop(int channel, double data_in, uint32_t overhead)
{
	uint32_t i;
	double data_out = 0;

	// Shift History Buffer Back
	for(i=N_FIR_BS-1;i>0;i--)
	{
		Buffer_BS[channel][i] = Buffer_BS[channel][i-1];
	}

	// Add New Sample
	Buffer_BS[channel][0] = data_in;

	// Filter
	for(i=0;i<N_FIR_BS;i++)
	{
		data_out = data_out + Buffer_BS[channel][i]*h_BS[i];
	}



	if(EMG_i>overhead) return data_out;
	else return 0;
}

double Moving_Average(int channel, double data_in, uint32_t overhead)
{
	uint32_t i;
	double data_out = 0;

	// Shift History Buffer Back
	for(i=N_AVG-1;i>0;i--)
	{
		Buffer_AVG[channel][i] = Buffer_AVG[channel][i-1];
	}

	// Add New Sample
	Buffer_AVG[channel][0] = data_in;

	// Filter
	for(i=0;i<N_AVG;i++)
	{
		data_out = data_out + _Q1abs(Buffer_AVG[channel][i]);
	}



	if(EMG_i>overhead) return data_out/N_AVG;
	else return 0;
}

double Normalize(int channel, double data_in, double data_in_old, uint32_t overhead)
{

	if(EMG_i>overhead)
	{
		// Store Maximum EMG Value (Dynamic)
		if(data_in>EMG_max[channel]) EMG_max[channel] = data_in;

		// Store Initial Minimum EMG Value (Once)
		if(data_in_old<0.1 && EMG_min_i[channel]==0 && EMG_i>overhead+N_AVG)
		{
			EMG_min_i[channel]=EMG_i;
			EMG_min[channel]=data_in;
		}
		// Store Minimum EMG Value (Dynamic)
		if(EMG_i>EMG_min_i[channel] && data_in<EMG_min[channel]) EMG_min[channel] = data_in;

		return (data_in-EMG_min[channel])/(EMG_max[channel]-EMG_min[channel]);
	}
	else
	{
		return data_in;
	}
}

double Error_Check(int channel, double input, double average, uint32_t overhead)
{
	if(EMG_i>overhead)
	{
		if(_Q1abs(input)>10*average || _Q1abs(input)<0.1*average) return average;
		else return input;
	}
	else return input;
}

// HIGH PASS FILTER
const int N_FIR_HP = 51; // Order of High Pass Filter
double Buffer_HP[8][51]; // Buffer for High Pass Filter
const double h_HP[51] = { // Filter Coefficients for High Pass Filter
   0.001199133912997,-0.002440111986486,-0.002035725104674, -0.00139925064403,
  0.0001070335519827, 0.002366069954306, 0.004596957382711, 0.005632855909442,
   0.004418459381303, 0.000573592710898,-0.005169814894392, -0.01091049180765,
   -0.01404581533516, -0.01219369321007,-0.004294831557407, 0.008507173118647,
    0.02261686565702,   0.0326106842957,  0.03266738121427,  0.01843638662614,
   -0.01124611203015, -0.05341045838623,  -0.1011975637409,  -0.1453328673629,
    -0.1764660756429,   0.8123093725108,  -0.1764660756429,  -0.1453328673629,
    -0.1011975637409, -0.05341045838623, -0.01124611203015,  0.01843638662614,
    0.03266738121427,   0.0326106842957,  0.02261686565702, 0.008507173118647,
  -0.004294831557407, -0.01219369321007, -0.01404581533516, -0.01091049180765,
  -0.005169814894392, 0.000573592710898, 0.004418459381303, 0.005632855909442,
   0.004596957382711, 0.002366069954306,0.0001070335519827, -0.00139925064403,
  -0.002035725104674,-0.002440111986486, 0.001199133912997
};


// BAND STOP FILTER
const int N_FIR_BS = 51; // Order of Band Stop Filter
double Buffer_BS[8][51]; // Buffer for Band Stop Filter
const double h_BS[51] = { // Filter Coefficients for Band Stop Filter
    0.01170915995068,4.568526127283e-05,-0.0009865874736183,-1.592672624989e-05,
  -0.001858304770361,-0.001315562519492, 0.006345768096185,  0.00431105416751,
   -0.01246776230191, -0.01002577553774,  0.01899605772893,  0.01837114053531,
    -0.0252761001099, -0.02961353939938,  0.02968670237753,  0.04266565383013,
   -0.03159577923216, -0.05677064733728,  0.02985485986251,  0.06996876714235,
   -0.02471776736712, -0.08108181425934,   0.0162002470427,  0.08824264510038,
  -0.005727847424186,   0.9091097201922,-0.005727847424186,  0.08824264510038,
     0.0162002470427, -0.08108181425934, -0.02471776736712,  0.06996876714235,
    0.02985485986251, -0.05677064733728, -0.03159577923216,  0.04266565383013,
    0.02968670237753, -0.02961353939938,  -0.0252761001099,  0.01837114053531,
    0.01899605772893, -0.01002577553774, -0.01246776230191,  0.00431105416751,
   0.006345768096185,-0.001315562519492,-0.001858304770361,-1.592672624989e-05,
  -0.0009865874736183,4.568526127283e-05,  0.01170915995068
};


// MOVING AVERAGE FILTER
const int N_AVG = 51; // Number of Samples for Moving Average
double Buffer_AVG[8][51]; // Buffer for Moving Average

*/
