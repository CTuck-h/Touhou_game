/*
 * graphics_HAL.c
 *
 *  Created on: Nov 14, 2025
 *      Author: Tuck
 */
//MOST OF THIS CODE IS INSPIRED BY THE CODE PROVIDED FROM THE CLASS LECTURES

#ifndef HAL_GRAPHICS_HAL_C_
#define HAL_GRAPHICS_HAL_C_




// This function initializes all the peripherals except graphics
#include <HAL/graphics_HAL.h>

#include <ti/grlib/grlib.h>

#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"

// Initializing the ADC which resides on SoC
void initADC() {
    ADC14_enableModule();

    ADC14_initModule(ADC_CLOCKSOURCE_SYSOSC,
                     ADC_PREDIVIDER_1,
                     ADC_DIVIDER_1,
                      0);

    // This configures the ADC to store output results
    // in ADC_MEM0 for joystick X and ADC_MEM1 for joystick Y.
    // The sequence is now from ADC_MEM0 to ADC_MEM1.
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true); // <-- UPDATED END MEMORY LOCATION

    // This configures the ADC in manual conversion mode
    // Software will start each conversion.
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
}


void startADC() {
   // Starts the ADC with the first conversion
   // in repeat-mode, subsequent conversions run automatically
   ADC14_enableConversion();
   ADC14_toggleConversionTrigger();
}


// Interfacing the Joystick with ADC (making the proper connections in software)
void initJoyStick() {

    // This configures ADC_MEM0 to store the result from
    // input channel A15 (Joystick X), in non-differential input mode
    ADC14_configureConversionMemory(ADC_X_CHANNEL,
                                   ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                   ADC_INPUT_A15,                 // joystick X (P6.0)
                                   ADC_NONDIFFERENTIAL_INPUTS);

    // This selects the GPIO as analog input for X-axis
    // A15 is multiplexed on GPIO port P6 pin PIN0
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                               GPIO_PIN0,
                                               GPIO_TERTIARY_MODULE_FUNCTION);

    // Assuming Y-axis is connected to A9 (P4.4) on the MSP432
    ADC14_configureConversionMemory(ADC_Y_CHANNEL,
                                   ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                   ADC_INPUT_A9,                  // joystick Y (P4.4)
                                   ADC_NONDIFFERENTIAL_INPUTS);

    // This selects the GPIO as analog input for Y-axis
    // A9 is multiplexed on GPIO port P4 pin PIN4
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                               GPIO_PIN4,
                                               GPIO_TERTIARY_MODULE_FUNCTION);
    // --------------------------------------------
}

void getSampleJoyStick(unsigned *X, unsigned *Y) {
    // ADC runs in continuous mode, read the conversion buffers
    *X = ADC14_getResult(ADC_X_CHANNEL); // Reads MEM0

    // Read the Y channel from MEM1
    *Y = ADC14_getResult(ADC_Y_CHANNEL);
}


#endif /* HAL_GRAPHICS_HAL_C_ */
