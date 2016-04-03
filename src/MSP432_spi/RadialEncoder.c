// RadialEncoder.c

// Elbow Orthosis
// Texas A&M University & Texas Instruments
// Fall 2015 - Spring 2016
// Authors: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas

#include "RadialEncoder.h"


int32_t raw_position = 0;
int32_t pos_pulse_count = 0;
int32_t neg_pulse_count = 0;

// ANGLE DAMPER LOOK-UP TABLES
// NORMAL CDF BASIS
const int N_DAMP = 20; // Order of Damping Look-Up Table
double h_DAMP[20] = { // Coefficients in Damping Look-Up Table
		0,
		0.0637646354428060,
		0.166161647646428,
		0.306579295220414,
		0.471011823050417,
		0.635444350880420,
		0.775861998454406,
		0.878259010658028,
		0.942023646100834,
		0.975930899347034,
		0.991327201858212,
		0.997296733567729,
		0.999273025352170,
		0.999831667917365,
		0.999966493618762,
		0.999994274700306,
		0.999999161721359,
		0.999999895622512,
		0.999999989704829,
		1
};


void encoderInit(){
    /* Configuring P3.5 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0 |GPIO_PIN1|GPIO_PIN2);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);//A,B,Z
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1);
    MAP_Interrupt_enableInterrupt(INT_PORT4);
    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster();
}


void Angle_Dampen(){
	uint32_t ANGLE_i;

	// Convert Measured Angle into Index for Lookup Table
	ANGLE_i=(uint32_t)(N_DAMP*(ANGLE_deg[0]-ANGLE_min)/(ANGLE_max-ANGLE_min));

	if(ANGLE_dir<0) ANGLE_damp=h_DAMP[ANGLE_i]; // Decreasing Angle (ELBOW CLOSING)
	else            ANGLE_damp=h_DAMP[N_DAMP-ANGLE_i]; // Increasing Angle (ELBOW OPENING)

}

//----------------------interrupts----------------------------------------------------------------------

void gpio_isr4(void)
{
//	uint32_t status;
	uint8_t val[3];
	val[0] = MAP_GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0);
	val[1] = MAP_GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN1);
	val[2] = MAP_GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2);

//    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);
//    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, status);
    if(val[0]==val[1]==val[2] ){ //one step CW
		printf(EUSCI_A0_MODULE,"in neg\r\n");
		raw_position++;
	}else{ //one step CCW
		printf(EUSCI_A0_MODULE,"in pos\r\n");
		raw_position--;
	}
}



