/**
 * analog servo controll module for arduino
 *
 * date created: January 2014
 * author: ondrejh.ck@email.cz
 *
 * description:
 * HW controll of analog servo. Modulating pulses with variable width.
 * Pulse frequency 1/20ms, width 1.0 - 2.0ms. Resolution 1000 steps.
 * Module uses Timer1 output compare unit.
 * Clock frequency of arduino should be 16MHz (otherwise it'd be nessesarry to change timing).
 * Servo pulse input is connected to PB1/OC1A output of MCU (arduino IO9).
 * Timer and outputs should be initialized by init_servo() first.
 * Actual servo position can be set by calling set_servo_position( pos ).
 * The integer value varies from -SERVO_MAX to +SERVO_MAX (see module header).
 * Higher or lower value is reduced to fit this range.
 * The position can be read by get_servo_position().
 *
 *   arduino         servo
 *  _________       _______
 * |         |     |       |
 * |     IO9 |---->| IN    |
 * |_________|     |_______|
 *
 * note:
 * The module can be easily extended to second servo output on OC1B.
 **/

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

