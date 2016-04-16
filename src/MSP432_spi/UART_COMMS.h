/*
 * UART_COMMS.h
 *
 *  Created on: Feb 20, 2016
 *      Author: rafael
 *
 *      Uart setup with a baud rate of 9600 with smclk running at 3MHz. LED indicators tell transmission status. Uart pins are connected to blue tooth
 *
 *      Pin map:
 *      3.2 = Rx
 *      3.3 = Tx
 *      1.0 = red led
 *      2.2 = blue LED
 *
 *  	CLK : SMCLK
 *
 */

#ifndef UART_COMMS_H_
#define UART_COMMS_H_

#include <driverlib.h>
#include "printf.h"

extern uint8_t Cal_Request;
extern uint8_t Read_flag;

void uart_setup();
int *read_cal_angles();

#endif /* UART_COMMS_H_ */
