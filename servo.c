#include <avr/io.h>
#include "servo.h"

int position = 0;

// set servo output
void set_servo_position(int pos)
{
    int lpos = pos;

    // test limits
    if (lpos>SERVO_MAX)
    {
        lpos=SERVO_MAX;
    }
    else if (lpos<-SERVO_MAX)
    {
        lpos=-SERVO_MAX;
    }
    // set position
    OCR1B = SERVO_CENTER + lpos;
    position = lpos;
}

// servo position return function
int get_servo_position(void)
{
    return position;
}

// init servo
void init_servo(int pos)
{
    // stop timer
    TCCR1B = 0;

    // servo timing
    OCR1A = SERVO_CLOCK;
    set_servo_position(pos);

    // timer init
    TCCR1A = (2<<COM1B0) | (1<<WGM10);
    TCCR1B = (2<<CS10) | (1<<WGM13);
    TCCR1C = (1<<FOC1B);

    // servo output
    DDRB |= (1<<2);
    PORTB |= (1<<2);
}

