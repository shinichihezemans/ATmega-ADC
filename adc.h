/*
*	@file adc.h
*	@brief analog digital conversion module
*
*	this module handles measuring analog signals
*
* Created: 3/17/2021 9:45:46 AM
*  Author: Shinichi
*/


#ifndef ADC_H_
#define ADC_H_
#include <stdbool.h>

/*
*	@brief initalizes adc module
*	if init when it is already initialized, it will do the same thing as adc_reset_channels
*	
*	@param channels[] channels on witch to measure
*	@param size size of channels array
*	if channels is set to NULL and size in 0, the adc will measure nothing
*
*	@return returns if init was successful
*/
bool adc_init(uint8_t channels[], uint8_t size);

/*
*	@brief resets on witch channels to measure
*	this method will fail when it is called while measuring
*
*	@param channels[] channels on witch to measure
*	@param size size of channels array
*	if channels is set to NULL and size in 0, the adc will measure nothing
*	@return returns if reset was successful
*/
bool adc_reset_channels(uint8_t channels[], uint8_t size);

/*
*	@brief starts measuring analog signal
*	@return return false when channels is NULL and true wen measuring starts
*/
bool adc_start_measure();

/*
*	@brief checks if adc module is done measuring
*	@return returns if adc module is done measuring
*/
bool adc_is_done();

/*
*	@brief gets value the adc module measured
*	@param index the index at witch the channel was when init or reset was called
*	@return returns the value the adc module measured
*/
uint8_t adc_get_value(uint8_t index);

#endif /* ADC_H_ */