#ifndef FOCUS_H_INCLUDED
#define FOCUS_H_INCLUDED
#include "misc.h"
#include "motor.h"
void setfocuserspeed(motor_t* mt,int speed);
void gotofocuser(int pos,int speed);
void gotofocuser(int pos);
void stopfocuser(void);
void setfocuser(int pos);
#endif
