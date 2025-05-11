#ifndef FOCUS_H_INCLUDED
#define FOCUS_H_INCLUDED
#include "misc.h"
#include "motor.h"
#include "tb6612.h"
void setfocuserspeed(motor_t* mt,int speed);
void gotofocuser(int pos,int speed);
void gotofocuser(int pos,stepper *motor);
void gotofocuser(int pos, int speed,stepper *motor);
void gotofocuser(int pos);
void stopfocuser(void);
void stopfocuser(stepper *motor);
void setfocuser(int pos);
int readauxconfig(void) ;
int rconv( int period);
int fconv( int period);
void gotoindex(uint8_t index) ;
#endif
