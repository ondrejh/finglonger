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

void use_force(int8_t data[4],uint8_t force)
{
    uint8_t i;
    for (i=0;i<4;i++) if (force>i) data[i]=0x08; else data[i]=0x7F;
}

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
    init_servo(500);
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
            static uint8_t seqv = 0;
            static uint8_t cnt = 0;
            uint8_t s;
            switch (seqv)
            {
                case 0: // start
                    set_servo_position(500);
                    disp_clearDisplay();
                    seqv++;
                    break;
                case 1: // wait test or clock button
                    if (BTN_CENTER)
                    {
                        seqv++;
                    }
                    break;
                case 2: // move down until -500 or force 4
                    s = sens_read();
                    use_force(data,s);
                    disp_displayAll(data);
                    set_servo_position(get_servo_position()-(4-s)*10);
                    if ((get_servo_position()<=-500) || (s>=4))
                    {
                        cnt=0;
                        seqv++;
                    }
                    break;
                case 3: // wait a while with force 3
                    cnt++;
                    if (cnt>100)
                    {
                        seqv++;
                    }
                    else
                    {
                        s = sens_read();
                        use_force(data,s);
                        disp_displayAll(data);
                        if (s>3) set_servo_position(get_servo_position()+1);
                        if (s<3) set_servo_position(get_servo_position()-(4-s));
                    }
                    break;
                case 4: // move back to top
                    s = sens_read();
                    use_force(data,s);
                    disp_displayAll(data);
                    set_servo_position(get_servo_position()+40);
                    if (get_servo_position()>=500) seqv=0;
                    break;
                default:
                    seqv=0;
                    break;
            }
            /*static uint8_t cnt = 0;
            cnt++;
            if (cnt>=5)
            {
                cnt=0;
                static uint16_t dispcnt=0;
                dispcnt++;
                if (dispcnt>=1000) dispcnt=0;
                uint16_t dispcnt_copy = dispcnt;
                data[3]=dispcnt_copy%10;
                dispcnt_copy/=10;
                data[2]=dispcnt_copy%10;
                dispcnt_copy/=10;
                data[1]=dispcnt_copy%10;
                data[0]=sens_read();
                disp_displayAll(data);
            }*/
        }
    }


    return -1;
}
