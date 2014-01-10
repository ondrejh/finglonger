#define F_CPU 16000000

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "uart.h"
#include "servo.h"
#include "disp.h"

#define BTN_RIGHT  ((PINC&0x01)==0)
#define BTN_CENTER ((PINC&0x02)==0)
#define BTN_LEFT   ((PINC&0x04)==0)

/** main program body */
int main(void)
{
    /// board settings
    PORTB &= ~(1<<5);
    DDRB  |=  (1<<5);
    // button inputs
    DDRC  &= ~0x07;
    PORTC |=  0x07;

    // initializations
    init_uart();
    init_servo(0);
    init_disp();

    // interrupt enable
    sei();

    // display test
    int8_t data[4]={1,2,3,4};
    disp_point(true);
    disp_displayAll(data);
    disp_displayOne(2,0);

    set_sleep_mode(SLEEP_MODE_IDLE);

    while(1)
    {
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }


    return -1;
}
