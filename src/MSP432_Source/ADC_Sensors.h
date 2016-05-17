/*
 * ADC_Sensor.h
 *
 * 	Elbow Orthosis
 * 	Texas A&M University & Texas Instruments
 *
 *  Created on: Fall 2015
 *      Author: Rafael Salas, Nathan Glaser, Joe Loredo, David Cuevas
 */
#ifndef MSP432_SOURCE_ADC_SENSORS_H_
#define MSP432_SOURCE_ADC_SENSORS_H_


void setup_adc();
void read_adc(uint16_t *adcBuffer);

#endif /* MSP432_SOURCE_ADC_SENSORS_H_ */
