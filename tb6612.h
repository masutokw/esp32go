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
//#define WB_O  digitalWrite(BIN_1, 1); digitalWrite(BIN_2, 1); //set  V0 for B winding
//#define WA_O  digitalWrite(AIN_1, 1); digitalWrite(AIN_2, 1);//set  V+ for A winding
enum motor_state
{ slew, stop, synced };
typedef struct stepper
{   long int max_steps;
    long int backslash;
    unsigned int speed_low, speed,speed_target,speed_counter;
    long int position;
    long int target;
    long int backcounter;
    enum motor_state state;
    int pcounter;
    short resolution;
    double temperature;
    int ustep_index;
    int8_t inv;
    uint8_t dir,step,enable,pwm,id;
   
    

}
stepper;
void generate_wave(int percent);
void init_stepper(stepper *motor,uint8_t dir,uint8_t step,uint8_t enable);

void move_to(stepper *motor, long int  target,int period);
void move_to (int dir);

void IRAM_ATTR do_step();
void step_out(uint8_t step);
void IRAM_ATTR dostep(stepper *motor);
void IRAM_ATTR aux_ISR();
#endif
