/*
 * drv8.c
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#include "drv8.h"

void intiallize(){
	MAP_WDT_A_holdTimer();
	GPIO_setAsOutputPin(GPIO_PORT_P3, nSLEEP);
	GPIO_setAsInputPin(GPIO_PORT_P3, nFAULT);

	// initialize pins


}



