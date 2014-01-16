// presure sensor module

#include <avr/io.h>
#include "sens.h"

#define SENS_ADC_CHANNEL 3

const uint16_t TouchTab[4] = {921, //4.5V
                              512, //2.5V
                              307, //1.5V
                              205}; //1.0V

// local funtion prototypes
uint16_t adc_read(uint8_t ch);

// initialize presure sensor
void init_sens(void)
{
    // initialize ADC
    // AREF = AVcc
    ADMUX = (1<<REFS0);
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read presure sensor
int8_t sens_read(void)
{
    uint16_t val = adc_read(SENS_ADC_CHANNEL);

    int8_t ret = 0;
    while(val<=TouchTab[ret])
    {
        ret++;
        if (ret>3) break;
    }

    return ret;
}

// read adc value
uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing

    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);

    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));

    return (ADC);
}
