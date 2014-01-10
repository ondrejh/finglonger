#define F_CPU 16000000

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "uart.h"
#include "servo.h"
#include "disp.h"
#include "sens.h"

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
    init_sens();

    // interrupt enable
    sei();

    // display test
    int8_t data[4]={0,0,0,0};
    /*disp_point(true);
    disp_displayAll(data);
    disp_displayOne(2,0);*/
    disp_point(false);

    set_sleep_mode(SLEEP_MODE_IDLE);

    while(1)
    {
        sleep_enable();
        sleep_cpu();
        sleep_disable();
        if (get_servo_ticks())
        {
            static uint8_t cnt = 0;
            cnt++;
            if (cnt>=5)
            {
                cnt=0;
                static uint16_t dispcnt=0;
                dispcnt++;
                if (dispcnt>=10000) dispcnt=0;
                uint16_t dispcnt_copy = dispcnt;
                data[3]=dispcnt_copy%10;
                dispcnt_copy/=10;
                data[2]=dispcnt_copy%10;
                dispcnt_copy/=10;
                data[1]=dispcnt_copy%10;
                dispcnt_copy/=10;
                data[0]=dispcnt_copy;
                disp_displayAll(data);
            }
        }
    }


    return -1;
}
