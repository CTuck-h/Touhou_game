/*
 * graphics_HAL.h
 *
 *  Created on: Nov 14, 2025
 *      Author: Tuck
 */

//MOST OF THIS CODE IS INSPIRED BY THE CODE PROVIDED FROM THE CLASS LECTURES
#ifndef HAL_GRAPHICS_HAL_H_
#define HAL_GRAPHICS_HAL_H_

// Joystick Movement Thresholds (14-bit ADC output range is 0 to 16383)
// where the joystick reads high X and low Y even when centered (i.e., center is biased).
#define UP_THRESHOLD    13000
#define DOWN_THRESHOLD  4000
#define LEFT_THRESHOLD  4000
#define RIGHT_THRESHOLD 13000

// ADC Memory Locations
#define ADC_X_CHANNEL ADC_MEM0
#define ADC_Y_CHANNEL ADC_MEM1

    void initADC();
    void startADC();
    void initJoyStick();
    void getSampleJoyStick(unsigned *X, unsigned *Y);


#endif /* HAL_GRAPHICS_HAL_H_ */
