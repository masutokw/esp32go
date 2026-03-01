#include "Arduino.h"
#include "mount.h"
#include "misc.h"
#include <Ticker.h>
#include "tb6612.h"
#ifdef RTC_IC
#include "RTClib.h"
extern RTC_IC rtc;
#endif
extern long sdt_millis;
extern c_star st_now, st_target, st_current, st_1, st_2;
extern int focusvolt;
extern int dcfocus;
extern int azcounter, altcounter;
extern int azbackcounter, altbackcounter;
extern char tzstr[50];
double timetarget = 1.0;
Ticker pulse_dec_tckr, pulse_ra_tckr;
char sel_flag;
char volatile sync_target = TRUE;
char volatile sync_stop = FALSE;
char volatile Az_track = TRUE;
extern stepper focus_motor;
extern stepper aux_motor;
#ifdef RA_preTrack
bool pretrack = false;
double true_target = 0;
#endif
bool az_goto = false;
bool home_goto = false;
extern boolean ongoing_pulse_ra;
extern boolean ongoing_pulse_dec;
boolean ongoing_pulse_n = false;
boolean ongoing_pulse_s = false;
boolean ongoing_pulse_w = false;
boolean ongoing_pulse_e = false;
mount_t *create_mount(void) {
  int maxcounter = AZ_RED;
  int maxcounteralt = ALT_RED;
  mount_t *m;
  m = (mount_t *)malloc(sizeof(mount_t));
  //if (m) return NULL;
  m->azmotor = (motor_t *)malloc(sizeof(motor_t));
  m->altmotor = (motor_t *)malloc(sizeof(motor_t));
  m->track = 1;
  m->rate[3][0] = RATE_SLEW;
  m->rate[2][0] = RATE_FIND;
  m->rate[1][0] = RATE_CENTER;
  m->rate[0][0] = RATE_GUIDE;
  m->rate[3][1] = RATE_SLEW;
  m->rate[2][1] = RATE_FIND;
  m->rate[1][1] = RATE_CENTER;
  m->rate[0][1] = RATE_GUIDE;
  m->srate = 3;
  m->maxspeed[0] = (m->rate[3][0] * SID_RATE_RAD);
  m->maxspeed[1] = (m->rate[3][1] * SID_RATE_RAD);
  m->longitude = LOCAL_LONGITUDE;
  m->lat = LOCAL_LATITUDE;
  m->time_zone = TIME_ZONE;
  m->prescaler = 1.0;

  m->is_tracking = TRUE;
  // m->mount_mode = ALTAZ;
  m->mount_mode = EQ;
  m->sync = FALSE;
  m->smode = 0;
  m->track_speed = SID_RATE_RAD;
  load_saved_pos();
  m->autoflip = 0;
  m->fix_ra_target = 0;
  m->parked = 0;
  init_motor(m->azmotor, AZ_ID, AZ_RED, SID_RATE_RAD, m->prescaler, m->maxspeed[0], 412, 0, 0);
  init_motor(m->altmotor, ALT_ID, ALT_RED, 0, m->prescaler, m->maxspeed[1], 412, 0, 0);
  m->alt_home = 45.0;
  m->az_home = 90.0;
  return m;
}

int destroy_mount(mount_t *m) {
  free(m->azmotor);
  free(m->altmotor);
  free(m);
  return 1;
}

void thread_motor(mount_t *m) {
  static byte n;
  n++;
  n %= 30;
  if (!n) eq_track(m);
  speed_up_down(((mount_t *)m)->altmotor);
  speed_up_down(((mount_t *)m)->azmotor);
}
void thread_motor2(mount_t *m) {
  static byte n;
  n++;
  n %= 30;
  if (!n) track(m);
  speed_up_down(((mount_t *)m)->altmotor);
  speed_up_down(((mount_t *)m)->azmotor);
}
void eq_track(mount_t *mt1) {
  double s;
  double delta;
  double sgndelta;
  double speed;

  readcounter(mt1->altmotor);
  //goto -------------------------------------------------------------------------
  bool slew = mt1->altmotor->slewing || mt1->azmotor->slewing;
  if (mt1->altmotor->slewing) {
    sgndelta = (sign(delta = mt1->altmotor->delta));
    if (fabs(delta) > (M_PI)) sgndelta = -sgndelta;

    if (sgndelta && mt1->autoflip && (get_pierside(mt1) != get_pierside_target(mt1)))
      sgndelta = (get_pierside(mt1)) ? mt1->hmf : -mt1->hmf;




    if (fabs(delta / (SEC_TO_RAD)) >= 1.0) {
      speed = fmin(mt1->maxspeed[1], fabs(delta)) * sgndelta;
      mt1->altmotor->targetspeed = -speed;
    } else {
      mt1->altmotor->targetspeed = 0.0;
      mt1->altmotor->slewing = 0;
    }
  }



  readcounter(mt1->azmotor);
  if (mt1->parked)
    sgndelta = sign(delta = (mt1->azmotor->delta = mt1->azmotor->position - mt1->fix_ra_target));
  else
    sgndelta = sign(delta = (mt1->azmotor->delta = mt1->azmotor->position - calc_Ra(mt1->azmotor->target, mt1->longitude)));

  if (mt1->azmotor->slewing) {
    if (fabs(delta) > (M_PI)) sgndelta = -sgndelta;
    if (fabs(delta / (SEC_TO_RAD)) > ARC_SEC_LMT) {
      speed = fmin(mt1->maxspeed[0], fabs(delta)) * sgndelta;
      mt1->azmotor->targetspeed = -(speed) + (SID_RATE_RAD);
    } else {
#ifdef RA_preTrack
      if (pretrack && true_target != 0) {
        mt1->azmotor->target = true_target;
        pretrack = false;
        true_target = 0;
        return;
      } else {
        mt1->azmotor->targetspeed = mt1->track_speed;  // * mt1->track;
      }
#else
      mt1->azmotor->targetspeed = mt1->track_speed;  // * mt1->track;
#endif
      mt1->azmotor->slewing = 0;
    }
  }

  if (home_goto && mt1->parked && !mt1->altmotor->slewing && abs(mt1->azmotor->targetspeed) < abs(mt1->track_speed))  // goto HOME ends here
  {
    home_goto = false;
    mt1->azmotor->slewing = 0;
    mount_park(mt1);
  }
  if (slew && !(mt1->altmotor->slewing || mt1->azmotor->slewing))
    buzzerOn(300);
}

void set_home(mount_t *mt) {

  mt->az_home = mt->azmotor->position * RAD_TO_DEG;
  mt->alt_home = mt->altmotor->position * RAD_TO_DEG;
  save_home('9',mt);
}


int goto_ra_dec(mount_t *mt, double ra, double dec) {
  mt->parked = 0;
  mt->is_tracking = TRUE;
  st_target.ra = ra;
  st_target.dec = dec;
  mt->azmotor->slewing = mt->altmotor->slewing = true;
  az_goto = true;
  return 1;
}

int sync_ra_dec(mount_t *mt) {
  st_current.timer_count = ((millis() - sdt_millis) / 1000.0);  //chrono_read(&ti);
  st_current.dec = st_target.dec = mt->dec_target;
  st_current.ra = st_target.ra = mt->ra_target;
  to_alt_az(&st_current);
  setpositionf(mt->azmotor, st_current.az);
  if (st_current.alt >= 0.0)
    setpositionf(mt->altmotor, st_current.alt);
  else
    setpositionf(mt->altmotor, (M_2PI + st_current.alt));
  mt->sync = FALSE;
  // sync_target=true;
  return 1;
}

int sync_eq(mount_t *mt) {
  mt->altmotor->slewing = mt->azmotor->slewing = FALSE;

  eq_to_enc(&(mt->azmotor->target), &(mt->altmotor->target),
            mt->ra_target, mt->dec_target, get_pierside(mt));

  if (mt->autoflip) {
    bool side = (calc_lha(mt->ra_target, mt->longitude) > 180.0);
    eq_to_enc(&(mt->azmotor->target), &(mt->altmotor->target), mt->ra_target, mt->dec_target, side);
  }

  setpositionf(mt->altmotor, mt->altmotor->target);
  setpositionf(mt->azmotor, calc_Ra(mt->azmotor->target, mt->longitude));
  mt->altmotor->slewing = mt->azmotor->slewing = FALSE;
  return 1;
}

int mount_stop(mount_t *mt, char direction) {
  uint16_t n = 0;
  uint16_t top = 200;
  mt->altmotor->slewing = mt->azmotor->slewing = FALSE;
  if (mt->mount_mode != EQ) {
    if (mt->parked) mt->parked = 0;
    switch (direction) {
      case 'n':
      case 's':
        mt->altmotor->targetspeed = 0.;
        /*  do
          {
            yield();
            delay(5);
            n++;
          }
          while ((n < top) && (fabs(mt->altmotor->current_speed) > 0.00001));*/
        break;


      case 'w':
      case 'e':
        mt->azmotor->targetspeed = 0.00;
        /*  do
          {
            yield();
            delay(5);
            n++;
          }
          while ((n < top) && (fabs(mt->azmotor->current_speed) > 0.00001));*/


        break;

      default:
        //  mt->altmotor->targetspeed = 0.0;
        break;
    }

    // sync_target = TRUE;
    sync_stop = TRUE;
    //track(mt);
  } else  // mt->is_tracking = TRUE;
  {
    if (mt->parked) {
      set_track_speed(mt, 1);
      mt->parked = 0;
    }
    mt->altmotor->slewing = mt->azmotor->slewing = FALSE;
    switch (direction) {
      case 'n':
      case 's':
        if ((mt->srate) == 0) mt->altmotor->locked = 1;
        else mt->altmotor->locked = 0;
        mt->altmotor->targetspeed = 0.0;
        break;
      case 'w':
      case 'e':
        mt->azmotor->targetspeed = mt->track_speed;  //* mt->track;
        mt->is_tracking = TRUE;
        break;
      default:
        mt->altmotor->targetspeed = 0.0;
        mt->azmotor->targetspeed = mt->track_speed;  //* mt->track;
        break;
    }
  }
  return 1;
}
void mount_move(mount_t *mt, char dir) {
  mt->altmotor->slewing = mt->azmotor->slewing = FALSE;
  mt->is_tracking = FALSE;
  int srate = mt->srate;
  int invert = (get_pierside(mt)) ? -1 : 1;
  int sid = (srate == 0) ? 1 : -1;
  if (mt->mount_mode != EQ) sid = 0;
  switch (dir) {
    case 'n':
      mt->altmotor->targetspeed = SID_RATE_RAD * mt->rate[srate][1] * invert;
      break;
    case 's':
      mt->altmotor->targetspeed = -SID_RATE_RAD * mt->rate[srate][1] * invert;
      break;
    case 'w':
      mt->azmotor->targetspeed = SID_RATE_RAD * (mt->rate[srate][0] + sid);
      break;
    case 'e':
      mt->azmotor->targetspeed = -SID_RATE_RAD * (mt->rate[srate][0] - sid);
      break;
    case 'h':
      mount_track_off(mt);
      Az_track = FALSE;  //timetarget=0.0;
      break;
    case 't':
      Az_track = TRUE;
      timetarget = 1.0;
      break;
  };
}
void pulse_stop_dec(mount_t *mt) {
  mt->altmotor->slewing = FALSE;
  mt->altmotor->locked = 1;
  mt->altmotor->targetspeed = 0.0;
  setspeed(mt->altmotor,0.0);
  ongoing_pulse_dec = false;
  ongoing_pulse_n = ongoing_pulse_s = false;
  //  pulse_dec_tckr.detach();
}
void pulse_stop_ra(mount_t *mt) {
  mt->azmotor->slewing = FALSE;
  mt->azmotor->targetspeed = mt->track_speed;  //* mt->track;
  setspeed(mt->azmotor, mt->track_speed);
  ongoing_pulse_ra = false;
  ongoing_pulse_w = ongoing_pulse_e = false;
  // pulse_ra_tckr.detach();
}

void pulse_guide(mount_t *mt, char dir, int interval) {
  mt->altmotor->slewing = mt->azmotor->slewing = FALSE;
  mt->srate = 0;
  int srate = mt->srate;
  int invert = (get_pierside(mt)) ? -1 : 1;
  // int  sid = (srate == 0) ? 1 : -1;
  int sid = 1;
  switch (dir) {
    case 'n':
      if(ongoing_pulse_n)
        break;
      ongoing_pulse_n = true;
      ongoing_pulse_dec = true;
      mt->altmotor->targetspeed = SID_RATE_RAD * mt->rate[0][1] * invert;
      setspeed(mt->altmotor, mt->altmotor->targetspeed);
      pulse_dec_tckr.once_ms(interval, pulse_stop_dec, mt);
      break;
    case 's':
      if(ongoing_pulse_s)
        break;
      ongoing_pulse_s = true;
      ongoing_pulse_dec = true;
      mt->altmotor->targetspeed = -SID_RATE_RAD * mt->rate[0][1] * invert;
      setspeed(mt->altmotor, mt->altmotor->targetspeed);
      pulse_dec_tckr.once_ms(interval, pulse_stop_dec, mt);
      break;
    case 'w':
      if(ongoing_pulse_w)
        break;
      ongoing_pulse_w = true;
      ongoing_pulse_ra = true;
      mt->azmotor->targetspeed = SID_RATE_RAD * (mt->rate[0][0] + sid);
      setspeed(mt->azmotor, mt->azmotor->targetspeed);
      pulse_ra_tckr.once_ms(interval, pulse_stop_ra, mt);
      break;
    case 'e':
      if(ongoing_pulse_e)
        break;
      ongoing_pulse_e = true;
      ongoing_pulse_ra = true;
      mt->azmotor->targetspeed = -SID_RATE_RAD * (mt->rate[0][0] - sid);
      setspeed(mt->azmotor, mt->azmotor->targetspeed);
      pulse_ra_tckr.once_ms(interval, pulse_stop_ra, mt);
      break;
  };
}
void select_rate(mount_t *mt, char dir) {
  switch (dir) {
    case 'C':
      mt->srate = 1;
      break;
    case 'G':
      mt->srate = 0;
      break;
    case 'M':
      mt->srate = 2;
      break;
    case 'S':
      mt->srate = 3;
      break;
  };
}

int mount_slew(mount_t *mt) {
  if (mt->autoflip) {
    bool side = (calc_lha(mt->ra_target, mt->longitude) > 180.0);
    eq_to_enc(&(mt->azmotor->target), &(mt->altmotor->target), mt->ra_target, mt->dec_target, side);
  } else
    eq_to_enc(&(mt->azmotor->target), &(mt->altmotor->target), mt->ra_target, mt->dec_target, (get_pierside(mt)));
  mt->azmotor->slewing = mt->altmotor->slewing = true;
  mt->fix_ra_target = calc_Ra(mt->azmotor->target, mt->longitude);
  if (mt->parked) {
    set_track_speed(mt, 1);
    mt->parked = 0;
  }

#ifdef RA_preTrack
  if (mt->azmotor->position > calc_Ra(mt->azmotor->target, mt->longitude)) {
    pretrack = true;
    true_target = mt->azmotor->target;
    mt->azmotor->target = mt->azmotor->target + 0.025;
  }
#endif

  return 1;
}


int get_pierside(mount_t *mt) {
  return (((mt->altmotor->counter) > (mt->altmotor->maxcounter / 4)) && ((mt->altmotor->counter) < (3 * mt->altmotor->maxcounter / 4)));
}
int get_pierside_target(mount_t *mt) {
  return (((mt->altmotor->target) > (M_PI / 2.0)) && ((mt->altmotor->target) < (M_PI * 1.5)));
}

void mount_lxde_str(char *message, mount_t *mt) {
  double ang = mt->altmotor->position;
  if (ang > 1.5 * M_PI) ang = ang - (M_PI * 2.0);
  else if (ang > M_PI / 2.0) ang = M_PI - ang;

  int x = ang * RAD_TO_DEG * 3600.0;
  char c = '+';
  if (x < 0) {
    x = -x;
    c = '-';
  }
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  sprintf(message, "%c%02d%c%02d:%02d#", c, gra, 225, min, sec);
};
void mount_lxra_str(char *message, mount_t *mt) {

  double ang = calc_Ra(mt->azmotor->position, mt->longitude);
  if (get_pierside(mt)) {
    if (ang < M_PI) ang += M_PI;
    else ang -= M_PI;
  }
  int seconds = ang * RAD_TO_DEG * 3600.0;
  int x = trunc(seconds) / 15.0;
  int rest = ((seconds % 15) * 2) / 3;
  rest %= 15;
  //rest *= 10;
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  //  sprintf(message, "%02d:%02d:%02d#", gra, min, sec);
  sprintf(message, "%02d:%02d:%02d.%d#", gra, min, sec, rest);
};

int readconfig(mount_t *mt) {
  int maxcounter, maxcounteralt, back_az, back_alt;
  double tmp, tmp2;
  File f;

  if (!SPIFFS.exists(MOUNT_FILE)) return -1;

  f = SPIFFS.open(MOUNT_FILE, FILE_READ);
  String s = f.readStringUntil('\n');
  maxcounter = s.toInt();
  s = f.readStringUntil('\n');
  maxcounteralt = s.toInt();
  for (int j = 0; j < 2; j++)
    for (int n = 0; n < 4; n++) {
      s = f.readStringUntil('\n');
      mt->rate[n][j] = s.toFloat();
    };
  mt->srate = I_SELECT_RATE;
  mt->maxspeed[0] = (mt->rate[3][0] * SID_RATE_RAD);
  mt->maxspeed[1] = (mt->rate[3][1] * SID_RATE_RAD);
  s = f.readStringUntil('\n');
  mt->prescaler = s.toFloat();
  if ((mt->prescaler < 0.5) || (mt->prescaler > 2.0)) mt->prescaler = 1.0;
  s = f.readStringUntil('\n');
  mt->longitude = s.toFloat();
  s = f.readStringUntil('\n');
  mt->lat = s.toFloat();
  mt->hmf = (mt->lat < 0) ? -1 : 1;
  s = f.readStringUntil('\n');
  mt->time_zone = s.toInt();
  s = f.readStringUntil('\n');
  tmp = s.toFloat();
  s = f.readStringUntil('\n');
  tmp2 = s.toFloat();
  s = f.readStringUntil('\n');
  back_az = s.toInt();
  if (azbackcounter > back_az) azbackcounter = back_az;
  s = f.readStringUntil('\n');
  back_alt = s.toInt();
  if (altbackcounter > back_alt) altbackcounter = back_alt;
  s = f.readStringUntil('\n');
  mt->mount_mode = (mount_mode_t)s.toInt();
  s = f.readStringUntil('\n');
  mt->track = (s.toInt() > 0);
  set_track_speed(mt, s.toInt());
  s = f.readStringUntil('\n');
  mt->autoflip = s.toInt();
  s = f.readStringUntil('\n');
  int tmpaz = s.toInt();
  s = f.readStringUntil('\n');
  int tmpalt = s.toInt();
  init_motor(mt->azmotor, AZ_ID, maxcounter, 0, mt->prescaler, mt->maxspeed[0], tmp, back_az, tmpaz);
  init_motor(mt->altmotor, ALT_ID, maxcounteralt, 0, mt->prescaler, mt->maxspeed[1], tmp2, back_alt, tmpalt);
  s = f.readStringUntil('\n');
  mt->azmotor->active = s.toInt();
  s = f.readStringUntil('\n');
  mt->altmotor->active = s.toInt();
  f.close();
  return 0;
}
void mount_track_off(mount_t *mt)

{
  mt->altmotor->slewing = mt->azmotor->slewing = mt->is_tracking = FALSE;
  mt->altmotor->targetspeed = 0.0;
  mt->azmotor->targetspeed = 0.0;
  mount_park(mt);
  sync_stop = TRUE;
}
void mount_park(mount_t *mt)

{
  mt->altmotor->slewing = mt->azmotor->slewing = mt->is_tracking = FALSE;
  mt->altmotor->targetspeed = 0.0;
  mt->azmotor->targetspeed = 0.0;
  //  mt->parked=true;
  File f = SPIFFS.open("/savedpos", "w");
  f.println(azcounter);
  f.println(altcounter);
  f.println(focus_motor.position);
  f.println(aux_motor.position);
  f.close();
}
void mount_goto_home(mount_t *mt) {
  switch (mt->mount_mode) {
    case ALTAZ:
      mt->parked = 1;
      az_goto = true;
      //  set_star(&st_target, 90.0, 0.0, (mt->lat > 0) ? 0.0 : 180.0, abs(mt->lat), 0);
      set_star(&st_target, 90.0, 0.0, mt->az_home, abs(mt->alt_home), 0);
      break;
    case ALIGN:
      set_star(&st_target, 90.0, 0.0, 00.0, 90 * sign(mt->lat), 0);
      mt->parked = 1;
      break;
    case EQ:
#ifdef NCP_HOME
      mt->altmotor->target = ((mt->lat > 0) ? (M_PI / 2) : (M_PI * 1.5)) + 5e-6;
#else
                                                     //mt->altmotor->target = M_PI;
      mt->altmotor->target = mt->alt_home * DEG_TO_RAD;
#endif
      //  mt->fix_ra_target = mt->azmotor->target = M_PI / 2;
      mt->fix_ra_target = mt->azmotor->target = mt->az_home * DEG_TO_RAD;
      mt->azmotor->slewing = mt->altmotor->slewing = true;
      mt->track = 0;
      mt->is_tracking = 0;
      mt->parked = 1;
      home_goto = true;
      break;
  }
}

void mount_home_set(mount_t *mt)

{
  mt->altmotor->slewing = mt->azmotor->slewing = mt->is_tracking = FALSE;
  mt->altmotor->targetspeed = 0.0;
  mt->azmotor->targetspeed = 0.0;
  delay(100);
  switch (mt->mount_mode) {
    case ALTAZ:
      if (mt->lat > 0)
        setpositionf(mt->azmotor, mt->az_home * DEG_TO_RAD);
      else
        setpositionf(mt->azmotor, mt->az_home * DEG_TO_RAD);
      delay(10);
      setpositionf(mt->altmotor, mt->alt_home * DEG_TO_RAD);
      break;
    case ALIGN:
      setpositionf(mt->azmotor, (3.0 * M_PI / 2.0));
      delay(10);
      setpositionf(mt->altmotor, M_PI);
      break;
    case EQ:
#ifdef NCP_HOME
      setpositionf(mt->altmotor, ((mt->lat > 0) ? (M_PI / 2) : (M_PI * 1.5)) + 5e-6);


#else
      //setpositionf(mt->altmotor, M_PI);
      setpositionf(mt->altmotor, mt->alt_home * DEG_TO_RAD);

#endif
      // setpositionf(mt->azmotor, M_PI / 2);
      setpositionf(mt->azmotor, mt->az_home * DEG_TO_RAD);

      break;
  }
  //   save_counters(ALT_ID);
  //  delay(10);
  //  save_counters(AZ_ID);
  //  delay(10);
}
void meridianflip(mount_t *mt, int side) {
  int artemp, count, dectemp;
  if (side != get_pierside(mt)) {
    mt->altmotor->slewing = mt->azmotor->slewing = FALSE;
    if (mt->altmotor->counter <= (mt->altmotor->maxcounter / 2))
      dectemp = (mt->altmotor->maxcounter / 2) - mt->altmotor->counter;
    else
      dectemp = (mt->altmotor->maxcounter + mt->altmotor->maxcounter / 2) - mt->altmotor->counter;
    artemp = (mt->azmotor->counter + mt->azmotor->maxcounter / 2) % mt->azmotor->maxcounter;
    setposition(mt->altmotor, dectemp);
    setposition(mt->azmotor, artemp);
  }
}
void tak_init(mount_t *mt) {

  reset_transforms(0.0, 0.0, 0.0);
  double temp = sidereal_timeGMT(mt->longitude, mt->time_zone) * 15.0;
  sdt_millis = millis();
  if (mt->mount_mode == ALTAZ) {

    set_star(&st_1, temp + 90.0, 0.0, 90.0, 0.0, 0);
    //   init_star(1, &st_now);
    set_star(&st_2, temp, mt->lat, 180.00, 89.99, 0);
    //   init_star(2, &st_now);

  } else if ((mt->mount_mode == ALIGN) || (mt->mount_mode == EQ)) {
    double ra;
    set_star(&st_1, temp, 0.0, 180.0, 0.0, 0);
    //   init_star(1, &st_now);
    ra = st_1.ra + M_PI / 2.0;
    if (ra < 0) ra += M_2PI;
    if (mt->lat >= 0.0)
      set_star(&st_2, ra * RAD_TO_DEG, 45, 90, 45, 0);
    else
      set_star(&st_2, ra * RAD_TO_DEG, -45, 270, 45, 0);
    //  init_star(2, &st_now);
  }
  compute_trasform(&st_1, &st_2);
  set_star(&st_now, temp, mt->lat, 0.0, 0.0, 0);
  to_alt_az(&st_now);
  //  is_aligned=0;
  //  is_slewing='0';
  //  counter_x=counter_y=0;
}


void track(mount_t *mt) {
  double d_az_r, d_alt_r;
  readcounter_n(mt->altmotor);
  readcounter(mt->azmotor);
  st_current.timer_count = ((millis() - sdt_millis) / 1000.0);
  if ((!mt->parked) && (timetarget > 0)) st_target.timer_count = st_current.timer_count;
  st_current.az = mt->azmotor->position;
  st_current.alt = mt->altmotor->position;
  st_current.p_mode = st_target.p_mode = get_pierside(mt);
  //compute ecuatorial current equatorial values to be send out from LX200 protocol interface
  to_equatorial(&st_current);
  if ((sync_target) || (sync_stop && ((mt->azmotor->speed == 0.0) || mt->altmotor->speed == 0.0))) {
    st_target.ra = mt->ra_target = st_current.ra;
    st_target.dec = mt->dec_target = st_current.dec;
    sync_target = FALSE;
    sync_stop = FALSE;
    mt->is_tracking = Az_track;  //ultimo
  }

  if (mt->is_tracking) {
    //compute next alt/az mount values  for target next lap second
    if (!mt->parked) {
      //  st_target.timer_count += 1.0;
      st_target.timer_count += timetarget;
      to_alt_az(&st_target);
    }
    //compute delta values :next values from actual values for desired target coordinates
    d_az_r = (st_target.az) - st_current.az;
    if (fabs(d_az_r) > (M_PI)) d_az_r -= (M_2PI * sign(d_az_r));
    d_alt_r = (st_target.alt) - st_current.alt;
    ;
    if (fabs(d_alt_r) > (M_PI)) d_alt_r -= M_2PI;

    // Compute and set timer intervals for stepper  rates
    settargetspeed(mt->azmotor, d_az_r);
    settargetspeed(mt->altmotor, d_alt_r);

    if (mt->azmotor->slewing) mt->azmotor->slewing = abs(d_az_r) > 100 / RAD_TO_ARCS;
    if (mt->altmotor->slewing) mt->altmotor->slewing = abs(d_alt_r) > 100 / RAD_TO_ARCS;
  }
  if (mt->sync) sync_ra_dec(mt);
  if (az_goto && !mt->azmotor->slewing && !mt->altmotor->slewing) {
    az_goto = false;
    buzzerOn(300);
  }
}

void align_sync_all(mount_t *mt, long ra, long dec) {
  switch (mt->smode) {
    case 0:
      mt->altmotor->slewing = mt->azmotor->slewing = FALSE;
      mt->ra_target = ra * 15.0 * SEC_TO_RAD;
      mt->dec_target = dec * SEC_TO_RAD;
      mt->sync = TRUE;
      break;
    case 1:
      reset_transforms(0.0, 0.0, 0.0);
      set_star(&st_1, ra * (15.0 / 3600.0), dec / 3600.0, mt->azmotor->position * RAD_TO_DEG, RAD_TO_DEG * mt->altmotor->position, ((millis() - sdt_millis) / 1000.0));
      // init_star(1, &st_1);
      break;
    case 2:
      set_star(&st_2, ra * (15.0 / 3600.0), dec / 3600.0, RAD_TO_DEG * mt->azmotor->position, RAD_TO_DEG * mt->altmotor->position, ((millis() - sdt_millis) / 1000.0));
      // init_star(2, &st_2);
      mt->is_tracking = FALSE;
      sync_target = TRUE;
      compute_trasform(&st_1, &st_2);
      mt->is_tracking = TRUE;

      break;
    default:
      break;
  }
};
void set_track_speed(mount_t *mt, int index) {
  if (index < 5) mt->track = index;
  else mt->track = 1;
  switch (mt->track) {
    case 0:
      mt->track_speed = 0.0;
      break;
    case 1:
      mt->track_speed = SID_RATE_RAD;
      break;
    case 2:
      mt->track_speed = SOLAR_RATE * SEC_TO_RAD;
      break;
    case 3:
      mt->track_speed = LUNAR_RATE * SEC_TO_RAD;
      break;
    case 4:
      mt->track_speed = KING_RATE * SEC_TO_RAD;
      break;
    default:
      mt->track_speed = SID_RATE_RAD;
      break;
  }
  mt->azmotor->targetspeed = mt->track_speed;
}

void load_saved_pos(void) {
  int zcount[2] = { 0, 0 };
  File f;
  if (SPIFFS.exists("/savedpos")) {
    String s;
    f = SPIFFS.open("/savedpos", "r");
    s = f.readStringUntil('\n');
    azcounter = s.toInt();
    s = f.readStringUntil('\n');
    altcounter = s.toInt();
    s = f.readStringUntil('\n');
    focus_motor.position = focus_motor.target = s.toInt();
    s = f.readStringUntil('\n');
    aux_motor.position = aux_motor.target = s.toInt();
    f.close();
#if defined(RTC_IC) && defined(RTC_NVRAM) && RTC_NVRAM > 0
#if RTC_IC == RTC_DS3231

    int i = 0;
    uint8_t *bufy = (uint8_t *)&zcount;
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_NVADDR);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)RTC_ADDRESS, (uint8_t)7);
    while (Wire.available()) bufy[i++] = Wire.read();
    azcounter = zcount[0];
    altcounter = zcount[1];
#else
    rtc.readnvram((uint8_t *)&azcounter, 4, RTC_NVADDR);
    rtc.readnvram((uint8_t *)&altcounter, 4, RTC_NVADDR + 4);
#endif
#endif

  } else
    azcounter = altcounter = focus_motor.position = focus_motor.target = 0;
}

void load_home(mount_t *mt) {
  File f;
  if (SPIFFS.exists("/home.config")) {
    String s;
    f = SPIFFS.open("/home.config", "r");
    s = f.readStringUntil('\n');
    mt->az_home = s.toDouble();
    s = f.readStringUntil('\n');
    mt->alt_home = s.toDouble();
  }
}
void save_home(char fc, mount_t *mt) {
  File f = SPIFFS.open("/home.config", "w");
  f.println(mt->az_home);
  f.println(mt->alt_home);
  f.println(fc);
  f.close();
}

void mount_fix_home(char fc, mount_t *mt) {
  if (mt->mount_mode == EQ)
    switch (fc) {
      case '0':
        if (mt->lat >= 0.0) {
          //mt->az_home = 0.0;
          mt->az_home = 90.0; // M_PI / 2 * RAD_TO_DEG;
          mt->alt_home = 90.0003;
        } else {
          mt->az_home = 270;
          mt->alt_home = 269.99;
        }
        break;
      case '4': // polar - east
        if (mt->lat >= 0.0) {
          //mt->az_home = 0.0;
          mt->az_home = 90; // M_PI / 2 * RAD_TO_DEG;
          mt->alt_home = 89.99;
        } else { 
          mt->az_home = 270;
          mt->alt_home = 270.0;
        }
        break;
      case '1':
        mt->az_home = 180;
        mt->alt_home = 180 - mt->lat;
        break;
      case '2':
        if (mt->lat >= 0.0)
          mt->az_home = 90.0;
        else
          mt->az_home = 270;
        mt->alt_home = 180;
        break;
      case '3':
        if (mt->lat >= 0.0)
          mt->az_home = 90.0;
        else
          mt->az_home = 270;
        mt->alt_home = 0.0;
        break;
    }
  else
    switch (fc) {
      case '0':
      case '4':
        if (mt->lat >= 0.0) {
          mt->az_home = 0.0;
          mt->alt_home = abs(mt->lat);
        } else {
          mt->az_home = 180.0;
          mt->alt_home = abs(mt->lat);
        }
        break;
      case '1':
        mt->az_home = 180.0;
        mt->alt_home = 89.0;
        break;
      case '2':
        mt->az_home = 90.0;
        mt->alt_home = 0.0;
        break;
      case '3':
        mt->az_home = 270.0;
        mt->alt_home = 0.0;
        break;
    }

  save_home(fc, mt);
}

char get_home_index(void) {
  char fc;
  File f;

  if (!SPIFFS.exists("/home.config")) return -1;
  f = SPIFFS.open("/home.config", FILE_READ);
  String s = f.readStringUntil('\n');
  s = f.readStringUntil('\n');
  s = f.readStringUntil('\n');
  fc=s[0];
  f.close();
  return fc;
}