/*
 * drv8.h
 *
 *  Created on: Feb 3, 2016
 *      Author: rafael
 */

#ifndef DRV8_H_
#define DRV8_H_
#include <driverlib.h>




// GPIO Port 1 Definitions
#define VREF    BIT3    // pot //P3.0
#define NC0     BIT4    //not needed
#define PUSH    BIT5    //
#define NC1     BIT7    //not needed

// GPIO Port 2 Definitions
#define NC2     BIT0    //not needed
#define BIN1    BIT1    // P2.6
#define BIN2    BIT2    // P2.7
#define NC3     BIT3    // not needed
#define AIN2    BIT4    // P2.5
#define	AIN1    BIT5    // P2.4
#define nSLEEP  BIT6    // P3.6. // digital high or low sleep if low
#define nFAULT  BIT7    // P3.7

void intiallize();



#endif /* DRV8_H_ */
