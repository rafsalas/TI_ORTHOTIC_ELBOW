/*
 * UART_COMMS.h
 *
 *  Created on: Feb 20, 2016
 *      Author: rafael
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
