#include <math.h>
#include "motor.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
//#define DIR_OUT_AZ 19
//#define DIR_OUT_ALT 2
extern int aux_count;
extern uint64_t period_az, period_alt;
extern int   azcounter, altcounter, azdir, altdir;
extern hw_timer_t * timer_az;
extern hw_timer_t * timer_alt;
int sign(double t)
{
  if (t < 0.0) return -1;
  else if (t == 0.0 )return 0;
  else return 1;

}

void init_motor(motor_t* mt, char ref, int maxcounter, double spd, double tick, double maxspd, double accel,int back)
{
  mt->speed = 0;
  mt->targetspeed = spd;
  mt->position = 0;
  mt->timertick = tick * 1e-6;
  mt->maxcounter = maxcounter; //8000*6*180;//
  mt->resolution = (2.0 * M_PI) / mt->maxcounter; //radians
  if (accel > 0.0)mt->acceleration = accel * SEC_TO_RAD ;
  else mt->acceleration = 20e-4;
  mt->id = ref;
  mt->slewing = 0;
  mt->maxspeed = maxspd;
 // set_motor_max_counter(ref, maxcounter);
  mt->backslash=back;
  setbackslash(mt,back);
}

inline void setspeed(motor_t* mt , double tspeed)
{
  uint64_t base;//ispeed
  // mt->current_speed=tspeed;
  //    base=0;    //timer2 preload
  //    postscaler=0;  //PIC timer2 iterations
  if (tspeed != 0.0)
  {
    base = fabs((mt->resolution) / ( tspeed * (mt->timertick)));

  }
  else base = 100000;
  if (base > 10000000) base = 10000000;
  if (mt->id) {
    azdir = sign(tspeed);
    if (azdir > 0) digitalWrite( DIR_OUT_AZ, 0) ; else digitalWrite( DIR_OUT_AZ, 1 );
    if (base != period_az)
    { timerAlarmWrite(timer_az, period_az = base, true);
      timerAlarmEnable(timer_az);
    }
  } else
  { if (altdir > 0) digitalWrite( DIR_OUT_ALT, 0); else digitalWrite( DIR_OUT_ALT, 1 );
    altdir = sign(tspeed);
    if (base != period_alt)
    { timerAlarmWrite(timer_alt, period_alt = base, true);
      timerAlarmEnable(timer_alt);
    }

  }


}



  void setpositionf(motor_t* mt, double pos)
  {
  mt->position = pos;
  mt->counter = trunc(mt->position / mt->resolution);
  if (mt->id) azcounter = mt->counter ; else
    altcounter = mt->counter;
  
  }

void setposition(motor_t* mt, int pos)
{ if (mt->id) azcounter = pos ; else
    altcounter = pos;
  mt->counter = pos;
  mt->position =  mt->resolution * pos;
}
void go_to(motor_t* mt, double position, double speed)
{
  mt->slewing = true;
}

int readcounter(motor_t* mt)
{ int n;
  if (mt->id) n = azcounter; else
    n = altcounter;
  if (n != -1)
  {
    mt->position = mt->resolution * (mt->counter = n);
    mt->delta = mt->position - mt->target;
  }
  return 1;
}
int readcounter_n(motor_t* mt)
{
  int temp = altcounter;
   if (temp >= 0)
  {
    mt->counter = temp;
    if (temp > (mt->maxcounter / 2))
    {
      temp -= mt->maxcounter;
    }
    mt->position = mt->resolution * (temp);
    //    mt->delta= mt->position-mt->target;
  }
return 1;
}


void setmaxcounter(motor_t* M, int value)
{
   M->maxcounter = value;
}
void settarget(motor_t* mt, int pos)
{
 // set_motor_target(mt->id, pos);
}

void speed_up_down(motor_t* mt)
{
  if  (mt->speed != mt->targetspeed)
  {
    if  (fabs(mt->targetspeed - mt->speed) < fabs(mt->acceleration))
    {
      mt->speed = mt->targetspeed;
    }
    if (mt->speed < mt->targetspeed) mt->speed = mt->speed + mt->acceleration;
    else if (mt->speed > mt->targetspeed)   mt->speed = mt->speed - mt->acceleration;
    setspeed(mt, mt->speed);
  }
}

void  setcounter(motor_t* mt, int count)
{
  //set_motor_counter(mt->id, count);
}

void  loadconf(motor_t* mt, char* name) {}


void  savemotorcounter(motor_t* mt)
{
  //save_counters (mt->id);
}

void settargetspeed(motor_t* mt, double tspeed)

{
  if (fabs(tspeed) <= mt->maxspeed) mt->targetspeed = tspeed;
  else mt->targetspeed = mt->maxspeed * sign (tspeed);
}

void setbackslash(motor_t* mt,int back)
{
//setmotorbackslash(mt->id,abs(back));
//set_motor_back_slash_mode(mt->id,(back>0)? 1:0);
}
