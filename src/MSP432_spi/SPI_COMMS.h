/*
 * SPI_COMMS.h
 *
 *  Created on: Feb 17, 2016
 *      Author: rafael
 */

#ifndef SPI_COMMS_H_
#define SPI_COMMS_H_

#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <QMathLib.h>
#include "printf.h"

// USER VARIABLES
#define NUM_CHANNELS 8 // Number of Signal Channels
#define EMG_History 50 // EMG Data History

// CLOCK RATES
uint16_t msp_clk_rate;//48Mhz
uint16_t ads_clk_rate;//2.048 MHz

// FLAGS
extern uint8_t Drdy; //Flag for SPI
extern uint8_t SPI_Cleared; // Flag to Wait Until Channel Clears
extern uint8_t SPI_Connected; //Flag for SPI Initialize Complete



// EMG DATA
extern double EMG[8][50]; // 8 Channel History (Filtered, Rectified, Averaged)


void spi_setup();
void spi_start();
void spi_register_setting();
void spi_write_registers();
void spi_read_registers();
void drdy_setup();

// COLLECT DATA FROM SPI CHANNEL
void SPI_Collect_Data();



// 2's COMPLEMENT CONVERSION
int32_t twos_to_signed (uint32_t msb, uint32_t mid, uint32_t lsb);


// HIGH PASS FILTER
double High_Pass(int channel, double data_in, uint32_t overhead);

// BAND STOP FILTER
double Band_Stop(int channel, double data_in, uint32_t overhead);

// Exclude Erroneous Data
double Error_Check(int channel, double input, double average, uint32_t overhead);

// MOVING AVERAGE FILTER
double Moving_Average(int channel, double data_in, uint32_t overhead);


// NORMALIZATION ROUTINE
double Normalize(int channel, double data_in, double data_in_old, uint32_t overhead);




#endif /* SPI_COMMS_H_ */
