#include "focus.h"
#include "tb6612.h"
#define GOTO_FOCUS_PERIOD 2000
int focusspd_current = 0;//dc
int dcfocus = 0;
int focusvolt = 127;
extern stepper *pmotor;
void setfocuserspeed(motor_t* mt, int speed)
{
  // aux_set_period(mt->id, speed);
}

void gotofocuser(int pos) {
  if(dcfocus!=1)
    move_to(pmotor, pos,GOTO_FOCUS_PERIOD);
  else
    if (pos == pmotor->max_steps) move_to(1); else if (pos == 0) move_to(-1); else move_to (0);

}
void gotofocuser( int pos, int speed) {
  if(dcfocus!=1)
  {
    move_to(pmotor, pos, speed);
  }
  else
  {
#ifndef STEP_FOCUS
#ifndef DRV_8833
    ledcWrite(1, speed);
    ledcWrite(2, speed);
#endif
#endif
    focusspd_current = speed;
    if (pos == pmotor->max_steps) move_to(1); else if (pos == 0) move_to(-1); else move_to (0);
  }

}
void stopfocuser(void) {
  if(dcfocus!=1) 
    move_to(pmotor, pmotor->position, pmotor->speed);
  else
    move_to (0);
}
void setfocuser(int pos)
{
  if(dcfocus!=1)
  pmotor->position = pmotor->target = pos;

}
