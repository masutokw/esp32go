#include "focus.h"
#include "tb6612.h"
int  focuspeed = 7;
int  focuspeed_low = 20;
int focusspd_current = 0;
int focusmax = 50000;
int dcfocus = 0;
int8_t focusinv = -1;
int focusvolt = 127;
extern stepper focus_motor;
void setfocuserspeed(motor_t* mt, int speed)
{
  // aux_set_period(mt->id, speed);
}

void gotofocuser(int pos) {
  if(dcfocus!=1)
    move_to(&focus_motor, pos);
  else
    if (pos == focus_motor.max_steps) move_to(1); else if (pos == 0) move_to(-1); else move_to (0);

}
void gotofocuser( int pos, int speed) {
  if(dcfocus!=1)
  {
    move_to(&focus_motor, pos, speed);
  }
  else
  {
#ifndef DRV_8833
    ledcWrite(1, speed);
    ledcWrite(2, speed);
#endif
    focusspd_current = speed;
    if (pos == focus_motor.max_steps) move_to(1); else if (pos == 0) move_to(-1); else move_to (0);
  }

}
void stopfocuser(void) {
  if(dcfocus!=1) 
    move_to(&focus_motor, focus_motor.position, focuspeed);
  else
    move_to (0);
}
void setfocuser(int pos)
{
  if(dcfocus!=1)
    focus_motor.position = focus_motor.target = pos;

}
