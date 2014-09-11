/**
 * analog servo controll module for arduino (header)
 * see *.c file for details
 **/

#ifndef _SERVO_H_
#define _SERVO_H_

#include <stdbool.h>

#define SERVO_CLOCK 20000
#define SERVO_CENTER 1500
#define SERVO_MAX 500

#define GENERATE_SERVO_TICKS

bool servo_output_enabled;

uint8_t get_servo_ticks(void);
void set_servo_position(int pos);
int get_servo_position(void);
void init_servo(int pos);

#endif
