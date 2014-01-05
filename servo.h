#define SERVO_CLOCK 20000
#define SERVO_CENTER 1500
#define SERVO_MAX 500

void set_servo_position(int pos);
int get_servo_position(void);
void init_servo(int pos);
