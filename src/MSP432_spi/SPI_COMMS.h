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

uint16_t msp_clk_rate;//48Mhz
uint16_t ads_clk_rate;//2.048 MHz
extern uint8_t Drdy; //Flag for SPI
extern uint8_t SPI_Cleared; // Flag to Wait Until Channel Clears
extern uint8_t SPI_Connected; //Flag for SPI Initialize Complete
extern int32_t sample[50];
extern uint32_t filtered[50];


uint8_t spi_index; // used to keep track of the first diminsion of the spi_dta matrix

#define window 50 //moving average window
#define NUM_CHANNELS 8 // Number of Signal Channels
#define NUM_BOARDS 1 // Number of ADS1299 Boards

static uint32_t sum = 0; //sum for moving average

void spi_setup();
void read_message(int32_t sample[50], uint32_t filtered[50]);
void spi_start();
void spi_register_setting();
void spi_write_registers();
void spi_read_registers();
void drdy_setup();
void conditionSamples(int32_t samples[50], uint32_t filtered[50]);
int32_t twos_to_signed (uint32_t msb, uint32_t mid, uint32_t lsb);
void SPI_Collect_Data();

#endif /* SPI_COMMS_H_ */
