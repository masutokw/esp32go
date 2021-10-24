/*
 * Parses LX200 protocol.
 */

#define ADD_DIGIT(var,digit) var=var*10+digit-'0';
#define APPEND strcat(response,tmessage);
#define SYNC_MESSAGE "sync#"
//#define SYNC_MESSAGE "Coordinates     matched.        #"

#include <string.h>
#include <stdio.h>
#include "mount.h"
#include "misc.h"
#include <math.h>

char response [200];
char tmessage[50];
const int month_days[] = {31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
extern c_star st_now, st_target, st_current;
struct _telescope_
{   long dec_target,ra_target;
    long alt_target,az_target;
    long current_dec,current_ra;
    long lat,longitude,h_secs;
    int day,month,year,dayofyear;
    int hour,min,sec;

}
mount;
extern long sdt_millis;
extern mount_t *telescope;
void lxprintdate(void)
{
sprintf(tmessage,"%02d/%02d/%02d#",mount.month,mount.day,mount.year); APPEND;
  //sprintf(tmessage,"%02d/%02d/%02d#",9,19,21); APPEND;
}
void lxprinttime(void)
{
  // sprintf(tmessage,"%02d:%02d:%02d#",17,05,mount.sec);APPEND;
   sprintf(tmessage,"%02d:%02d:%02d#",mount.hour,mount.min,mount.sec);APPEND;
}
void lxprintsite(void)
{
    sprintf(tmessage,"Site Name#");APPEND;
};

void ltime(void)
{
    long pj =(long)1;
}

void set_cmd_exe(char cmd,long date)
{ int temp ;
    switch (cmd)
    {
    case 'r':
        mount.ra_target=date;
        break;
    case 'd':
        mount.dec_target=date ;
        break;
    case 'a':
        mount.alt_target=date;
        break;
    case 'z':
        mount.az_target=date ;
        break;
    case 't':
        mount.lat=date ;
        telescope->lat=date/3600.0;

        break;
    case 'g':
        mount.longitude=date ;
		if (date <648000)
        telescope->longitude=-date/3600.0; else
		telescope->longitude=(1296000.0-date)/3600.0;

        break;
    case 'L' :
        mount.h_secs=date;
        mount.hour=date/3600;
        temp = (date % 3600);
        mount.min = temp / 60;
        mount.sec = temp % 60;
        //setclock (mount.year,mount.month,mount.day,mount.hour,mount.min,mount.sec,telescope->time_zone)
        setclock (22,8,01,14,6,12,telescope->time_zone);
        break;
    case 'S':
        break;

    }
}
void set_date( int day,int month,int year)
{   mount.month=month-1;
    mount.day=day;
    mount.year=100+year;
     setclock (mount.year,mount.month,mount.day,mount.hour,mount.min,mount.sec,telescope->time_zone);
	//sprintf(tmessage,"%cUpdating Planetary Data# %d %d %d  #",'1', mount.month,mount.day,mount.year);APPEND;
	sprintf(tmessage,"%cUpdating Planetary Data#     #",'1');APPEND;
}
void set_time( int hour,int min,int sec)
{  //sprintf(tmessage,"year %d  month %d day %d Hour %d Min %d sec %d GMT %d" ,mount.year,mount.month, mount.day,hour,min,sec,telescope->time_zone);APPEND;
   //setclock (mount.year,mount.month, mount.day,hour,min,sec,telescope->time_zone);
    mount.min=min;
    mount.hour=hour;
    mount.sec=sec;
  // setclock (mount.year,mount.month,mount.day,hour,min,sec,telescope->time_zone);
   sprintf(tmessage,"1");APPEND;

}


//----------------------------------------------------------------------------------------
long command( char *str )
{
    char *p = str, *pe = str + strlen( str );
    int cs;
    char stcmd;
    long deg=0;
    int min=0;
    int sec=0;
    int neg = 1;
    tmessage[0]=0;
    response[0]=0;

    %%{
        machine command;
        write data;
    }%%




    %%{
#Acciones
        action getgrads {ADD_DIGIT(deg,fc); }
        action getmin {ADD_DIGIT(min,fc); }
        action getsec {ADD_DIGIT(sec,fc); }
        action neg { neg=-1;}
        action dir {mount_move(telescope,stcmd);}
        action Goto {goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD); sprintf(tmessage,"0");APPEND;}
        action stop {mount_stop(telescope,stcmd);}
        action rate {select_rate(telescope,stcmd); }
        action return_ra { lxprintra1(tmessage, st_current.ra); APPEND;}
        action return_dec {lxprintde1(tmessage, st_current.dec); APPEND;}
        action return_az { lxprintaz1(tmessage, st_current.az); APPEND;}
        action return_alt {lxprintde1(tmessage, st_current.alt); APPEND;}
        action return_ra_target { lxprintra1(tmessage, st_target.ra); APPEND;}
        action return_dec_target {lxprintde1(tmessage, st_target.dec); APPEND;}
        action return_date {lxprintdate1(tmessage);APPEND;}
        action return_site { lxprintsite();}
        action ok  {sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
        action return_longitude {lxprintlong1(tmessage,telescope->longitude);APPEND;}
        action return_lat {lxprintlat1(tmessage,telescope->lat);APPEND;}
        action return_sid_time { ;}
        action sync { align_sync_all(telescope,mount.ra_target,mount.dec_target); sprintf(tmessage,"sync#");APPEND;}
        action rafrac {deg+=(fc-'0')*6;}
        action return_local_time { lxprinttime1(tmessage);APPEND;}
        action set_cmd_exec {set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
        action addmin {deg=deg*3600+min*60;}
        action addsec {deg+=sec;}
        action storecmd {stcmd=fc;}
        action setdate {set_date(min,deg,sec);}
        action return_align{sprintf(tmessage,"A"); APPEND; }
		action set_gmt_offset{ telescope->time_zone=deg;}
		action return_GMT_offset {lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
        action settime{set_time(deg,min,sec);}
#definicion sintaxis LX terminos auxiliares
        sexmin =  ([0-5][0-9])$getmin@addmin ;
        sex= ([0-5][0-9] )$getsec@addsec;
		grad=([\+] |''|space | [\-]@neg) ((digit @getgrads){2,3});
        deg = grad punct sexmin (any  sex)? ;
        RA = ([0-2] digit) $getgrads   (':'|'/') sexmin ('.'digit@rafrac | (':'|'/') sex) ;

#Definicion sintaxis comandos
        Poll= 'G'( 'R'%return_ra |
                   'D'%return_dec|
                   'r'%return_ra_target |
                   'd'%return_dec_target |
                   'Z'%return_az |
                   'A'%return_alt |
                   'G'%return_GMT_offset |
                   'L'%return_local_time |
                   'S'%return_local_time|
                   'C'%return_date|
                   'M'%return_site|
                   'g'%return_longitude|
                   't'%return_lat);

        Move = 'M' ([nswe]@storecmd %dir | 'S'%Goto);
        Rate = 'R' [CGMS]@storecmd (''|[0-4]) %rate;
		Timezone='G'(''|space)([\+] | [\-]@neg)((digit @getgrads){1,2}) ('.' digit)? %set_gmt_offset%ok;
        date ='C' (''|space)  (digit@getgrads){2} '/' (digit @getmin){2} '/' (digit @getsec){2}%setdate;
        time ='L' (''|space) (digit @getgrads){2}':'(digit@getmin){2} ':'(digit @getsec){2}%settime;
        Set='S'((([dazgt]@storecmd (''|space) deg ) |([rS]@storecmd (''|space) RA))%set_cmd_exec| Timezone |date | time) ;

        Sync = "CM"(''|'R')%sync;
        Stop ='Q' (''|[nsew])@storecmd %stop;
       	ACK = 0x06 @return_align;
        main :=  (ACK|''|'#') (':' (Set | Move | Stop|Rate | Sync | Poll) '#')* ;

# Initialize and execute.
        write init;
        write exec;
    }%%

//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
};



