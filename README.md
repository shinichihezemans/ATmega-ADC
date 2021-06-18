# ATmega-ADC
Small library to read multiple analog value's. 
This Library shines in reading a series of anolog values.

## Usage
First call ```adc_init``` to initiate the ADC module an set the channels to listen to.\
The channels can be found in the ATmega documentation.\
Before calling ```adc_start``` enable the interrupt vector by calling ```sei()```.\
When ```adc_start``` is called all of the channels are read and cycled through by the interrupt service routine. \
When all analog values are read, ```adc_is_done``` will return true and the values can be retrieved by calling ```adc_get_value```.

This library supports 8-bit values. This library cycles through the channels using the interrupt service routine. This limits the waiting to only once instead of waiting for every analog to digital conversion. 