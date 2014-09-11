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

#define BTN_MINUTE ((PINC&0x01)==0)
#define BTN_HOUR   ((PINC&0x04)==0)

#define MAX_HOURS 12
#define CNT_SET_TIMEOUT 250 //5s

void use_force(int8_t data[4],uint8_t force)
{
    uint8_t i;
    for (i=0;i<4;i++) if (force>i) data[i]=0x08; else data[i]=0x7F;
}

void use_time(int8_t data[4],uint8_t hours, uint8_t minutes)
{
    data[0]=hours/10;
    data[1]=hours%10;
    data[2]=minutes/10;
    data[3]=minutes%10;
}

#define SEQV_RESET 0
#define SEQV_PUSH 10
#define SEQV_SET_TIME 20
#define SEQV_COUNT_TIME 30

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

    // time
    int8_t hours=0,minutes=0,seconds=0;
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
            static uint8_t pushcnt = 0;
            uint8_t s;
            switch (seqv)
            {
                case SEQV_RESET: // start
                    set_servo_position(500);
                    disp_clearDisplay();
                    hours=0;
                    minutes=0;
                    seconds=0;
                    pushcnt=0;
                    seqv++;
                    break;
                case (SEQV_RESET+1): // wait buttons released
                    if ((!BTN_MINUTE) && (!BTN_HOUR))
                    {
                        cnt++;
                        if (cnt>=5)
                        {
                            cnt=0;
                            seqv++;
                        }
                    }
                    else
                    {
                        cnt=0;
                    }
                    break;
                case (SEQV_RESET+2): // wait buttons
                    if (BTN_HOUR || BTN_MINUTE)
                    {
                        servo_output_enabled = true;
                        seqv=SEQV_SET_TIME;
                    }
                    else
                    {
                        cnt++;
                        if (cnt>=50) servo_output_enabled = false;
                    }
                    break;
                case (SEQV_PUSH): // move down until -500 or force 4
                    s = sens_read();
                    disp_point(false);
                    use_force(data,s);
                    disp_displayAll(data);
                    set_servo_position(get_servo_position()-(4-s)*10);
                    if ((get_servo_position()<=-500) || (s>=4))
                    {
                        cnt=0;
                        seqv++;
                    }
                    break;
                case (SEQV_PUSH+1): // wait a while with force 3
                    cnt++;
                    if (cnt>25)
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
                case (SEQV_PUSH+2): // move back to top
                    s = sens_read();
                    use_force(data,s);
                    disp_displayAll(data);
                    set_servo_position(get_servo_position()+40);
                    if (get_servo_position()>=500)
                    {
                        cnt=0;
                        seqv++;
                    }
                    break;
                case (SEQV_PUSH+3): // wait a while (1s)
                    cnt++;
                    if (cnt>50)
                    {
                        pushcnt++;
                        if (pushcnt>1) seqv=0;
                        else seqv=SEQV_PUSH;
                    }
                    break;
                case (SEQV_SET_TIME): // setup clock timer
                    disp_point(true);
                    seconds=0;
                    cnt=0;
                    seqv++;
                    break;
                case (SEQV_SET_TIME+1): // wait btn press
                    cnt++;
                    if (BTN_MINUTE)
                    {
                        minutes++;
                        if (minutes>=60) minutes=0;
                        seqv++;
                        cnt=0;
                    }
                    else if(BTN_HOUR)
                    {
                        hours++;
                        if (hours>=MAX_HOURS) hours=0;
                        seqv++;
                        cnt=0;
                    }
                    else if (cnt>CNT_SET_TIMEOUT)
                    {
                        if ((hours==0) && (minutes==0)) minutes=1;
                        servo_output_enabled = false;
                        seqv=SEQV_COUNT_TIME;
                    }
                    use_time(data,hours,minutes);
                    disp_displayAll(data);
                    break;
                case (SEQV_SET_TIME+2): // wait btn release
                    if ((!BTN_MINUTE) && (!BTN_HOUR))
                    {
                        cnt++;
                        if (cnt>=5) seqv=SEQV_SET_TIME;
                    }
                    else
                    {
                        if (BTN_MINUTE&&BTN_HOUR) seqv++;
                        cnt=0;
                    }
                    break;
                case (SEQV_SET_TIME+3): // both buttons on (reset?)
                    if (BTN_MINUTE&&BTN_HOUR)
                    {
                        if ((cnt%10)==0)
                        {
                            if (cnt/10%2) disp_clearDisplay();
                            else disp_displayAll(data);
                        }
                        cnt++;
                        if (cnt>=100) seqv=SEQV_RESET;
                    }
                    else seqv--;
                    break;
                case (SEQV_COUNT_TIME): // countdown
                    cnt=1;
                    disp_point(true);
                    if (hours!=0) use_time(data,hours,minutes);
                    else use_time(data,minutes,seconds);
                    disp_displayAll(data);
                    seqv++;
                    break;
                case (SEQV_COUNT_TIME+1): // wait half second
                    cnt++;
                    if (BTN_HOUR || BTN_MINUTE) seqv=SEQV_SET_TIME;
                    else if (cnt>=25)
                    {
                        disp_point(false);
                        disp_displayAll(data);
                        seqv++;
                    }
                    break;
                case (SEQV_COUNT_TIME+2): // wait the rest of second
                    cnt++;
                    if (BTN_HOUR || BTN_MINUTE) seqv=SEQV_SET_TIME;
                    else if (cnt>=50)
                    {
                        seconds--;
                        if (seconds<0)
                        {
                            seconds=59;
                            minutes--;
                            if (minutes<0)
                            {
                                minutes=59;
                                hours--;
                            }
                        }
                        if ((seconds==0)&&(minutes==0)&&(hours==0))
                        {
                            cnt=0;
                            disp_point(true);
                            use_time(data,0,0);
                            servo_output_enabled = true;
                            seqv++;
                        }
                        else seqv=SEQV_COUNT_TIME;
                    }
                    break;
                case (SEQV_COUNT_TIME+3): // start countdown last second
                    if (BTN_HOUR || BTN_MINUTE) seqv=SEQV_SET_TIME;
                    else
                    {
                        if ((cnt%10)==0)
                        {
                            if (cnt/10%2) disp_clearDisplay();
                            else disp_displayAll(data);
                        }
                        cnt++;
                        if (cnt>=50) seqv=SEQV_PUSH;
                    }
                    break;
                default:
                    seqv=SEQV_RESET;
                    break;
            }
        }
    }


    return -1;
}
