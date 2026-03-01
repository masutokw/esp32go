#ifndef MOUNT_H_INCLUDED
#define MOUNT_H_INCLUDED
#include "conf.h"
#include <FS.h>
#include "motor.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "taki.h"
#define SID_RATE 15.04106711786691
#define SID_RATE_RAD SID_RATE *SEC_TO_RAD
#define SIDERALF = 15.041068558362671057482635080546
#define SOLAR_RATE 15.0
#define LUNAR_RATE 14.685
#define KING_RATE 15.0369
#define ARC_SEC_LMT 1.0
#define AZ_RED 8000 * 4 * 144
#define ALT_RED 8000 * 4 * 144
#define RATE_GUIDE 0.5
#define RATE_CENTER 8
#define RATE_FIND 50
#define RATE_SLEW 300
#define LOCAL_LONGITUDE -4.2
#define LOCAL_LATITUDE 36.72
#define ALT_ID 0X00
#define AZ_ID 0x01
#define TIME_ZONE 1
typedef enum { EQ,
               ALTAZ,
               ALIGN } mount_mode_t;
typedef struct
{
  mount_mode_t mount_mode;
  motor_t *altmotor, *azmotor;
  double dec_target, ra_target;  //radians
  double alt_target, az_target;
  double az_home,alt_home;
  double lat, longitude;
  double rate[4][2];
  double prescaler;
  double maxspeed[2];
  int srate;
  int track;
  int time_zone;
  char is_tracking;
  char sync;
  int smode;
  double track_speed;
  int autoflip;
  int hmf;
  double fix_ra_target;
  char parked;

} mount_t;

mount_t *create_mount(void);
int readconfig(mount_t *mt);
int destroy_mount(mount_t *m);
void mount_move(mount_t *mt, char direction);
int mount_stop(mount_t *mt, char direction);
void select_rate(mount_t *mt, char rate);
int sync_ra_dec(mount_t *mt);
void thread_motor(mount_t *m);
void thread_motor2(mount_t *m);
int get_pierside(mount_t *mt);
int get_pierside_target(mount_t *mt);
int goto_ra_dec(mount_t *mt, double ra, double dec);
void mount_lxde_str(char *message, mount_t *mt);
void mount_lxra_str(char *message, mount_t *mt);
void eq_to_enc(double *ra, double *dec, double a, double b, int pier);
void mount_park(mount_t *mt);
void mount_home_set(mount_t *mt);
void tak_init(mount_t *mt);
void track(mount_t *mt1);
void eq_track(mount_t *mt1);
void align_sync_all(mount_t *mt, long ra, long dec);
void mount_track_off(mount_t *mt);
int mount_slew(mount_t *mt);
int sync_eq(mount_t *mt);
void pulse_guide(mount_t *mt, char dir, int interval);
void meridianflip(mount_t *mt, int side);
void set_track_speed(mount_t *mt, int index);
void load_saved_pos(void);
void mount_goto_home(mount_t *mt);
void set_home(mount_t *mt);
void load_home(mount_t *mt);
void save_home(char fc, mount_t *mt);
void mount_fix_home(char fc, mount_t *mt);
char get_home_index(void);
#endif
