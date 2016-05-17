// SPI_COMMS.h

/* 	Elbow Orthosis
 * 	Texas A&M University & Texas Instruments
 *
 *  Created on: Fall 2015
 *      Author: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevasevas
*/
#ifndef MSP432_SOURCE_SPI_COMMS_H_
#define MSP432_SOURCE_SPI_COMMS_H_

#include <driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <QMathLib.h>

// USER VARIABLES
#define NUM_CHANNELS 8 // Number of Signal Channels
#define NUM_ACTIVE_CHANNELS 4 // Number of Active Signal Channels

extern int EMG_History; // EMG Data History

extern const uint8_t BICEPS; // Biceps Array Index
extern const uint8_t TRICEPS; // Triceps Array Index
extern const uint8_t FOREARM_I; // Inner Forearm Array Index
extern const uint8_t FOREARM_O; // Outer Forearm Array Index


// CLOCK RATES
uint16_t msp_clk_rate;//48Mhz
uint16_t ads_clk_rate;//2.048 MHz

// FLAGS
extern uint8_t Drdy; //Flag for SPI
extern uint8_t SPI_Cleared; // Flag to Wait Until Channel Clears
extern uint8_t SPI_Connected; //Flag for SPI Initialize Complete
extern double Upper_Arm_Intention;//used for motor
extern double Lower_Arm_Intention;//used for motor
extern int8_t Direction_flag;//used for motor
int Direction_flag_i; // Direction Count

// EMG DATA
extern double EMG[8][100+11-1]; // 8 Channel History (Filtered, Rectified, Averaged)

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
void Convolution(void);


//Compare between the two tricep and bicep
void Comparator(void);



#endif /* MSP432_SOURCE_SPI_COMMS_H_ */
