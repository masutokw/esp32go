#include <cmath>
#include "focus.h"
#include "tb6612.h"
#define GOTO_FOCUS_PERIOD 2000
#ifdef HIRES_TIMER
#define TIMER_PER 1.0e+6
#else
#define TIMER_PER 1.0e+4
#endif
int focusspd_current = 0;  //dc
int dcfocus = 0;
int focusvolt = 127;
extern stepper *pmotor, aux_motor, focus_motor;
void setfocuserspeed(motor_t* mt, int speed) {
  // aux_set_period(mt->id, speed);
}

void gotofocuser(int pos) {
  if (dcfocus != 1)
    move_to(pmotor, pos, (pmotor->speed) * 2);
  else if (pos == pmotor->max_steps) move_to(1);
  else if (pos == 0) move_to(-1);
  else move_to(0);
}

void gotofocuser(int pos,stepper *motor) {
  if (dcfocus != 1)
    move_to(motor, pos, (motor->speed) * 2);
  else if (pos == motor->max_steps) move_to(1);
  else if (pos == 0) move_to(-1);
  else move_to(0);
}

void gotofocuser(int pos, int speed) {
  if (dcfocus != 1) {
    move_to(pmotor, pos, speed);
  } else {
#ifndef DRV_8833
    ledcWrite(1, speed);
    ledcWrite(2, speed);
#endif
    focusspd_current = speed;
    if (pos == pmotor->max_steps) move_to(1);
    else if (pos == 0) move_to(-1);
    else move_to(0);
  }
}

void gotofocuser(int pos, int speed,stepper *motor) {
  if (dcfocus != 1) {
    move_to(motor, pos, speed);
  } else {
#ifndef DRV_8833
    ledcWrite(1, speed);
    ledcWrite(2, speed);
#endif
    focusspd_current = speed;
    if (pos == motor->max_steps) move_to(1);
    else if (pos == 0) move_to(-1);
    else move_to(0);
  }
}
void stopfocuser(void) {
  if (dcfocus != 1)
    move_to(pmotor, pmotor->position, pmotor->speed);
  else
    move_to(0);
}
void stopfocuser(stepper *motor) {
  if (dcfocus != 1)
    move_to(motor, motor->position, motor->speed);
  else
    move_to(0);
}
void setfocuser(int pos) {
  if (dcfocus != 1)
    pmotor->position = pmotor->target = pos;
}
void setfocuser(int pos,stepper *motor) {
  if (dcfocus != 1)
    motor->position = motor->target = pos;
}
int rconv(int period) {
  if (dcfocus)
    return round(fmin(period / 2.55, 100));

  if (period == 0)
    return 65535;
  else
    return trunc((TIMER_PER/ period));
}

int fconv(int period) {
  if (dcfocus)
    return round(fmin(2.55 * period, 255.0));

  if (period == 0)
    return 65535;
  else
    return trunc((TIMER_PER / period));
}
/*
int readauxconfig(void) {
  File f;
  if (!SPIFFS.exists(AUX_FILE)) return -1;

  f = SPIFFS.open(AUX_FILE, FILE_READ);

  String s = f.readStringUntil('\n');
  focus_motor.max_steps = s.toInt();
  s = f.readStringUntil('\n');
  focus_motor.speed_low = s.toInt();
  s = f.readStringUntil('\n');
  focus_motor.speed = s.toInt();
  s = f.readStringUntil('\n');
  int tmpfocus = s.toInt();
  focus_motor.inv = (tmpfocus > 0) ? 0 : 1;
  focus_motor.pwm = abs(tmpfocus);
  s = f.readStringUntil('\n');
  aux_motor.max_steps = s.toInt();
  s = f.readStringUntil('\n');
  aux_motor.speed_low = s.toInt();
  s = f.readStringUntil('\n');
  aux_motor.speed = s.toInt();
  s = f.readStringUntil('\n');
  tmpfocus = s.toInt();
  aux_motor.inv = (tmpfocus > 0) ? 0 : 1;
  aux_motor.pwm = abs(tmpfocus);
  s = f.readStringUntil('\n');
  dcfocus = s.toInt();
#ifdef M_STEP
generate_wave( focus_motor.pwm);
#endif
  return 0;
}*/

int readauxconfig(void) {
  File f;
  int tlow, tspeed;
  if (!SPIFFS.exists(AUX_FILE)) return -1;

  f = SPIFFS.open(AUX_FILE, FILE_READ);

  String s = f.readStringUntil('\n');
  focus_motor.max_steps = s.toInt();
  s = f.readStringUntil('\n');
  tlow = s.toInt();
  s = f.readStringUntil('\n');
  tspeed = s.toInt();
  s = f.readStringUntil('\n');
  int tmpfocus = s.toInt();
  focus_motor.inv = (tmpfocus > 0) ? 0 : 1;
  focus_motor.pwm = abs(tmpfocus);
  s = f.readStringUntil('\n');
  aux_motor.max_steps = s.toInt();
  s = f.readStringUntil('\n');
  aux_motor.speed_low = fconv(s.toInt());
  s = f.readStringUntil('\n');
  aux_motor.speed = fconv(s.toInt());
  s = f.readStringUntil('\n');
  tmpfocus = s.toInt();
  aux_motor.inv = (tmpfocus > 0) ? 0 : 1;
  aux_motor.pwm = abs(tmpfocus);
  s = f.readStringUntil('\n');
  dcfocus = s.toInt();
  focus_motor.speed_low = fconv(tlow);
  focus_motor.speed = fconv(tspeed);
  s = f.readStringUntil('\n');
  aux_motor.id=s.toInt();
#ifdef M_STEP
  generate_wave(focus_motor.pwm);
#endif
  return 0;
}
