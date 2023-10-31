#include "tb6612.h"
#include <Ticker.h>
int32_t max_steps;
stepper focus_motor;
extern Ticker  focuser_tckr;
extern int8_t focusinv;
extern int focusvolt, focusspd_current;
void init_stepper(stepper *motor)
{
  motor->max_steps = 50000;
  motor->backslash = 0;
  motor->step_size = 0.1;
  motor->speed = 10;
  // motor->position = 800;
  motor->target = 0;
  motor->backcounter = 0;
  motor->resolution = 0;
  motor->state = stop;
  motor->pcounter = 0;
  motor->resolution;
  motor->temperature = 25;
  motor->period = 0;
  motor->periodtemp = 0;
}

void move_to(stepper *motor, long int  target)
{
  focuser_tckr.attach_ms(5, do_step, &focus_motor);
  motor->target = target;
  motor->state = slew;
  if ( motor->position == target) {
    focuser_tckr.detach();
    motor->state = synced;
    motor->resolution = 0;
    motor->target = motor->position;
    WA_O; WB_O
  }
  else if ((motor->target) < motor->position)
    motor->resolution = -1;
  else if ((motor->target) > motor->position)
    motor->resolution = 1;


}
void move_to(stepper *motor, long int  target, int period)
{
  focuser_tckr.attach_ms(max(1, period), do_step, &focus_motor);
  motor->target = target;
  motor->state = slew;
  if ( motor->position == target) {
    focuser_tckr.detach();
    motor->state = synced;
    motor->resolution = 0;
    motor->target = motor->position;
    WA_O; WB_O

  }
  else if ((motor->target) < motor->position)
    motor->resolution = -1;
  else if ((motor->target) > motor->position)
    motor->resolution = 1;


}

void do_step(stepper *motor)
{
  if ((motor->state == slew) && (motor->position == motor->target))
  {
    motor->state = synced;
    motor->resolution = 0;
    WA_O; WB_O
    focuser_tckr.detach();
  }
  if (motor->state != synced) {
    if (( motor->backcounter <= 0) || ( motor->backcounter >= motor->backslash))
      motor->position += motor->resolution;
    else
      motor->backcounter += motor->resolution ;

    motor->pcounter = (motor->pcounter + motor->resolution);
    if (motor->pcounter < 0)
      motor->pcounter += 8;
    motor->pcounter %= 8;
    step_out(motor->pcounter);
  }

}
#ifndef DRV_8833
void move_to (int dir)
{ if (dir < 0) {
    WA_N
  } else if (dir == 0) {
    WA_O
  } else
  {
    WA_P;
  }
}
#else
void move_to (int dir)
{ digitalWrite(AIN_1, 1);
  digitalWrite(AIN_2, 1);
  if (dir < 0) {
    ledcWrite(2, 0);
    ledcWrite(1, focusspd_current );
  } else if (dir == 0) {
    ledcWrite(2, 0);
    ledcWrite(1, 0);
  } else
  {
    ledcWrite(1, 0);
    ledcWrite(2, focusspd_current);
  }
}
#endif


void step_out(uint8_t step)
{ if (focusinv < 0)step = 7 - step;
  switch (step) {
    case 0: WA_P; WB_N; break;
    case 1: WA_P; WB_O; break;
    case 2: WA_P; WB_P; break;
    case 3: WA_O; WB_P; break;
    case 4: WA_N; WB_P; break;
    case 5: WA_N; WB_O; break;
    case 6: WA_N; WB_N; break;
    case 7: WA_O; WB_N; break;
    default: break;

  }
}
