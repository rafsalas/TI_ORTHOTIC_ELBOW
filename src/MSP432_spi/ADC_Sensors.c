
//******************************************************************************
#include "msp.h"
#include <driverlib.h>
#include <stdint.h>
#include "ADC_Sensors.h"
#include <string.h>

// Hard-Coded ADC Calibration
uint16_t POT1_ADC90 = 8500; // Potentiometer 1 ADC Value at 90 Degrees (Inside Pot)
uint16_t POT1_ADC180 = 16383; // Potentiometer 1 ADC Value at 180 Degrees (Inside Pot)
uint16_t POT2_ADC90 = 8500; // Potentiometer 2 ADC Value at 90 Degrees (Outside Pot)
uint16_t POT2_ADC180 = 16383; // Potentiometer 2 ADC Value at 180 Degrees (Outside Pot)


void setup_adc(){

    /* Setting reference voltage to 2.5  and enabling reference */
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();

    MAP_ADC14_enableModule();
    //~47 KHz
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_32, ADC_DIVIDER_2,  0);
    /* Configuring GPIOs for Analog In */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,
            GPIO_PIN5 | GPIO_PIN4 | GPIO_PIN3 | GPIO_PIN2, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM7 (A0 - A1)  with no repeat)
     * with internal 2.5v reference */

    MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM3, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A0, false);
    MAP_ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A1, false);
    MAP_ADC14_configureConversionMemory(ADC_MEM2, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A2, false);
    MAP_ADC14_configureConversionMemory(ADC_MEM3, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A3, false);

    /* Setting up the sample timer to automatically step through the sequence convert.*/
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Triggering the start of the sample */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();

}

void read_adc(uint16_t *adcBuffer){
	uint16_t tempBuffer [4];

    /* Zero-filling buffer */
    memset(tempBuffer, 0x00, 4);
    MAP_ADC14_getMultiSequenceResult(tempBuffer);
    adcBuffer[0] = (180.0-90.0)/(POT1_ADC180-POT1_ADC90)*((uint16_t)tempBuffer[0]-POT1_ADC90)+90.0; // Potentiometer 1
    adcBuffer[1] = (180.0-90.0)/(POT2_ADC180-POT2_ADC90)*((uint16_t)tempBuffer[1]-POT2_ADC90)+90.0; // Potentiometer 2
    adcBuffer[2] = tempBuffer[2]; // FSR 1
    adcBuffer[3] = tempBuffer[3]; // FSR 2
}
