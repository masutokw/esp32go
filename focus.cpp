#include "focus.h"
#include "tb6612.h"
int  focuspeed = 7;
int  focuspeed_low = 20;
int focusmax = 50000;
int8_t focusinv = -1;
int focusvolt = 127;
extern stepper focus_motor;
void setfocuserspeed(motor_t* mt, int speed)
{
  // aux_set_period(mt->id, speed);
}

void gotofocuser(int pos) {
#ifndef DC_FOCUS
  move_to(&focus_motor, pos);
#else
  if (pos == focus_motor.max_steps) move_to(1); else if (pos == 0) move_to(-1); else move_to (0);
#endif

}
void gotofocuser( int pos, int speed) {
#ifndef DC_FOCUS
  move_to(&focus_motor, pos, speed);
#else
 ledcWrite(1, speed);
 ledcWrite(2, speed);
  if (pos == focus_motor.max_steps) move_to(1); else if (pos == 0) move_to(-1); else move_to (0);
#endif

}
void stopfocuser(void) {
#ifndef DC_FOCUS
  move_to(&focus_motor, focus_motor.position, focuspeed);
#else
  move_to (0);
#endif
}
void setfocuser(int pos)
{
#ifndef DC_FOCUS
focus_motor.position=focus_motor.target=pos;
#endif
 }
