#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED
#include "conf.h"
#include <time.h>
#include <sys/time.h>
#include <machine/time.h>
#include <sys/reent.h>
#include <math.h>
#include <stdio.h>
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif
#define K_SID 1.00273790935
#define SEC_TO_RAD (M_PI/(3600.0*180.0))
#define JD2000          2451545.0
typedef struct timespec wallclock_t;
void sdt_init(double longitude,int tz);
double sidereal_timeGMT_alt(double longitudez);
double sidereal_timeGMT (double longitude,int t);
double calc_Ra(double lha,double longitude);
double calc_lha(double ra,double longitude);
void lxprintde1(char* message,double ang);
void lxprintde(char* message,double ang);
void lxprintra1(char *message,double ang);
void lxprintra(char *message,double ang);
void lxprintaz1(char *message,double ang);
void lxprintlong1(char *message,double ang);
void lxprintlat1(char *message,double ang);
void lxprinttime1(char *message);
void lxprintdate1(char *message);
void config_NTP(int zone,int dls );
void lxprintGMT_offset(char *message,double offset );
void setclock (int year,int month,int day,int hour,int min,int sec,int gmt);
void ln_get_equ_prec (double mean_ra,double mean_dec, double JD,double  *position_ra,double  *position_dec);
long getDecimal(float val);
void setwifipad(int ip3, int ip2);

#endif
