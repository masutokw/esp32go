#include "tb6612.h"
#include <Ticker.h>
int32_t max_steps;
stepper focus_motor;
extern Ticker  focuser_tckr;
extern int8_t focusinv;
extern int focusvolt, focusspd_current, focuspeed_low;
#define LOG2(n) (((sizeof(unsigned int) * CHAR_BIT) - 1) - (__builtin_clz((n))))
#define WAVE_SIZE 32
#define MSTEPS 8
#define MSTEPS4 (MSTEPS*4)
const static uint16_t wave_f[] = {0, 12, 25, 37, 50, 62, 74, 86, 98, 109, 121, 131, 142, 153, 163, 172, 181,
                                  190, 198, 206, 213, 220, 226, 232, 237, 241, 245, 249, 251, 252, 253, 254, 255
                                 };

uint16_t wave[MSTEPS + 1];
unsigned int mdiv = LOG2(MSTEPS);
void generate_wave(int percent)
{ int n;
  for (n = 0; n <= MSTEPS ; n++) {
    wave[n] = (wave_f[n * (WAVE_SIZE / MSTEPS)] * percent) / 255;
  }
}
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
  //if (period >10) motor.resolution*=4

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
#ifdef M_STEP
  if (period < focuspeed_low) motor->resolution *= (MSTEPS / 4);
#endif
}

#ifdef M_STEP
void do_step(stepper *motor)
{
  uint8_t p, s, j;
  uint16_t  pwma, pwmb;

  if ((motor->position == motor->target) && (motor->resolution != 0))
  {
    motor->resolution = 0;
    // motor->target = 0xF0000000;
    focuser_tckr.detach();
  }

#ifdef BACKSLASH_COMP
  if (dir < 0)
  {
    if (backcounter == 0) position += motor->resolution;
    else backcounter += motor->resolution ;
  }
  else
  {
    if (backcounter == backslash)position += motor->resolution;
    else backcounter += motor->resolution ;
  }

#else
  motor->position += motor->resolution;
#endif // BACKSLASH_COMP
  motor->ustep_index += motor->resolution;
  if ( motor->ustep_index < 0) motor->ustep_index += MSTEPS4;
  else if ( motor->ustep_index >= MSTEPS4)  motor->ustep_index -= MSTEPS4;
  s = motor->ustep_index >> mdiv;// MSTEPS_DIV; /// MSTEPS;
  j = MSTEPS - (p =  motor->ustep_index % (MSTEPS));
  if (s & 1) //check odd even step
  {
    pwma = wave[p];
    pwmb = wave[j];
    if (s == 1)
    {
      WA_P;
      WB_N
    }
    else
    {
      WA_N
      WB_P
    }
  }
  else
  {
    pwma = wave[j];
    pwmb = wave[p];

    if (s == 0)
    {
      WA_P
      WB_P
    }
    else
    {
      WA_N
      WB_N
    }
  }

  ledcWrite(1, pwma);
  ledcWrite(2, pwmb);
}
#else
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
#endif
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
{
#ifndef FYSECT
  if (focusinv < 0)step = 7 - step;
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
#else
  if  (focus_motor.resolution > 0)
    digitalWrite( DIR_OUT_FOCUS, 1);
  else digitalWrite( DIR_OUT_FOCUS, 0);
  digitalWrite(CLOCK_OUT_FOCUS, 0);
  char pulse_w;
  for (pulse_w = 0; pulse_w < 23; pulse_w++) __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;");
  digitalWrite(CLOCK_OUT_FOCUS, 1);
#endif
}
