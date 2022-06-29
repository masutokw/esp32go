#include "focus.h"

int  focuspeed=7;
int  focuspeed_low=20;
int focusmax=50000;
int8_t focusinv=-1;
int focusvolt=127;
void setfocuserspeed(motor_t* mt,int speed)
{
 // aux_set_period(mt->id, speed);
}
void gotofocuser(motor_t* mt,int pos,int speed) {
  int focusdest;
  int count, fspeed;
  if (pos < 1) pos = 0;
  focusdest = pos;
//  set_aux_target(mt->id, pos);

 // setfocuserspeed(mt,speed * sign(pos - mt->auxcounter));

}
void stopfocuser(motor_t* mt) {
 // setfocuserspeed(mt,0);
}
