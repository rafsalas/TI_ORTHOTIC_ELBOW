/*
 * RadialEncoder.c
 *
 *  Created on: Nov 30, 2015
 *      Author: rafael
 */
#include "RadialEncoder.h"

void encoderInit(){

/*	P1OUT |= (ENCODER_A+ENCODER_B);	//enable pull-up resistor
	P1REN |= ENCODER_A+ENCODER_B;	//enable pull-up resistor
	P1IFG &= ~ENCODER_A;			//clear interupt flag
	P1IE |= ENCODER_A;				//enable interupt for encoder

	__enable_interrupt();*/
}


/**
 * function call on CCW rotation, modify code but don't rename it!
 */
void stepCCW(){
//	P1OUT ^= LED1; //toogle led1
}

/**
 * function call on CW rotation, modify code but don't rename it!
 */
void stepCW(){
	//P1OUT ^= LED2; //toogle led2
}





//#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
/*	if(P1IN & ENCODER_B){ //one step CCW
		stepCCW(); //call function for step CCW
	}else{ //one step CW
		stepCW(); //call function for step CW
	}

	P1IFG &= ~ENCODER_A;	//clear interupt flag*/
}


