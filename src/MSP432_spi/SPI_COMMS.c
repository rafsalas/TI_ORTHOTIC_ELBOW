// SPI_COMMS.c

// Elbow Orthosis
// Texas A&M University & Texas Instruments
// Fall 2015 - Spring 2016
// Authors: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas

#include "SPI_COMMS.h"

// ADS1299 REGISTER SETTINGS
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


// SPI DATA
uint8_t SPI_Raw_Data[54]; // 54 Packets * 8 Bits per Packet = 216 Bits
double SPI_Data[8]; // 8 Channels (Unconditioned, 2's Complemented)
double SPI_Data_Window[8][100]; // 8 Channels, 100 Sample Window


// EMG DATA
uint32_t EMG_i; // EMG Sample Number
double EMG_Voltage_Window[100]; // Raw Data / ADC Resolution
const double ADS1299_Resolution = 0x800000;//0x1000000; // 24-Bit Resolution
double EMG_Convolution[100+11-1]; // Convolution Result


// DSP PARAMETERS
const int N_WIN = 100; // Window Size

// HIGH PASS FILTER
const int N_FIR_HP = 11; // Order of High Pass Filter
double h_HP[11] = { // Filter Coefficients for High Pass Filter
	    -0.2455987326682,  -0.2602154869553,   0.1160770995248,  -0.1397783727177,
	    0.04711656383727,   0.8898372317322,  0.04711656383727,  -0.1397783727177,
	     0.1160770995248,  -0.2602154869553,  -0.2455987326682
};


//-------------------------------------------------------
void spi_setup(){
/* SPI Master Configuration Parameter */
	msp_clk_rate = 48000;//48Mhz
	ads_clk_rate = 2048; //2.048 MHz

	EMG_i = 0;
	Drdy = 0x00;
    /* Starting and enabling LFXT (32kHz) */
	MAP_CS_initClockSignal(CS_SMCLK , CS_DCOCLK_SELECT , CS_CLOCK_DIVIDER_1);// clock source CS_ACLK, Use external clock, no clock divisions
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION );
	//selecting pins for spi mode
	//pin 5 = clk  pin 6 = mosi pin 7 =miso
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1 , GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7,
			GPIO_PRIMARY_MODULE_FUNCTION );

	//configuring SPI for 3wire master mode

	const eUSCI_SPI_MasterConfig spiMasterConfig =
	{
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,              // ACLK Clock Source
		MAP_CS_getSMCLK(),//32768,                                     // ACLK = LFXT = 32.768khz
		2000000,//1000000,                                    // SPICLK = 500khz (110k)
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
	//msb = msb & 0xFF; mid = mid & 0xFF; lsb = lsb & 0xFF;

	uint32_t num = (msb<<16)|(mid<<8)|(lsb); 	// Concatenate Bytes

	//num = num&(0xFFFFFF); // Cast Number as 24 Bit Integer

	if((num>>23)%2==1) return-((1<<24)-num); // If 24th Bit is a 1, then use 2's Complement
	else return num; // Otherwise, no need for 2's complement

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

	//spi_register_setting();

	// START COMMAND
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
	//__delay_cycles(5000);

	// READ DATA CONTINUOUSLY
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x10); // RDATAC
	//__delay_cycles(5000);

	// READ DATA BY COMMAND
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x12); // RDATAC
	//__delay_cycles(5000);


	// COMPLETE FLAG
	SPI_Connected=0x01; //Flag for SPI Initialize Complete
}

void spi_register_setting(){
	// Power Up sequencing - Single ended Input
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x11); //SDATAC  0001 0001
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x43); //CONFIG3 address 0100 0011
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x6C); //Register data 0110 1100
	__delay_cycles(960);
	__delay_cycles(10000);

	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x41); //CONFIG1 address 0100 0001
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00);// Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x96); //Register data - 500Hz data rate
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x42); //CONFIG2 address
//	 __delay_cycles(960);
	 __delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0xC0); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x45); //CH1SET address
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x07); //Write to 8 registers
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data - normal electrode
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Register data
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x55); //MISC1 address is 15h
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x20); //Register data - Set SRB1
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE,0x44); //LOFF address
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	MAP_SPI_transmitData(EUSCI_B0_MODULE,0x00); //Write to 1 register
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms

	MAP_SPI_transmitData(EUSCI_B0_MODULE,0x06); //Register data - 24nA, 31.2Hz
//	__delay_cycles(960);
	__delay_cycles(5000); //Wait 150ms
	__delay_cycles(1920);

	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x23); //config4
	__delay_cycles(5000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //read one reg
	__delay_cycles(5000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); //read one reg

	__delay_cycles(5000);

	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
//	__delay_cycles(960);
	__delay_cycles(5000);
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x10); // read data cont
	//__delay_cycles(5000);
}

void spi_write_registers(){
	// Write ADS1299 Firmware Registers

	// Stop Continuous Data Flow for Register Setting
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x11); //SDATAC
	__delay_cycles(100);

	// ID Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x40); // Write Starting at ID Address
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x0C); // Write 13 Registers
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x3E); // ID Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x96); // CONFIG1 Register (DR 250 Hz)
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x95); // CONFIG1 Register (DR 500 Hz)
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x94); // CONFIG1 Register (DR 1000 Hz)
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x93); // CONFIG1 Register (DR 2000 Hz)
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0xC0); // CONFIG2 Register
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0xD0); // Square Wave Test Signal
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x60); // CONFIG3 Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // LOFF Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // CH1SET Register
	//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x05); // Square Wave Test Signal
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH2SET Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH3SET Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH4SET Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH5SET Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH6SET Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH7SET Register
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x61); // CH8SET Register
	__delay_cycles(1000);

}

void spi_read_registers(){
	// Read ADS1299 Firmware Registers

	// Stop Continuous Data Flow for Register Setting
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x11); //SDATAC
	__delay_cycles(1000);


	// Read Address Command
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x20); // Read Starting at ID Address
	__delay_cycles(1000);
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x17); // Read 24 Registers
	__delay_cycles(1000);


	//1 ID Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	ID_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record ID Register
	__delay_cycles(1000);

	//2 CONFIG1 Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CONFIG1_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG1 Register
	__delay_cycles(1000);

	//3 CONFIG2 Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CONFIG2_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG2 Register
	__delay_cycles(1000);

	//4 CONFIG3 Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CONFIG3_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG3 Register
	__delay_cycles(1000);

	//5 LOFF Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF Register
	__delay_cycles(1000);

	//6 CH1SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH1SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH1SET Register
	__delay_cycles(1000);

	//7 CH2SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH2SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH2SET Register
	__delay_cycles(1000);

	//8 CH3SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH3SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH3SET Register
	__delay_cycles(1000);

	//9 CH4SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH4SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH4SET Register
	__delay_cycles(1000);

	//10 CH5SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH5SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH5SET Register
	__delay_cycles(1000);

	//11 CH6SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH6SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH6SET Register
	__delay_cycles(1000);

	//12 CH7SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH7SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH7SET Register
	__delay_cycles(1000);

	//13 CH8SET Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	CH8SET_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CH8SET Register
	__delay_cycles(1000);

	//14 BIAS_SENSP Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	BIAS_SENSP_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record BIAS_SENSP Register
	__delay_cycles(1000);

	//15 BIAS_SENSN Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	BIAS_SENSN_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record BIAS_SENSN Register
	__delay_cycles(1000);

	//16 LOFF_SENSP Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_SENSP_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_SENSP Register
	__delay_cycles(1000);

	//17 LOFF_SENSN Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_SENSN_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_SENSN Register
	__delay_cycles(1000);

	//18 LOFF_FLIP Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_FLIP_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_FLIP Register
	__delay_cycles(1000);

	//19 LOFF_STATP Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_STATP_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_STATP Register
	__delay_cycles(1000);

	//20 LOFF_STATN Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_STATN_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record LOFF_STATN Register
	__delay_cycles(1000);

	//21 GPIO Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	GPIO_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record GPIO Register
	__delay_cycles(1000);

	//22 MISC1 Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	MISC1_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record MISC1 Register
	__delay_cycles(1000);

	//23 MISC2 Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	MISC2_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record MISC2 Register
	__delay_cycles(1000);

	//24 CONFIG4 Register
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x00); // Dummy Signal
	__delay_cycles(1000);
	LOFF_SENSN_reg = MAP_SPI_receiveData(EUSCI_B0_MODULE); // Record CONFIG4 Register
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

#include <eusci.h>
#include <spi.h>

void SPI_Collect_Data(void)
{
	int win_i,i;

	uint32_t lsb;
	uint32_t mid;
	uint32_t msb;

	uint32_t stuck_i; // Stuck Counter
	uint32_t stuck_i_max = 100;
	uint8_t stuck; // Stuck Flag

	uint32_t container_1;
	int32_t container_2;
	uint8_t buffer_unempty;

	//
	MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
	//

	if(SPI_Connected)
	{

		//MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
		__delay_cycles(100);

		for(win_i=0;win_i<N_WIN;win_i++)
		{
			stuck_i=0;
			// DELAY IF DRDY NOT READY
			Drdy = GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN5);
			while(Drdy!=0)
			{

				if(buffer_unempty)
				{
					__delay_cycles(80); // SPI Delay
					EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00; // Clear Buffer
					__delay_cycles(80); // SPI Delay
					buffer_unempty = EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r;
				}


				Drdy = GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN5); // Check DRDY Pin

			}

			msb=0;
			while((msb!=0xC0 || mid!=0x00 || lsb!=0x00) && stuck_i<stuck_i_max)
			{
				// READ DATA BY COMMAND
				__delay_cycles(80); // SPI Delay
				EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x12; // RDATAC
				__delay_cycles(80); // SPI Delay


				// Routine Must Issue 24 + CHANNELS*24 SCLK Signals (216 Clock Signals)

				// VERIFY DATA WITH STATUS HEADER
				EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00;
				__delay_cycles(80); // SPI Delay
				msb=EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r; // 0xC0 Expected
				__delay_cycles(80); // SPI Delay

				EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00;
				__delay_cycles(80); // SPI Delay
				mid=EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r; // 0x00 Expected
				__delay_cycles(80); // SPI Delay

				EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00;
				__delay_cycles(80); // SPI Delay
				lsb=EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r; //0x00 Expected
				__delay_cycles(80); // SPI Delay

				stuck_i=stuck_i+1; // Detect if Stuck in Loop
			}

			stuck=0;
	    	if((msb==0xC0 && mid==0x00 && lsb==0x00) && stuck_i<stuck_i_max)
	    	{

				// PROMPT DATA STREAM
				//for(i=0;i<NUM_ACTIVE_CHANNELS;i++)
				for(i=0;i<NUM_CHANNELS;i++)

				{

					EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00;
					__delay_cycles(80); // SPI Delay
					msb = EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r;
					__delay_cycles(80); // SPI Delay


					EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00;
					__delay_cycles(80); // SPI Delay
					mid = EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r;
					__delay_cycles(80); // SPI Delay

					EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00;
					__delay_cycles(80); // SPI Delay
					lsb = EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r;
					__delay_cycles(80); // SPI Delay


					// Concatenate
					container_1=(msb<<16)|(mid<<8)|(lsb);

					// 2's Complement
					if((container_1>>23)%2==1) container_2=-((1<<24)-container_1);
					else container_2=container_1;

					SPI_Data_Window[i][win_i]=container_2;
				}

				// CHECK BUFFER
				__delay_cycles(80); // SPI Delay
				EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x00; // Clear Buffer
				__delay_cycles(80); // SPI Delay
				buffer_unempty = EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rRXBUF.r;
				if(buffer_unempty) win_i=win_i-1; // Invalidate Data if Buffer Not Empty

			}
			if(stuck_i>stuck_i_max)
			{
				win_i=win_i-1; // Invalidate Data if Stuck
				MAP_SPI_transmitData(EUSCI_B0_MODULE, 0x08); //START
			}

			__delay_cycles(100); // SPI Delay


		}
	    // STOP SPI CHANNEL
		//
		//EUSCI_B_CMSIS(EUSCI_B0_MODULE)->rTXBUF.r = 0x0A; // STOP DATA Conversion
		//
		__delay_cycles(10000); // SPI Delay
		//__delay_cycles(1000000); // SPI Delay
	}
}

void EMG_Condition_Data(void)
{
	int i,j;
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
		Convolution();

		sum = 0;
		for(j=N_FIR_HP;j<N_WIN-1;j++)
		{
			//EMG[i][j]=EMG_Convolution[i]
			//sum = sum + _Q1abs(*(double*)(p+i)); // Sum Middle of Convolution
			sum = sum + _Q1abs(EMG_Convolution[j]); // Sum Middle of Convolution
		}

		average = sum/(N_WIN-N_FIR_HP); // Average Middle of Convolution


		//if(average>EMG_max[i] || EMG_max[i]==0) EMG_max[i]=average;

		if(average<EMG_min[i] || EMG_min[i]==0) EMG_min[i]=average;

		// EMG History Buffer
		for(j=EMG_History-1;j>0;j--)
		{
			EMG[i][j]=EMG[i][j-1];
		}

		EMG[i][0]=(average-EMG_min[i])/(EMG_max[i]-EMG_min[i]);

		if(EMG[i][0] >= 1){
			EMG[i][0] = 1;
		}
	}

}

void Convolution()
{
	//Result[N_a+N_b-1];
	int n;
	for (n=(0)+N_FIR_HP;n<(N_FIR_HP+N_WIN-1)-N_FIR_HP;n++)
	{
		int kmin, kmax, k;

		EMG_Convolution[n] = 0;

		if(n>=N_FIR_HP-1) kmin=n-(N_FIR_HP-1);
		else kmin=0;

		if(n<N_WIN-1) kmax=n;
		else kmax=N_WIN-1;

		for(k=kmin;k<=kmax;k++)
		{
			EMG_Convolution[n]=EMG_Voltage_Window[k]*h_HP[n-k];
		}
	}
}


void Comparator(){
	//comparing bicep and tricep/wrist stuffs intention
	if(EMG[0][0] > EMG[1][0]){ //bicep intention decrease angle
		Upper_Arm_Intention = EMG[0][0];
		Direction_flag = 1;
	}else if(EMG[0][0] < EMG[1][0]){//tricep intention increase angle
		Upper_Arm_Intention = EMG[1][0];
		Direction_flag = 0;
	}else{
		Upper_Arm_Intention = 0;
		Direction_flag = -1;
	}

	/*
	if(EMG[2][0] > EMG[3][0]){ //forearm top intention
		Lower_Arm_Intention = EMG[2][0];
	}else if(EMG[2][0] < EMG[3][0]){//forearm bottom intention
		Lower_Arm_Intention = EMG[3][0];
	}else{
		Lower_Arm_Intention = 0;
	}*/

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
