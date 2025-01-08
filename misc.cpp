#include "misc.h"
#include "time.h"
double sdt;
long sdt_millis;
extern WiFiClass Wifi;
extern int  wifi_pad_IP3;
extern int wifi_pad_IP2;
char tzstr[50] = TZ_SPAIN;
void sdt_init(double longitude, int tz)
{
  sdt_millis = millis();
  sdt = sidereal_timeGMT(longitude, tz);
}
//input deg ,output hour
double ln_range_degrees (double angle)
{
  double temp;

  if (angle >= 0.0 && angle < 360.0)
    return angle;

  temp = (int)(angle / 360);
  if (angle < 0.0)
    temp --;
  temp *= 360;
  return angle - temp;
}
double sidereal_timeGMT (double longitude, int tz)
{
  struct timeval now;
  double JD;
  double sidereal, tmp;
  double T;
  gettimeofday(&now, NULL);
  JD = ((now.tv_sec + now.tv_usec / 1000000.0)) / 86400.0 + 2440587.5;
  T = (JD - 2451545.0) / 36525.0;
  /* calc mean angle */
  sidereal = 280.46061837 + (360.98564736629 * (JD - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
  /* add a convenient multiple of 360 degrees */
  sidereal = fmod (sidereal + longitude, 360.0);
  /* change to hours */
  sidereal /= 15.0 ;
  return sidereal;
}
//input rad,deg output rad
double calc_Ra( double lha, double longitude)

{
  double temp;
  double sid = sidereal_timeGMT_alt(longitude) * 15.0;
  temp = (lha * 180.0) / M_PI;
  /*  if  (sid>=temp) return ((sid-lha) *M_PI)/180.0                             ;
    else return  ((sid-lha+360.0) *(M_PI))/180.0;*/
  if  (sid >= temp) return sid * (M_PI / 180.0) - lha;
  else return  sid * (M_PI / 180.0) - lha + 2.0 * M_PI;


}
double calc_lha(double ra, double longitude)
{
  double sid = sidereal_timeGMT(longitude, 1) * 15.0;
  double tmp = (sid - (ra * RAD_TO_DEG));
  if (tmp < 0.0 ) tmp += 360.0;
  return tmp ;
}

void lxprintde1(char* message, double ang)

{
  if (ang > 3 * (M_PI / 2)) ang = ang - (M_PI * 2.0);
  else if (ang > M_PI / 2 ) ang = M_PI - ang;
  if (ang < - (M_PI / 2)) ang = -ang - M_PI;
  int x = ang * RAD_TO_DEG * 3600.0;
  char c = '+';
  if (x < 0)
  {
    x = -x;
    c = '-';
  }
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  sprintf(message, "%c%02d%c%02d:%02d#", c, gra, 225, min, sec);

};
void lxprintde(char* message, double ang)

{
  if (ang > M_PI) ang = ang - (M_PI * 2.0);

  int x = ang * RAD_TO_DEG * 3600.0;
  char c = '+';
  if (x < 0)
  {
    x = -x;
    c = '-';
  }
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  sprintf(message, "%c%02d %02d:%02d", c, gra, min, sec);

};
void lxprintra1(char *message, double ang)
{ if (ang < 0) ang = abs(2 * M_PI - ang);
  int seconds = ang * RAD_TO_DEG * 3600.0;
  int x = trunc (seconds) / 15.0;
  int rest = ((seconds % 15) * 2) / 3;
  rest %= 15;
  //rest *= 10;
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  //sprintf(message, "%02d:%02d:%02d#", gra, min, sec);
  sprintf(message, "%02d:%02d:%02d.%d#", gra, min, sec, rest);
};
void lxprintra(char *message, double ang)
{ if (ang < 0) ang = abs(2 * M_PI - ang);
  int x = ang * RAD_TO_DEG * 3600.0 / 15.0;
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  sprintf(message, "%02d:%02d:%02d", gra, min, sec);

};

void lxprintaz1(char *message, double ang)
{


  int x = ang * RAD_TO_DEG * 3600.0 ;
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  sprintf(message, "%03d%c%02d:%02d#", gra, 225, min, sec);
}

void lxprintlat1(char *message, double ang)
{

  int x = ang  * 3600.0;
  char c = '+';
  if (x < 0)
  {
    x = -x;
    c = '-';
  }
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  sprintf(message, "%c%02d%c%02d#", c, gra, 225, min);
}

void lxprintlong1(char *message, double ang)
{

  int x = ang * 3600.0;
  char c = '-';
  if (x < 0)
  {
    x = -x;
    c = '+';
  }
  int gra = x / 3600;
  int temp = (x % 3600);
  int min = temp / 60;
  int sec = temp % 60;
  sprintf(message, "%c%03d%c%02d#", c, gra, 225, min);
}
void lxprinttime1(char *message)
{
  time_t now;
  struct tm *mtime;
  now = time(nullptr);
  mtime = localtime(&now);
  sprintf(message, "%02d:%02d:%02d#", mtime->tm_hour, mtime->tm_min, mtime->tm_sec);

}

void lxprintdate1(char *message)
{
  time_t now;
  struct tm *mtime;
  now = time(nullptr);
  mtime = localtime(&now);
  if ((mtime->tm_year) < 100) mtime->tm_year = 100;
  sprintf(message, "%02d/%02d/%02d#", mtime->tm_mon + 1, mtime->tm_mday, mtime->tm_year - 100);
}
void lxprintGMT_offset(char *message, double offset )
{
  int gmt = offset;
  char c = '+';
  if (offset > 0) c = '-';

  // sprintf(message, "%c%02d.0#", c, gmt);
  sprintf(message, "%c%02d#", c, abs(gmt));
}
void setclock (int year, int month, int day, int hour, int min, int sec, int gmt)
{
  time_t lxtime;
  struct tm  lxtimeinfo;
  struct timeval tv;

  lxtimeinfo.tm_year = year;
  lxtimeinfo.tm_mon = month;
  lxtimeinfo.tm_mday = day;
  lxtimeinfo.tm_hour = hour;
  lxtimeinfo.tm_min = min;
  lxtimeinfo.tm_sec = sec;
  lxtime = mktime (&lxtimeinfo );
  tv.tv_sec = lxtime;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);

}
/*
void config_NTP(int zone, int dls)
{
  char tx[10];
  //  configTime(0, 0,  "pool.ntp.org");
  if (zone) {
  //  sprintf(tx, "<%+03d>%d", -zone, zone);
   sprintf(tx, "<%+03d>%d", zone, -zone);
    configTzTime(tx, "pool.ntp.org");
    setenv("TZ", tx, 1);
  }
  else {
    setenv("TZ", tzstr , 1);
    configTzTime( tzstr, "pool.ntp.org");
  }

  tzset();

}*/


  void config_NTP(int zone, int dls)
  {
  configTime(zone * 3600, dls * 3600,"pool.ntp.org");
  #ifdef RETRY_NTP
  // check DNS first to avoid timeouts
  IPAddress ip;
  if (Wifi.hostByName("pool.ntp.org", ip) != 1 )
    return;
  struct tm time;
  int n = 0;
  if (n < 2 && !getLocalTime(&time))
  {
    n++;
    configTime(zone * 3600, dls * 3600,  "pool.ntp.org");
  }
  #endif
  }

void enc_to_eq(double x, double y, double *a, double  *b, char *pier)
{
  *a = x;
  *pier = true;
  if (y <= M_PI / 2.0) *b = y;
  else if (y < (M_PI * 3 / 2))
  {
    {
      *b = M_PI - y ;
      if (x < M_PI) *a += M_PI ;
      else *a -= M_PI;
      *pier = false;
    }

  }
  else  *b = y - M_PI * 2;

}


void eq_to_enc(double *ra, double *dec, double a, double  b, int pier)
{

  if (!pier)
  {
    *ra = a;
    if (b < 0.0)  *dec = b + 2 * M_PI;
    else *dec = b;
  }
  else
  {
    *dec = M_PI - b;
    if (a >= M_PI) *ra = a - M_PI;
    else *ra = M_PI + a;
  }
}
//output hours

double sidereal_timeGMT_alt(double longitude)
{
  double temp = (millis() - sdt_millis) / (1000.0 * 3600.0);
  temp = sdt + temp * K_SID;
  if (temp >= 24.0) return temp - 24.0;
  return temp;
}
//from LIBNOVA
void ln_get_equ_prec (double mean_ra, double mean_dec, double JD, double  *position_ra, double *position_dec)
{
  double t, t2, t3, A, B, C, zeta, eta, theta, ra, dec;

  /* change original ra and dec to radians */
  mean_ra = mean_ra / (RAD_TO_DEG);
  mean_dec = mean_dec / (RAD_TO_DEG);

  /* calc t, zeta, eta and theta for J2000.0 Equ 20.3 */
  t = (JD - JD2000) / 36525.0;
  t *= 1.0 / 3600.0;
  t2 = t * t;
  t3 = t2 * t;
  zeta = 2306.2181 * t + 0.30188 * t2 + 0.017998 * t3;
  eta = 2306.2181 * t + 1.09468 * t2 + 0.041833 * t3;
  theta = 2004.3109 * t - 0.42665 * t2 - 0.041833 * t3;
  zeta =  (zeta) / (RAD_TO_DEG);
  eta =  (eta) / (RAD_TO_DEG);;
  theta =  (theta) / (RAD_TO_DEG);;

  /* calc A,B,C equ 20.4 */
  A = cos (mean_dec) * sin (mean_ra + zeta);
  B = cos (theta) * cos (mean_dec) * cos (mean_ra + zeta) - sin (theta) * sin (mean_dec);
  C = sin (theta) * cos (mean_dec) * cos (mean_ra + zeta) + cos (theta) * sin (mean_dec);

  ra = atan2 (A, B) + eta;

  /* check for object near celestial pole */
  if (mean_dec > (0.4 * M_PI) || mean_dec < (-0.4 * M_PI))
  {
    /* close to pole */
    dec = acos (sqrt(A * A + B * B));
    if (mean_dec < 0.)
      dec *= -1; /* 0 <= acos() <= PI */
  }
  else
  {
    /* not close to pole */
    dec = asin (C);
  }

  /* change to degrees */
  *position_ra = ln_range_degrees ((RAD_TO_DEG) * (ra));
  *position_dec = (RAD_TO_DEG) * (dec);
}
long getDecimal(float val)
{
  int intPart = int(val);
  long decPart = 10000 * (val - intPart); //I am multiplying by 10000 assuming that the foat values will have a maximum of four decimal places
  //Change to match the number of decimal places you need
  if (decPart > 0)return (decPart);       //return the decimal part of float number if it is available
  else if (decPart < 0)return ((-1) * decPart); //if negative, multiply by -1
  else if (decPart = 0)return (0);       //return 0 if decimal part of float number is not available
  return 0;
}
void setwifipad(int ip3, int ip2)
{
  if ((ip3 > 0) && (ip3 < 255)) {
    wifi_pad_IP3 = ip3;
    wifi_pad_IP2 = ip2;

  }


}
int getoffset(void) {

  time_t  rawt;
  struct tm *tinfo;
  struct tm  linfo, ginfo;
  int y;


  rawt  = time(nullptr);
  tinfo = localtime( &rawt );

  gmtime_r( &rawt, &ginfo);
  localtime_r( &rawt, &linfo);
linfo.tm_isdst=0;

  time_t g = mktime(&ginfo);
  time_t l = mktime(&linfo);

  double offsetSeconds = l-g;// difftime( g, l );
  int    offsetHours   = (int)offsetSeconds / (60 * 60);


  return   offsetHours;//linfo.tm_isdst ? offsetHours + 1 : offsetHours;
}



/*

  void Eq_to_alaz(double ar, dec,*az, *alt)
  // localdatetime: Tdatetime);
  {


    double h, la, sin_alt, sina_az, sin_az, cos_az;



    h = degtorad(Local_Sideral_Time(localdatetime) * 15) - Ar;
    la = degtorad(Latitude);
    sin_alt = sin(dec) * sin(la) + cos(dec) * cos(la) * cos(h);
    alt = arcsin(sin_alt);
    sin_az = -sin(h) * cos(dec) / cos(alt);
    cos_az = (sin(dec) - sin(la) * sin_alt) / (cos(la) * cos(alt));
    az = arctan2(sin_az, cos_az);
  if (az < 0)  az = 2 * pi + az;
  }

  void altaztoEQ(void Eq_to_alaz(double *ar, *dec,az, alt)
  //  localdatetime: Tdatetime);
  {
    double h, la, d, tmp, tmp2, cosh, sinh;
    la = degtorad(Latitude);
    dec = arcsin((sin(alt) * sin(la) + cos(alt) * cos(la) * cos(az)));
    tmp = cos(la) * cos(dec);
    if (tmp != 0.0)
        cosh = (sin(alt) - (sin(la) * sin(dec))) / tmp;
    sinh = -sin(az) * cos(alt) / cos(dec);
    h = abs(arcsin(sinh));
    if (sinh > 0)
    {
        if (cosh > 0)
            Ar = h;
        else;
        Ar = pi - h;
    }
    else
    {
        if (cosh > 0)
            Ar = -h;
        else;
        Ar = pi + h;
    }

  }

*/
