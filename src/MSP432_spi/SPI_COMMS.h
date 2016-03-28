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
#define NUM_ACTIVE_CHANNELS 4 // Number of Active Signal Channels
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

// NORMALIZATION ROUTINE
extern double EMG_max[8]; // Maximum EMG Signal
extern double EMG_min[8];// = {100, 100, 100, 100, 100, 100, 100, 100}; // Minimum EMG Signal
extern double EMG_min_i[8];// = {51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51, 51+51+51}; // Minimum EMG Signal Index


void spi_setup();
void spi_start();
void spi_register_setting();
void spi_write_registers();
void spi_read_registers();
void drdy_setup();

// COLLECT DATA FROM SPI CHANNEL
void SPI_Collect_Data();

// CONDITION DATA FROM SPI CHANNEL
void EMG_Condition_Data();

// 2's COMPLEMENT CONVERSION
int32_t twos_to_signed (uint32_t msb, uint32_t mid, uint32_t lsb);

// CONVOLUTION COMPUTATION
// Trim Unstable + Irrelevant Edges
void Convolution2();
void Convolution(uint32_t trim, double* a, uint32_t N_a, double* b, uint32_t N_b, double* p);

//Compare between the two tricep and bicep
void Comparator();


#endif /* SPI_COMMS_H_ */
