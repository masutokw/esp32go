#include "esp32-hal-timer.h"
#include "tb6612.h"
#include <Ticker.h>
int32_t max_steps;
stepper focus_motor, *pmotor;
//extern Ticker focuser_tckr;
extern int8_t focusinv;
extern int focusvolt, focusspd_current, focuspeed_low;
extern hw_timer_t *timer_focus;
#define LOG2(n) (((sizeof(unsigned int) * CHAR_BIT) - 1) - (__builtin_clz((n))))
#define WAVE_SIZE 32
#define MSTEPS 16
#define MSTEPS4 (MSTEPS * 4)
const static uint16_t wave_f[] = { 0, 12, 25, 37, 50, 62, 74, 86, 98, 109, 121, 131, 142, 153, 163, 172, 181,
                                   190, 198, 206, 213, 220, 226, 232, 237, 241, 245, 249, 251, 252, 253, 254, 255 };

uint16_t wave[MSTEPS + 1];
unsigned int mdiv = LOG2(MSTEPS);
void generate_wave(int percent) {
  int n;
  for (n = 0; n <= MSTEPS; n++) {
    wave[n] = (wave_f[n * (WAVE_SIZE / MSTEPS)] * percent) / 255;
  }
}
void init_stepper(stepper *motor) {
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
  pmotor = motor;
}

void move_to(stepper *motor, long int target, int period) {

  motor->target = target;
  motor->state = slew;

  if (motor->position == target) {
//  timerAlarmDisable(timer_focus);
#ifdef STEP_FOCUS
    digitalWrite(ENABLE_FOCUS, DEN_DRIVER);
#else
    WA_O WB_O
#endif

    motor->state = synced;
    motor->resolution = 0;
    motor->target = motor->position;


  } else {
    if ((motor->target) < motor->position)
      motor->resolution = -1;
    else if ((motor->target) > motor->position)
      motor->resolution = 1;
#ifdef STEP_FOCUS
    digitalWrite(ENABLE_FOCUS, EN_DRIVER);
#endif

    timerAlarmWrite(timer_focus, max(100, period), true);
    timerAlarmEnable(timer_focus);
  }
#ifdef M_STEP
  if (period < focuspeed_low) motor->resolution *= (MSTEPS / 4);
#endif
}



//-----------------------------------------------------------------------------------
//move_to for DC Motors bridges tb6612  and DRV883
#ifndef DRV_8833
void move_to(int dir) {
  if (dir < 0) {
    WA_N
  } else if (dir == 0) {
    WA_O
  } else {
    WA_P;
  }
}
#else
void move_to(int dir) {
  digitalWrite(AIN_1, 1);
  digitalWrite(AIN_2, 1);
  if (dir < 0) {
    ledcWrite(2, 0);
    ledcWrite(1, focusspd_current);
  } else if (dir == 0) {
    ledcWrite(2, 0);
    ledcWrite(1, 0);
  } else {
    ledcWrite(1, 0);
    ledcWrite(2, focusspd_current);
  }
}
#endif
//--------------------------------------------------------------------------------------------

inline void step_out(uint8_t step) {
  if (focusinv < 0) step = 7 - step;
  switch (step) {
    case 0:
      WA_P WB_N break;
    case 1:
      WA_P WB_O break;
    case 2:
      WA_P WB_P break;
    case 3:
      WA_O WB_P break;
    case 4:
      WA_N WB_P break;
    case 5:
      WA_N WB_O break;
    case 6:
      WA_N WB_N break;
    case 7:
      WA_O WB_N break;
    default: break;
  }
}

void IRAM_ATTR dostep() {
  uint8_t p, s, j;
  uint16_t pwma, pwmb;
  if ((pmotor->state == slew) && (pmotor->position == pmotor->target)) {
    pmotor->state = synced;
    pmotor->resolution = 0;
#ifdef STEP_FOCUS
    digitalWrite(ENABLE_FOCUS, DEN_DRIVER);
#else
    WA_O WB_O
#endif
    timerAlarmDisable(timer_focus);
  }
  if (pmotor->state != synced) {
    if ((pmotor->backcounter <= 0) || (pmotor->backcounter >= pmotor->backslash))
      pmotor->position += pmotor->resolution;
    else
      pmotor->backcounter += pmotor->resolution;
#ifdef STEP_FOCUS

    if (pmotor->resolution > 0)
      digitalWrite(DIR_OUT_FOCUS, 1);
    else digitalWrite(DIR_OUT_FOCUS, 0);
    //digitalWrite(ENABLE_FOCUS, EN_DRIVER);
    digitalWrite(CLOCK_OUT_FOCUS, 0);
    char pulse_w;
    for (pulse_w = 0; pulse_w < 10; pulse_w++) __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;");
    digitalWrite(CLOCK_OUT_FOCUS, 1);
#else
#ifdef M_STEP
    pmotor->ustep_index += pmotor->resolution;
    if (pmotor->ustep_index < 0) pmotor->ustep_index += MSTEPS4;
    else if (pmotor->ustep_index >= MSTEPS4) pmotor->ustep_index -= MSTEPS4;
    s = pmotor->ustep_index >> mdiv;  // MSTEPS_DIV; /// MSTEPS;
    j = MSTEPS - (p = pmotor->ustep_index % (MSTEPS));
    if (s & 1)  //check odd even step
    {
      pwma = wave[p];
      pwmb = wave[j];
      if (s == 1) {
        WA_P WB_N
      } else {
        WA_N WB_P
      }
    } else {
      pwma = wave[j];
      pwmb = wave[p];

      if (s == 0) {
        WA_P WB_P
      } else {
        WA_N WB_N
      }
    }

    ledcWrite(1, pwma);
    ledcWrite(2, pwmb);

#else
    pmotor->pcounter = (pmotor->pcounter + pmotor->resolution);
    if (pmotor->pcounter < 0)
      pmotor->pcounter += 8;
    pmotor->pcounter %= 8;
    step_out(pmotor->pcounter);
#endif
#endif
  }
}
