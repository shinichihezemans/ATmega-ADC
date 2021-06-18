/*
* adc.c
*
* Created: 3/17/2021 9:45:31 AM
*  Author: Shinichi
*/
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "adc.h"

#ifndef DEBUG
#define DEBUG true
#endif

#define ADCSRA_ADIE 3 //adc interrupt enable
#define ADCSRA_ADSC 6 //adc start
#define ADCSRA_ADEN 7 //adc enable

static uint8_t *_channels;
static uint8_t *_values;
static uint8_t _size = 0;
static uint8_t current_index = 0;
static bool _done = false;

static bool reset_channels(uint8_t channels[], uint8_t size);
static bool validate_channels(uint8_t channels[], uint8_t size);
static bool validate_channel(uint8_t channel);

bool adc_init(uint8_t channels[], uint8_t size){
	static bool initialized = false;
	//check if adc already initialized
	if (initialized)
	return adc_reset_channels(channels, size);
	
	//try to set the channels
	if (!reset_channels(channels, size))
	return false;
	
	//internal, left adjust, start on 0
	ADMUX = 0b11100000;
	//adc on, start, no free running, interrupt enable.
	ADCSRA = (1 << ADCSRA_ADIE) | (1 << ADCSRA_ADEN) | 0b00000110;
	
	_done = true;
	initialized = true;
	return true;
}

bool adc_reset_channels(uint8_t channels[], uint8_t size){
	//check is maybe not needed. if changed while measuring, the program will not crash but might give garbage values
	if (_done)
	return reset_channels(channels, size);
	else
	return false;
}

/*
*	@brief resets on witch channels to measure analog signal
*	@param channels[] channels on witch to measure
*	@param size size of channels array
*	if channels is set to NULL and size in 0, the adc will measure nothing
*
*	@return returns is reset was successful
*/
bool reset_channels(uint8_t channels[], uint8_t size){
	//check if user wants to deliberately measure nothing
	//if channels is NULL and size is not or size is 0 but channels is not, something went wrong so reset channels won't work (next if filters these out)
	if (!channels && size == 0)
	{
		free(_channels);
		free(_values);
		
		_size = 0;
		return true;
	}
	
	//check if given channels are not null and the size is between 1 and 32 or if channels are not valid, if so return false
	if (!channels || size <= 0 || size > 32 || !validate_channels(channels, size))
	return false;
	
	//check if _channels is already initialized or freed
	if (_channels)
	{
		//re allocate
		uint8_t *temp_channel_ptr = (uint8_t *) realloc(_channels, (sizeof(uint8_t)) * size);
		uint8_t *temp_value_ptr = (uint8_t *) realloc(_values, (sizeof(uint8_t)) * size);
		
		//TODO what if one fails but the other one doesn't
		if (!temp_channel_ptr || !temp_value_ptr)
		return false;
		
		//set the new pointers
		_channels = temp_channel_ptr;
		_values = temp_value_ptr;
		
		//copy new channels to newly allcoated memory
		memcpy(_channels, channels, size);
		
		_size = size;
		return true;
	}
	else
	{
		//initialize if first time or freed
		_channels = (uint8_t *) malloc((sizeof(uint8_t)) * size);
		_values = (uint8_t *) malloc((sizeof(uint8_t)) * size);
		
		//if malloc fails free memory
		if (!_channels || !_values)
		{
			free(_channels);
			free(_values);
			return false;
		}
		
		//copy new channels to newly allcoated memory
		memcpy(_channels, channels, size);
		
		_size = size;
		return true;
	}
}

/*
*	@brief validates if channels are valid
*	@param channels[] channels to check
*	@param size the size of channels array
*/
static bool validate_channels(uint8_t channels[], uint8_t size){
	//TODO check for repeating channels
	for (int index = 0; index < size; index++)
	{
		if (!validate_channel(channels[index]))
		return false;
	}
	return true;
}

/*
*	@brief checks is channel is valid
*	channel must be in [0, 32]
*/
static bool validate_channel(uint8_t channel){
	return (channel >= 0 && channel < 32);
}

bool adc_start_measure(){
	//check if adc ca start measuring
	if (!_channels || !_done)
	return false;
	
	_done = false;
	
	current_index = 0;
	//clear pin
	ADMUX &= 0b11100000;
	//set pin to first channel
	ADMUX |= _channels[current_index];
	//start next conversion
	ADCSRA |= (1 << ADCSRA_ADSC);
	
	return true;
}

bool adc_is_done(){
	return _done;
}

uint8_t adc_get_value(uint8_t index){
	if (_done && index >= 0 && index < _size)
	return _values[index];
	return 0;
}

ISR( ADC_vect){
	
	//filter wrong states out
	if (!_channels || _size == 0 || current_index >= _size || current_index < 0)
	{
		_done = true;
		#if DEBUG
		PORTE |= 0x08;
		#endif
		//return;
	}
	else
	{
		uint8_t channel = ADMUX & 0b00011111;

		if (channel == _channels[current_index])
		{
			_values[current_index] = ADCH;
			//check if end of array
			if (current_index >= _size - 1)
			{
				_done = true;
				//return;
			}
			else
			{
				//advance index
				current_index++;
				//clear channel
				ADMUX &= 0b11100000;
				//set pin to next channel
				ADMUX |= _channels[current_index];
				//start next conversion
				ADCSRA |= 0b01000000;
				//return;
			}
		}
		else
		{
			//runtime change
			//set debug pin
			#if DEBUG
			PORTE |= 0x04;
			#endif
			
			//start at beginning
			current_index = 0;
			//clear channel
			ADMUX &= 0b11100000;
			//set pin to next channel
			ADMUX |= _channels[current_index];
			//start next conversion
			ADCSRA |= 0b01000000;
			//return;
		}
	}
}