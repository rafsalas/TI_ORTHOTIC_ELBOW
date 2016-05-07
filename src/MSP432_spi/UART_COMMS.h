/*
 * UART_COMMS.h
 *
 * 	Elbow Orthosis
 * 	Texas A&M University & Texas Instruments
 *
 *  Created on: Fall 2015
 *      Author: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas
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

extern uint8_t Cal_Request;
extern uint8_t Read_flag;

void uart_setup();
int *read_cal_angles();

#endif /* UART_COMMS_H_ */
