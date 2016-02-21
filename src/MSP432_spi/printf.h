/*
 * printf library for the MSP432
 *Taken from git repository:
 *https://github.com/samvrlewis/MSP432-printf.git
 *Made by Sam Lewis.
 *
 *
 */

#ifndef PRINTF_H_
#define PRINTF_H_
#include <driverlib.h>

void printf(uint32_t moduleInstance, char *, ...);

#endif /* PRINTF_H_ */
