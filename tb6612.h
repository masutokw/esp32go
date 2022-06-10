#ifndef TB6612_H_INCLUDED
#define TB6612_H_INCLUDED
#include "conf.h"
//MACROS TB6612 HBRIDGE INPUTS
#define WA_P  digitalWrite(AIN_1, 1); digitalWrite(AIN_2, 0);//set  V+ for A winding
#define WA_N  digitalWrite(AIN_1, 0); digitalWrite(AIN_2, 1); //set  V- for A winding
#define WB_P  digitalWrite(BIN_1, 1); digitalWrite(BIN_2, 0); //set  V+ for B winding
#define WB_N  digitalWrite(BIN_1, 0); digitalWrite(BIN_2, 1); //set  V- for B winding
#define WB_O  digitalWrite(BIN_1, 0); digitalWrite(BIN_2, 0); //set  V0 for B winding
#define WA_O  digitalWrite(AIN_1, 0); digitalWrite(AIN_2, 0);//set  V+ for A winding
enum motor_state
{ slew, stop, sync };
typedef struct stepper
{   long int max_steps;
    long int backslash;
    double step_size;
    unsigned int speed;
    long int position;
    long int target;
    long int backcounter;
    enum motor_state state;
    int pcounter;
    short resolution;
    double temperature;
    int period,periodtemp;

}
stepper;
void init_stepper(stepper *motor);
void move_to(stepper *motor,long int  target);
void move_to(stepper *motor, long int  target,int period);
void do_step(stepper *motor);
void step_out(uint8_t step);
#endif
