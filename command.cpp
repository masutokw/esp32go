
#line 1 "command.rl"
/*
 * Parses LX200 protocol you must process this file with RAGEL compiler to get command.cpp file
 */

#define ADD_DIGIT(var,digit) var=var*10+digit-'0';
#define APPEND strcat(response,tmessage);
#define SYNC_MESSAGE "sync#"
//#define SYNC_MESSAGE "Coordinates     matched.        #"
#define UPDATEPLA "1Updating    Planetary Data. #                #"
#include <string.h>
#include <stdio.h>
#include "mount.h"
#include "misc.h"
#include <math.h>
#include "tb6612.h"
#include "focus.h"
#include "FS.h"
#ifdef NUNCHUCK_CONTROL
#include "nunchuck.h"
#endif
#include "tmc.h"
#include "wheel.h"
char response [300];
char tmessage[300];
extern c_star st_now, st_target, st_current;
extern char volatile sync_target;
extern stepper focus_motor,aux_motor,*pmotor;
stepper *lmotor=&focus_motor;
extern int  dcfocus;
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
void sendfile(const char *filename);
void conf(void);
void lxprintsite(void)
{
    sprintf(tmessage,"Site Name#");APPEND;
};

void appcmd(char cmd)
{switch(cmd)
{
	
  case 'z':sprintf(tmessage,"%d",telescope->azmotor->maxcounter);
  break;
  case 'a':sprintf(tmessage,"%d",telescope->altmotor->maxcounter);
  break;
  case 'g':sprintf(tmessage,"%f",telescope->rate[0][0]);
  break;
  case 'j':sprintf(tmessage,"%f",telescope->rate[0][1]);
  break;
  case 'A':conf();
  break;
  case 'T':sendfile(TMC_FILE);
  break;
  case 'F':sendfile(AUX_FILE);
  break;
  case 'D':sendfile(WHEEL_FILE);
  break;
  
}


}
void sendfile2(const char *filename){
File f = SPIFFS.open(filename,"r");
  if (!f) exit;
 String fsize=String(f.size()+1);
     f.read((uint8_t *)tmessage,f.size()+1);
	 APPEND("patata");
	 f.close();   
   }
   
void sendfile(const char *filename){
String s="";
File f = SPIFFS.open(filename,"r");
  if (!f) exit;
 while (f.available()){
  s+=char (f.read());
  }
s.toCharArray(tmessage,s.length()+1);
tmessage[ s.length() + 1]=0;
	 f.close();   
   }
   


void conf(void)
{
	
	sprintf(tmessage,"%d\r\n%d\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.4f\r\n%.6f\r\n%.6f\r\n%d\r\n%.0f\r\n%.0f\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n#\r\n",
          telescope->azmotor->maxcounter, telescope->altmotor->maxcounter,
          telescope->rate[0][0], telescope->rate[1][0], telescope->rate[2][0], telescope->rate[3][0],
          telescope->rate[0][1], telescope->rate[1][1], telescope->rate[2][1], telescope->rate[3][1],
		  telescope->prescaler,
          telescope->longitude, telescope->lat, telescope->time_zone,
		  telescope->azmotor->acceleration / SEC_TO_RAD, telescope->altmotor->acceleration / SEC_TO_RAD,
		 telescope->azmotor->backlash, telescope->altmotor->backlash,
		  telescope->mount_mode ,telescope->track, telescope->autoflip, telescope->azmotor->cw,
		  telescope->altmotor->cw,  telescope->azmotor->active, telescope->altmotor->active
		  );
		  readconfig(telescope);
			 
		
          
         	 
}
void conf_write(char *text,const char *filename)
{ File f = SPIFFS.open(filename,"w");
  if (!f) exit;
     f.print(text);
	 f.close();   
   
  }


void set_cmd_exe(char cmd,long date)
{ int temp ;
    switch (cmd)
    {
    case 'r':
       if (telescope->mount_mode) mount.ra_target=date;
	   else
	    telescope->azmotor->target=telescope->ra_target=date*SEC_TO_RAD*15.0;
        break;
    case 'd':
        if (telescope->mount_mode) mount.dec_target=date;
		else
		{  telescope->dec_target=date*SEC_TO_RAD;
        if  (telescope->dec_target<0.0)
            telescope->altmotor->target=2*M_PI+telescope->dec_target;
        else
            telescope->altmotor->target=telescope->dec_target;}
        break;
    case 'a':
        mount.alt_target=date;
		telescope->alt_target=date*SEC_TO_RAD;
        break;
    case 'z':
        mount.az_target=date ;
		telescope->az_target=date*SEC_TO_RAD;
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
        setclock (mount.year,mount.month,mount.day,mount.hour,mount.min,mount.sec,telescope->time_zone);
        //setclock (22,8,01,14,6,12,telescope->time_zone);
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
    if (telescope->mount_mode == EQ) {
    sdt_init(telescope->longitude, telescope->time_zone);
    }
  else
  { telescope->is_tracking = FALSE;
    sync_target = TRUE;
    tak_init(telescope);
    telescope->azmotor->targetspeed=0.0;
    telescope->altmotor->targetspeed=0.0;
  }
	//sprintf(tmessage,"%cUpdating Planetary Data#     #",'1');APPEND;
	sprintf(tmessage,"%cUpdating    Planetary Data. #                #",'1');APPEND;
}
void set_time( int hour,int min,int sec)
{
    mount.min=min;
    mount.hour=hour;
    mount.sec=sec;
    sprintf(tmessage,"%c",'1');APPEND;
}
void setnunchuk(char enable)
{
	#ifdef NUNCHUCK_CONTROL
	 if (enable=='1')
  {
    nunchuck_init(SDA_PIN, SCL_PIN);
    nunchuck_disable(FALSE);
    
  }
  else
    nunchuck_disable(TRUE);
#endif
}
void setflipmode(char enable)
{if (enable=='1') telescope->autoflip=1 ;
else 
	telescope->autoflip=0;
}

//----------------------------------------------------------------------------------------
long command( char *str )
{
    char *p = str, *pe = str + strlen( str );
    int cs;
    char stcmd,*mark;
	
    long deg=0;
    int min=0;
    int sec=0;
    int neg = 1;
	int ip3 =0;
	int ip2 =0;
    tmessage[0]=0;
    response[0]=0;
	int pulse=0;
	int focus_counter=0;
    
#line 2 "command.cpp"
static const int command_start = 327;
static const int command_first_final = 327;
static const int command_error = 0;

static const int command_en_main = 327;


#line 242 "command.rl"





    
#line 8 "command.cpp"
	{
	cs = command_start;
	}

#line 11 "command.cpp"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr20:
#line 328 "command.rl"
	{;}
	goto st327;
tr21:
#line 326 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	goto st327;
tr22:
#line 327 "command.rl"
	{telescope->track=1;}
	goto st327;
tr24:
#line 281 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	goto st327;
tr26:
#line 331 "command.rl"
	{if (telescope->azmotor->slewing || telescope->altmotor->slewing) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	goto st327;
tr37:
#line 308 "command.rl"
	{gotofocuser(lmotor->max_steps,lmotor->speed_low,lmotor);}
	goto st327;
tr39:
#line 306 "command.rl"
	{gotofocuser(lmotor->max_steps,lmotor->speed,lmotor);}
	goto st327;
tr40:
#line 307 "command.rl"
	{gotofocuser(0,lmotor->speed_low,lmotor);}
	goto st327;
tr42:
#line 305 "command.rl"
	{gotofocuser(0,lmotor->speed,lmotor);}
	goto st327;
tr50:
#line 310 "command.rl"
	{gotofocuser(focus_counter,lmotor);}
	goto st327;
tr51:
#line 317 "command.rl"
	{sprintf(tmessage,"%d#",lmotor->state<stop);APPEND;}
	goto st327;
tr63:
#line 314 "command.rl"
	{lmotor->position=lmotor->target=focus_counter;}
	goto st327;
tr71:
#line 309 "command.rl"
	{gotofocuser(lmotor->position+(focus_counter*neg));}
	goto st327;
tr72:
#line 311 "command.rl"
	{stopfocuser(lmotor);}
	goto st327;
tr73:
#line 315 "command.rl"
	{sprintf(tmessage,"%05d#",lmotor->position);APPEND;}
	goto st327;
tr94:
#line 271 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	goto st327;
tr95:
#line 274 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	goto st327;
tr96:
#line 268 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	goto st327;
tr97:
#line 301 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	goto st327;
tr98:
#line 333 "command.rl"
	{sprintf(tmessage, "%02d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2)+((telescope->azmotor->slewing || telescope->altmotor->slewing)<<3));APPEND;}
	goto st327;
tr99:
#line 288 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	goto st327;
tr100:
#line 275 "command.rl"
	{ lxprintsite();}
	goto st327;
tr101:
#line 266 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	goto st327;
tr102:
#line 289 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
	goto st327;
tr103:
#line 304 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	goto st327;
tr109:
#line 334 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
	goto st327;
tr110:
#line 338 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	goto st327;
tr111:
#line 335 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
	goto st327;
tr112:
#line 336 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	goto st327;
tr113:
#line 337 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
	goto st327;
tr114:
#line 270 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	goto st327;
tr115:
#line 303 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	goto st327;
tr116:
#line 273 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	goto st327;
tr117:
#line 277 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	goto st327;
tr118:
#line 332 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
	goto st327;
tr119:
#line 272 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	goto st327;
tr120:
#line 278 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	goto st327;
tr121:
#line 266 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 268 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 270 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 271 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 333 "command.rl"
	{sprintf(tmessage, "%02d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2)+((telescope->azmotor->slewing || telescope->altmotor->slewing)<<3));APPEND;}
#line 316 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st327;
tr127:
#line 339 "command.rl"
	{setwifipad(ip3,ip2);}
	goto st327;
tr134:
#line 259 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 buzzerOn(300);
					 sprintf(tmessage,"0");APPEND;}
	goto st327;
tr135:
#line 257 "command.rl"
	{mount_move(telescope,stcmd);}
	goto st327;
tr141:
#line 258 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	goto st327;
tr143:
#line 320 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->parked? "1" : "0"));APPEND;}
	goto st327;
tr144:
#line 264 "command.rl"
	{mount_stop(telescope,stcmd);}
	goto st327;
tr147:
#line 265 "command.rl"
	{select_rate(telescope,stcmd); }
	goto st327;
tr163:
#line 297 "command.rl"
	{set_date(min,deg,sec);}
	goto st327;
tr168:
#line 299 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 276 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	goto st327;
tr181:
#line 302 "command.rl"
	{set_time(deg,min,sec);}
	goto st327;
tr191:
#line 290 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st327;
tr212:
#line 340 "command.rl"
	{appcmd(stcmd);APPEND;}
	goto st327;
tr214:
#line 341 "command.rl"
	{mount_park(telescope);ESP.restart();}
	goto st327;
tr453:
#line 319 "command.rl"
	{buzzerOn(300);mount_goto_home(telescope);}
	goto st327;
tr454:
#line 329 "command.rl"
	{set_home(telescope);}
	goto st327;
tr461:
#line 324 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
	goto st327;
tr462:
#line 321 "command.rl"
	{mount_home_set(telescope);}
	goto st327;
tr463:
#line 323 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
	goto st327;
st327:
	if ( ++p == pe )
		goto _test_eof327;
case 327:
#line 209 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr468;
		case 35: goto st1;
		case 58: goto st2;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr468:
#line 298 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 225 "command.cpp"
	if ( (*p) == 58 )
		goto st2;
	goto st0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 65: goto st3;
		case 67: goto st7;
		case 68: goto st10;
		case 70: goto tr5;
		case 71: goto st45;
		case 73: goto st70;
		case 77: goto st79;
		case 80: goto st88;
		case 81: goto st90;
		case 82: goto st92;
		case 83: goto st95;
		case 88: goto tr5;
		case 97: goto st147;
		case 99: goto st148;
		case 104: goto st315;
		case 112: goto st319;
	}
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 65: goto st4;
		case 76: goto st5;
		case 80: goto st6;
	}
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 35 )
		goto tr20;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 35 )
		goto tr21;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 35 )
		goto tr22;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 77 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 35: goto tr24;
		case 82: goto st9;
	}
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 35 )
		goto tr24;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 35 )
		goto tr26;
	goto st0;
tr5:
#line 313 "command.rl"
	{if ((*p)=='F') lmotor=&focus_motor; else lmotor=&aux_motor;}
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 319 "command.cpp"
	switch( (*p) ) {
		case 43: goto st12;
		case 45: goto st14;
		case 65: goto st16;
		case 66: goto st23;
		case 73: goto st24;
		case 76: goto st26;
		case 80: goto st35;
		case 81: goto st42;
		case 112: goto st43;
		case 115: goto st44;
	}
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 35: goto tr37;
		case 43: goto st13;
	}
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 35 )
		goto tr39;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 35: goto tr40;
		case 45: goto st15;
	}
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 35 )
		goto tr42;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 43: goto st17;
		case 45: goto tr44;
	}
	goto st0;
tr44:
#line 256 "command.rl"
	{ neg=-1;}
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 380 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr45;
	goto st0;
tr45:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 390 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr46;
	goto st0;
tr46:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 400 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr47;
	goto st0;
tr47:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 410 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr48;
	goto st0;
tr48:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 420 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr49;
	goto st0;
tr49:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 430 "command.cpp"
	if ( (*p) == 35 )
		goto tr50;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 35 )
		goto tr51;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( 48 <= (*p) && (*p) <= 56 )
		goto tr52;
	goto st0;
tr52:
#line 318 "command.rl"
	{gotoindex((*p)-'0');}
	goto st25;
tr74:
#line 312 "command.rl"
	{if ((*p)=='0') pmotor=&focus_motor; else pmotor=&aux_motor; buzzerOn(300);}
	goto st25;
tr208:
#line 342 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st25;
tr452:
#line 322 "command.rl"
	{mount_fix_home((*p),telescope);}
	goto st25;
tr464:
#line 362 "command.rl"
	{setflipmode((*p));}
	goto st25;
tr466:
#line 361 "command.rl"
	{setnunchuk((*p));}
	goto st25;
tr467:
#line 325 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 480 "command.cpp"
	if ( (*p) == 35 )
		goto st327;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	if ( (*p) == 83 )
		goto st27;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( (*p) == 49 )
		goto st28;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 43: goto st29;
		case 45: goto tr57;
	}
	goto st0;
tr57:
#line 256 "command.rl"
	{ neg=-1;}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 513 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr58;
	goto st0;
tr58:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 523 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr59;
	goto st0;
tr59:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 533 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr60;
	goto st0;
tr60:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 543 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr61;
	goto st0;
tr61:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 553 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr62;
	goto st0;
tr62:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 563 "command.cpp"
	if ( (*p) == 35 )
		goto tr63;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	switch( (*p) ) {
		case 43: goto st36;
		case 45: goto tr65;
	}
	goto st0;
tr65:
#line 256 "command.rl"
	{ neg=-1;}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 582 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr66;
	goto st0;
tr66:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 592 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr67;
	goto st0;
tr67:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 602 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr68;
	goto st0;
tr68:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 612 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr69;
	goto st0;
tr69:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
#line 622 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr70;
	goto st0;
tr70:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 632 "command.cpp"
	if ( (*p) == 35 )
		goto tr71;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) == 35 )
		goto tr72;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 35 )
		goto tr73;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 44 )
		goto tr74;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr74;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	switch( (*p) ) {
		case 65: goto st46;
		case 67: goto st47;
		case 68: goto st48;
		case 71: goto st49;
		case 75: goto st50;
		case 76: goto st51;
		case 77: goto st52;
		case 82: goto st53;
		case 83: goto st54;
		case 84: goto st55;
		case 86: goto st56;
		case 90: goto st62;
		case 99: goto st63;
		case 100: goto st64;
		case 103: goto st65;
		case 107: goto st66;
		case 114: goto st67;
		case 116: goto st68;
		case 120: goto st69;
	}
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 35 )
		goto tr94;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 35 )
		goto tr95;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 35 )
		goto tr96;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 35 )
		goto tr97;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 35 )
		goto tr98;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 35 )
		goto tr99;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( (*p) == 35 )
		goto tr100;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) == 35 )
		goto tr101;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 35 )
		goto tr102;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 35 )
		goto tr103;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	switch( (*p) ) {
		case 68: goto st57;
		case 70: goto st58;
		case 78: goto st59;
		case 80: goto st60;
		case 84: goto st61;
	}
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 35 )
		goto tr109;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 35 )
		goto tr110;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 35 )
		goto tr111;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 35 )
		goto tr112;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 35 )
		goto tr113;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 35 )
		goto tr114;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 35 )
		goto tr115;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 35 )
		goto tr116;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 35 )
		goto tr117;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 35 )
		goto tr118;
	goto st0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( (*p) == 35 )
		goto tr119;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( (*p) == 35 )
		goto tr120;
	goto st0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( (*p) == 35 )
		goto tr121;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) == 80 )
		goto st71;
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr123;
	goto st0;
tr123:
#line 253 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 878 "command.cpp"
	if ( (*p) == 46 )
		goto st73;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr125;
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr126;
	goto st0;
tr126:
#line 254 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 897 "command.cpp"
	if ( (*p) == 35 )
		goto tr127;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr128;
	goto st0;
tr128:
#line 254 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st75;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
#line 909 "command.cpp"
	if ( (*p) == 35 )
		goto tr127;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr129;
	goto st0;
tr129:
#line 254 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st76;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
#line 921 "command.cpp"
	if ( (*p) == 35 )
		goto tr127;
	goto st0;
tr125:
#line 253 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
#line 931 "command.cpp"
	if ( (*p) == 46 )
		goto st73;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr130;
	goto st0;
tr130:
#line 253 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
#line 943 "command.cpp"
	if ( (*p) == 46 )
		goto st73;
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	switch( (*p) ) {
		case 83: goto st80;
		case 101: goto tr132;
		case 103: goto st82;
		case 104: goto tr132;
		case 110: goto tr132;
		case 119: goto tr132;
	}
	if ( 115 <= (*p) && (*p) <= 116 )
		goto tr132;
	goto st0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	if ( (*p) == 35 )
		goto tr134;
	goto st0;
tr132:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
#line 975 "command.cpp"
	if ( (*p) == 35 )
		goto tr135;
	goto st0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	switch( (*p) ) {
		case 101: goto tr136;
		case 110: goto tr136;
		case 115: goto tr136;
		case 119: goto tr136;
	}
	goto st0;
tr136:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st83;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
#line 996 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr137;
	goto st0;
tr137:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 1006 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr138;
	goto st0;
tr138:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st85;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
#line 1016 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr139;
	goto st0;
tr139:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
#line 1026 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr140;
	goto st0;
tr140:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st87;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
#line 1036 "command.cpp"
	if ( (*p) == 35 )
		goto tr141;
	goto st0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	if ( (*p) == 80 )
		goto st89;
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( (*p) == 35 )
		goto tr143;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	switch( (*p) ) {
		case 35: goto tr144;
		case 101: goto tr145;
		case 110: goto tr145;
		case 115: goto tr145;
		case 119: goto tr145;
	}
	goto st0;
tr145:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st91;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
#line 1072 "command.cpp"
	if ( (*p) == 35 )
		goto tr144;
	goto st0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	switch( (*p) ) {
		case 67: goto tr146;
		case 71: goto tr146;
		case 77: goto tr146;
		case 83: goto tr146;
	}
	goto st0;
tr146:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st93;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
#line 1093 "command.cpp"
	if ( (*p) == 35 )
		goto tr147;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st94;
	goto st0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( (*p) == 35 )
		goto tr147;
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	switch( (*p) ) {
		case 67: goto st96;
		case 71: goto st106;
		case 76: goto st113;
		case 83: goto tr152;
		case 97: goto tr153;
		case 100: goto tr153;
		case 103: goto tr153;
		case 114: goto tr152;
		case 116: goto tr153;
		case 122: goto tr153;
	}
	goto st0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( (*p) == 32 )
		goto st97;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr155;
	} else if ( (*p) >= 9 )
		goto st97;
	goto st0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr155;
	goto st0;
tr155:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st98;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
#line 1148 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr156;
	goto st0;
tr156:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st99;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
#line 1158 "command.cpp"
	if ( (*p) == 47 )
		goto st100;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr158;
	goto st0;
tr158:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st101;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
#line 1175 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr159;
	goto st0;
tr159:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
#line 1185 "command.cpp"
	if ( (*p) == 47 )
		goto st103;
	goto st0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr161;
	goto st0;
tr161:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st104;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
#line 1202 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr162;
	goto st0;
tr162:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
#line 1212 "command.cpp"
	if ( (*p) == 35 )
		goto tr163;
	goto st0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	switch( (*p) ) {
		case 32: goto st107;
		case 43: goto st108;
		case 45: goto tr166;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st107;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	switch( (*p) ) {
		case 43: goto st108;
		case 45: goto tr166;
	}
	goto st0;
tr166:
#line 256 "command.rl"
	{ neg=-1;}
	goto st108;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
#line 1243 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr167;
	goto st0;
tr167:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 1253 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr168;
		case 44: goto st110;
		case 46: goto st110;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr170;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st111;
	goto st0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	if ( (*p) == 35 )
		goto tr168;
	goto st0;
tr170:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1282 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr168;
		case 44: goto st110;
		case 46: goto st110;
	}
	goto st0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( (*p) == 32 )
		goto st114;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr173;
	} else if ( (*p) >= 9 )
		goto st114;
	goto st0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr173;
	goto st0;
tr173:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st115;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
#line 1314 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr174;
	goto st0;
tr174:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st116;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
#line 1324 "command.cpp"
	if ( (*p) == 58 )
		goto st117;
	goto st0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr176;
	goto st0;
tr176:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
#line 1341 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr177;
	goto st0;
tr177:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st119;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
#line 1351 "command.cpp"
	if ( (*p) == 58 )
		goto st120;
	goto st0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr179;
	goto st0;
tr179:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st121;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
#line 1368 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr180;
	goto st0;
tr180:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st122;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
#line 1378 "command.cpp"
	if ( (*p) == 35 )
		goto tr181;
	goto st0;
tr152:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st123;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
#line 1388 "command.cpp"
	if ( (*p) == 32 )
		goto st124;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr183;
	} else if ( (*p) >= 9 )
		goto st124;
	goto st0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr183;
	goto st0;
tr183:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st125;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
#line 1410 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr184;
	goto st0;
tr184:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st126;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
#line 1420 "command.cpp"
	switch( (*p) ) {
		case 47: goto st127;
		case 58: goto st127;
	}
	goto st0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr186;
	goto st0;
tr186:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st128;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
#line 1439 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr187;
	goto st0;
tr187:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 293 "command.rl"
	{deg=deg*3600+min*60;}
	goto st129;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
#line 1450 "command.cpp"
	switch( (*p) ) {
		case 46: goto st130;
		case 47: goto st132;
		case 58: goto st132;
	}
	goto st0;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr190;
	goto st0;
tr190:
#line 287 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st131;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
#line 1470 "command.cpp"
	if ( (*p) == 35 )
		goto tr191;
	goto st0;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr192;
	goto st0;
tr192:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st133;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
#line 1487 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr193;
	goto st0;
tr193:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 294 "command.rl"
	{deg+=sec;}
	goto st134;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
#line 1498 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr191;
		case 44: goto st135;
		case 46: goto st135;
		case 58: goto st131;
	}
	goto st0;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st136;
	goto st0;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
	switch( (*p) ) {
		case 35: goto tr191;
		case 58: goto st131;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st137;
	goto st0;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
	switch( (*p) ) {
		case 35: goto tr191;
		case 58: goto st131;
	}
	goto st0;
tr153:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st138;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
#line 1539 "command.cpp"
	switch( (*p) ) {
		case 32: goto st139;
		case 43: goto st140;
		case 45: goto tr200;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr201;
	} else if ( (*p) >= 9 )
		goto st139;
	goto st0;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	switch( (*p) ) {
		case 32: goto st140;
		case 43: goto st140;
		case 45: goto tr200;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr201;
	} else if ( (*p) >= 9 )
		goto st140;
	goto st0;
tr200:
#line 256 "command.rl"
	{ neg=-1;}
	goto st140;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
#line 1572 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr201;
	goto st0;
tr201:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st141;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
#line 1582 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr203;
		} else if ( (*p) >= 33 )
			goto st142;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st142;
		} else if ( (*p) >= 91 )
			goto st142;
	} else
		goto st142;
	goto st0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr204;
	goto st0;
tr204:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st143;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
#line 1611 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr205;
	goto st0;
tr205:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 293 "command.rl"
	{deg=deg*3600+min*60;}
	goto st144;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
#line 1622 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	goto st132;
tr206:
#line 290 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st328;
st328:
	if ( ++p == pe )
		goto _test_eof328;
case 328:
#line 1634 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr468;
		case 35: goto st1;
		case 58: goto st2;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr192;
	goto st0;
tr203:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st145;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
#line 1649 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr207;
		} else if ( (*p) >= 33 )
			goto st142;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st142;
		} else if ( (*p) >= 91 )
			goto st142;
	} else
		goto st142;
	goto st0;
tr207:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st146;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
#line 1671 "command.cpp"
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st142;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st142;
		} else if ( (*p) >= 91 )
			goto st142;
	} else
		goto st142;
	goto st0;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr208;
	goto st0;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
	switch( (*p) ) {
		case 65: goto tr209;
		case 68: goto tr209;
		case 70: goto tr209;
		case 82: goto st150;
		case 84: goto tr209;
		case 97: goto tr209;
		case 100: goto tr211;
		case 102: goto tr211;
		case 103: goto tr209;
		case 106: goto tr209;
		case 110: goto tr211;
		case 119: goto tr211;
		case 122: goto tr209;
	}
	if ( 115 <= (*p) && (*p) <= 116 )
		goto tr211;
	goto st0;
tr209:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st149;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
#line 1719 "command.cpp"
	if ( (*p) == 35 )
		goto tr212;
	goto st0;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	if ( (*p) == 82 )
		goto st151;
	goto st0;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	if ( (*p) == 35 )
		goto tr214;
	goto st0;
tr268:
#line 318 "command.rl"
	{gotoindex((*p)-'0');}
	goto st152;
tr289:
#line 312 "command.rl"
	{if ((*p)=='0') pmotor=&focus_motor; else pmotor=&aux_motor; buzzerOn(300);}
	goto st152;
tr424:
#line 342 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st152;
tr211:
#line 296 "command.rl"
	{stcmd=(*p);mark=p;mark++;}
	goto st152;
tr433:
#line 322 "command.rl"
	{mount_fix_home((*p),telescope);}
	goto st152;
tr445:
#line 362 "command.rl"
	{setflipmode((*p));}
	goto st152;
tr447:
#line 361 "command.rl"
	{setnunchuk((*p));}
	goto st152;
tr448:
#line 325 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st152;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
#line 1772 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	goto st152;
tr216:
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr236:
#line 328 "command.rl"
	{;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr237:
#line 326 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr238:
#line 327 "command.rl"
	{telescope->track=1;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr240:
#line 281 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr242:
#line 331 "command.rl"
	{if (telescope->azmotor->slewing || telescope->altmotor->slewing) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr253:
#line 308 "command.rl"
	{gotofocuser(lmotor->max_steps,lmotor->speed_low,lmotor);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr255:
#line 306 "command.rl"
	{gotofocuser(lmotor->max_steps,lmotor->speed,lmotor);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr256:
#line 307 "command.rl"
	{gotofocuser(0,lmotor->speed_low,lmotor);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr258:
#line 305 "command.rl"
	{gotofocuser(0,lmotor->speed,lmotor);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr266:
#line 310 "command.rl"
	{gotofocuser(focus_counter,lmotor);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr267:
#line 317 "command.rl"
	{sprintf(tmessage,"%d#",lmotor->state<stop);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr278:
#line 314 "command.rl"
	{lmotor->position=lmotor->target=focus_counter;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr286:
#line 309 "command.rl"
	{gotofocuser(lmotor->position+(focus_counter*neg));}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr287:
#line 311 "command.rl"
	{stopfocuser(lmotor);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr288:
#line 315 "command.rl"
	{sprintf(tmessage,"%05d#",lmotor->position);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr309:
#line 271 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr310:
#line 274 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr311:
#line 268 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr312:
#line 301 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr313:
#line 333 "command.rl"
	{sprintf(tmessage, "%02d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2)+((telescope->azmotor->slewing || telescope->altmotor->slewing)<<3));APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr314:
#line 288 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr315:
#line 275 "command.rl"
	{ lxprintsite();}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr316:
#line 266 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr317:
#line 289 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr318:
#line 304 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr324:
#line 334 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr325:
#line 338 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr326:
#line 335 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr327:
#line 336 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr328:
#line 337 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr329:
#line 270 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr330:
#line 303 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr331:
#line 273 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr332:
#line 277 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr333:
#line 332 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr334:
#line 272 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr335:
#line 278 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr336:
#line 266 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 268 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 270 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 271 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 333 "command.rl"
	{sprintf(tmessage, "%02d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2)+((telescope->azmotor->slewing || telescope->altmotor->slewing)<<3));APPEND;}
#line 316 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr342:
#line 339 "command.rl"
	{setwifipad(ip3,ip2);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr349:
#line 259 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 buzzerOn(300);
					 sprintf(tmessage,"0");APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr350:
#line 257 "command.rl"
	{mount_move(telescope,stcmd);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr356:
#line 258 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr358:
#line 320 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->parked? "1" : "0"));APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr359:
#line 264 "command.rl"
	{mount_stop(telescope,stcmd);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr362:
#line 265 "command.rl"
	{select_rate(telescope,stcmd); }
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr378:
#line 297 "command.rl"
	{set_date(min,deg,sec);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr383:
#line 299 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 276 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr396:
#line 302 "command.rl"
	{set_time(deg,min,sec);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr406:
#line 290 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr427:
#line 340 "command.rl"
	{appcmd(stcmd);APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr429:
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
#line 341 "command.rl"
	{mount_park(telescope);ESP.restart();}
	goto st329;
tr434:
#line 319 "command.rl"
	{buzzerOn(300);mount_goto_home(telescope);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr435:
#line 329 "command.rl"
	{set_home(telescope);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr442:
#line 324 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr443:
#line 321 "command.rl"
	{mount_home_set(telescope);}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
tr444:
#line 323 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st329;
st329:
	if ( ++p == pe )
		goto _test_eof329;
case 329:
#line 2540 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr470;
		case 35: goto tr216;
		case 58: goto st154;
	}
	goto st152;
tr470:
#line 298 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st153;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
#line 2553 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 58: goto st154;
	}
	goto st152;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	switch( (*p) ) {
		case 35: goto tr216;
		case 65: goto st155;
		case 67: goto st159;
		case 68: goto st162;
		case 70: goto tr221;
		case 71: goto st196;
		case 73: goto st221;
		case 77: goto st230;
		case 80: goto st239;
		case 81: goto st241;
		case 82: goto st243;
		case 83: goto st246;
		case 88: goto tr221;
		case 97: goto st298;
		case 99: goto st299;
		case 104: goto st303;
		case 112: goto st307;
	}
	goto st152;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	switch( (*p) ) {
		case 35: goto tr216;
		case 65: goto st156;
		case 76: goto st157;
		case 80: goto st158;
	}
	goto st152;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	if ( (*p) == 35 )
		goto tr236;
	goto st152;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	if ( (*p) == 35 )
		goto tr237;
	goto st152;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	if ( (*p) == 35 )
		goto tr238;
	goto st152;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	switch( (*p) ) {
		case 35: goto tr216;
		case 77: goto st160;
	}
	goto st152;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
	switch( (*p) ) {
		case 35: goto tr240;
		case 82: goto st161;
	}
	goto st152;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
	if ( (*p) == 35 )
		goto tr240;
	goto st152;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
	if ( (*p) == 35 )
		goto tr242;
	goto st152;
tr221:
#line 313 "command.rl"
	{if ((*p)=='F') lmotor=&focus_motor; else lmotor=&aux_motor;}
	goto st163;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
#line 2653 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 43: goto st164;
		case 45: goto st166;
		case 65: goto st168;
		case 66: goto st175;
		case 73: goto st176;
		case 76: goto st177;
		case 80: goto st186;
		case 81: goto st193;
		case 112: goto st194;
		case 115: goto st195;
	}
	goto st152;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
	switch( (*p) ) {
		case 35: goto tr253;
		case 43: goto st165;
	}
	goto st152;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
	if ( (*p) == 35 )
		goto tr255;
	goto st152;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
	switch( (*p) ) {
		case 35: goto tr256;
		case 45: goto st167;
	}
	goto st152;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
	if ( (*p) == 35 )
		goto tr258;
	goto st152;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
	switch( (*p) ) {
		case 35: goto tr216;
		case 43: goto st169;
		case 45: goto tr260;
	}
	goto st152;
tr260:
#line 256 "command.rl"
	{ neg=-1;}
	goto st169;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
#line 2716 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr261;
	goto st152;
tr261:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st170;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
#line 2728 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr262;
	goto st152;
tr262:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st171;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
#line 2740 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr263;
	goto st152;
tr263:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st172;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
#line 2752 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr264;
	goto st152;
tr264:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st173;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
#line 2764 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr265;
	goto st152;
tr265:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st174;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
#line 2776 "command.cpp"
	if ( (*p) == 35 )
		goto tr266;
	goto st152;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
	if ( (*p) == 35 )
		goto tr267;
	goto st152;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 56 )
		goto tr268;
	goto st152;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
	switch( (*p) ) {
		case 35: goto tr216;
		case 83: goto st178;
	}
	goto st152;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
	switch( (*p) ) {
		case 35: goto tr216;
		case 49: goto st179;
	}
	goto st152;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
	switch( (*p) ) {
		case 35: goto tr216;
		case 43: goto st180;
		case 45: goto tr272;
	}
	goto st152;
tr272:
#line 256 "command.rl"
	{ neg=-1;}
	goto st180;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
#line 2830 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr273;
	goto st152;
tr273:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st181;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
#line 2842 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr274;
	goto st152;
tr274:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st182;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
#line 2854 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr275;
	goto st152;
tr275:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st183;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
#line 2866 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr276;
	goto st152;
tr276:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st184;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
#line 2878 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr277;
	goto st152;
tr277:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st185;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
#line 2890 "command.cpp"
	if ( (*p) == 35 )
		goto tr278;
	goto st152;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
	switch( (*p) ) {
		case 35: goto tr216;
		case 43: goto st187;
		case 45: goto tr280;
	}
	goto st152;
tr280:
#line 256 "command.rl"
	{ neg=-1;}
	goto st187;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
#line 2910 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr281;
	goto st152;
tr281:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st188;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
#line 2922 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr282;
	goto st152;
tr282:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st189;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
#line 2934 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr283;
	goto st152;
tr283:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st190;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
#line 2946 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr284;
	goto st152;
tr284:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st191;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
#line 2958 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr285;
	goto st152;
tr285:
#line 255 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st192;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
#line 2970 "command.cpp"
	if ( (*p) == 35 )
		goto tr286;
	goto st152;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	if ( (*p) == 35 )
		goto tr287;
	goto st152;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	if ( (*p) == 35 )
		goto tr288;
	goto st152;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	switch( (*p) ) {
		case 35: goto tr216;
		case 44: goto tr289;
	}
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr289;
	goto st152;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
	switch( (*p) ) {
		case 35: goto tr216;
		case 65: goto st197;
		case 67: goto st198;
		case 68: goto st199;
		case 71: goto st200;
		case 75: goto st201;
		case 76: goto st202;
		case 77: goto st203;
		case 82: goto st204;
		case 83: goto st205;
		case 84: goto st206;
		case 86: goto st207;
		case 90: goto st213;
		case 99: goto st214;
		case 100: goto st215;
		case 103: goto st216;
		case 107: goto st217;
		case 114: goto st218;
		case 116: goto st219;
		case 120: goto st220;
	}
	goto st152;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
	if ( (*p) == 35 )
		goto tr309;
	goto st152;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	if ( (*p) == 35 )
		goto tr310;
	goto st152;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	if ( (*p) == 35 )
		goto tr311;
	goto st152;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	if ( (*p) == 35 )
		goto tr312;
	goto st152;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	if ( (*p) == 35 )
		goto tr313;
	goto st152;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	if ( (*p) == 35 )
		goto tr314;
	goto st152;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
	if ( (*p) == 35 )
		goto tr315;
	goto st152;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
	if ( (*p) == 35 )
		goto tr316;
	goto st152;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
	if ( (*p) == 35 )
		goto tr317;
	goto st152;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
	if ( (*p) == 35 )
		goto tr318;
	goto st152;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	switch( (*p) ) {
		case 35: goto tr216;
		case 68: goto st208;
		case 70: goto st209;
		case 78: goto st210;
		case 80: goto st211;
		case 84: goto st212;
	}
	goto st152;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	if ( (*p) == 35 )
		goto tr324;
	goto st152;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
	if ( (*p) == 35 )
		goto tr325;
	goto st152;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
	if ( (*p) == 35 )
		goto tr326;
	goto st152;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	if ( (*p) == 35 )
		goto tr327;
	goto st152;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	if ( (*p) == 35 )
		goto tr328;
	goto st152;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
	if ( (*p) == 35 )
		goto tr329;
	goto st152;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
	if ( (*p) == 35 )
		goto tr330;
	goto st152;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
	if ( (*p) == 35 )
		goto tr331;
	goto st152;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
	if ( (*p) == 35 )
		goto tr332;
	goto st152;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
	if ( (*p) == 35 )
		goto tr333;
	goto st152;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
	if ( (*p) == 35 )
		goto tr334;
	goto st152;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
	if ( (*p) == 35 )
		goto tr335;
	goto st152;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
	if ( (*p) == 35 )
		goto tr336;
	goto st152;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
	switch( (*p) ) {
		case 35: goto tr216;
		case 80: goto st222;
	}
	goto st152;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr338;
	goto st152;
tr338:
#line 253 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st223;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
#line 3224 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 46: goto st224;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr340;
	goto st152;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr341;
	goto st152;
tr341:
#line 254 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st225;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
#line 3247 "command.cpp"
	if ( (*p) == 35 )
		goto tr342;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr343;
	goto st152;
tr343:
#line 254 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st226;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
#line 3259 "command.cpp"
	if ( (*p) == 35 )
		goto tr342;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr344;
	goto st152;
tr344:
#line 254 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st227;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
#line 3271 "command.cpp"
	if ( (*p) == 35 )
		goto tr342;
	goto st152;
tr340:
#line 253 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st228;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
#line 3281 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 46: goto st224;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr345;
	goto st152;
tr345:
#line 253 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st229;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
#line 3295 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 46: goto st224;
	}
	goto st152;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
	switch( (*p) ) {
		case 35: goto tr216;
		case 83: goto st231;
		case 101: goto tr347;
		case 103: goto st233;
		case 104: goto tr347;
		case 110: goto tr347;
		case 119: goto tr347;
	}
	if ( 115 <= (*p) && (*p) <= 116 )
		goto tr347;
	goto st152;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
	if ( (*p) == 35 )
		goto tr349;
	goto st152;
tr347:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st232;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
#line 3330 "command.cpp"
	if ( (*p) == 35 )
		goto tr350;
	goto st152;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	switch( (*p) ) {
		case 35: goto tr216;
		case 101: goto tr351;
		case 110: goto tr351;
		case 115: goto tr351;
		case 119: goto tr351;
	}
	goto st152;
tr351:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st234;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
#line 3352 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr352;
	goto st152;
tr352:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st235;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
#line 3364 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr353;
	goto st152;
tr353:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st236;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
#line 3376 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr354;
	goto st152;
tr354:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st237;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
#line 3388 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr355;
	goto st152;
tr355:
#line 252 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st238;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
#line 3400 "command.cpp"
	if ( (*p) == 35 )
		goto tr356;
	goto st152;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
	switch( (*p) ) {
		case 35: goto tr216;
		case 80: goto st240;
	}
	goto st152;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
	if ( (*p) == 35 )
		goto tr358;
	goto st152;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
	switch( (*p) ) {
		case 35: goto tr359;
		case 101: goto tr360;
		case 110: goto tr360;
		case 115: goto tr360;
		case 119: goto tr360;
	}
	goto st152;
tr360:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st242;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
#line 3438 "command.cpp"
	if ( (*p) == 35 )
		goto tr359;
	goto st152;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
	switch( (*p) ) {
		case 35: goto tr216;
		case 67: goto tr361;
		case 71: goto tr361;
		case 77: goto tr361;
		case 83: goto tr361;
	}
	goto st152;
tr361:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st244;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
#line 3460 "command.cpp"
	if ( (*p) == 35 )
		goto tr362;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st245;
	goto st152;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
	if ( (*p) == 35 )
		goto tr362;
	goto st152;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
	switch( (*p) ) {
		case 35: goto tr216;
		case 67: goto st247;
		case 71: goto st257;
		case 76: goto st264;
		case 83: goto tr367;
		case 97: goto tr368;
		case 100: goto tr368;
		case 103: goto tr368;
		case 114: goto tr367;
		case 116: goto tr368;
		case 122: goto tr368;
	}
	goto st152;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
	switch( (*p) ) {
		case 32: goto st248;
		case 35: goto tr216;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr370;
	} else if ( (*p) >= 9 )
		goto st248;
	goto st152;
st248:
	if ( ++p == pe )
		goto _test_eof248;
case 248:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr370;
	goto st152;
tr370:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st249;
st249:
	if ( ++p == pe )
		goto _test_eof249;
case 249:
#line 3520 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr371;
	goto st152;
tr371:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st250;
st250:
	if ( ++p == pe )
		goto _test_eof250;
case 250:
#line 3532 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 47: goto st251;
	}
	goto st152;
st251:
	if ( ++p == pe )
		goto _test_eof251;
case 251:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr373;
	goto st152;
tr373:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st252;
st252:
	if ( ++p == pe )
		goto _test_eof252;
case 252:
#line 3553 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr374;
	goto st152;
tr374:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st253;
st253:
	if ( ++p == pe )
		goto _test_eof253;
case 253:
#line 3565 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 47: goto st254;
	}
	goto st152;
st254:
	if ( ++p == pe )
		goto _test_eof254;
case 254:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr376;
	goto st152;
tr376:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st255;
st255:
	if ( ++p == pe )
		goto _test_eof255;
case 255:
#line 3586 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr377;
	goto st152;
tr377:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st256;
st256:
	if ( ++p == pe )
		goto _test_eof256;
case 256:
#line 3598 "command.cpp"
	if ( (*p) == 35 )
		goto tr378;
	goto st152;
st257:
	if ( ++p == pe )
		goto _test_eof257;
case 257:
	switch( (*p) ) {
		case 32: goto st258;
		case 35: goto tr216;
		case 43: goto st259;
		case 45: goto tr381;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st258;
	goto st152;
st258:
	if ( ++p == pe )
		goto _test_eof258;
case 258:
	switch( (*p) ) {
		case 35: goto tr216;
		case 43: goto st259;
		case 45: goto tr381;
	}
	goto st152;
tr381:
#line 256 "command.rl"
	{ neg=-1;}
	goto st259;
st259:
	if ( ++p == pe )
		goto _test_eof259;
case 259:
#line 3631 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr382;
	goto st152;
tr382:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st260;
st260:
	if ( ++p == pe )
		goto _test_eof260;
case 260:
#line 3643 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr383;
		case 44: goto st261;
		case 46: goto st261;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr385;
	goto st152;
st261:
	if ( ++p == pe )
		goto _test_eof261;
case 261:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st262;
	goto st152;
st262:
	if ( ++p == pe )
		goto _test_eof262;
case 262:
	if ( (*p) == 35 )
		goto tr383;
	goto st152;
tr385:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st263;
st263:
	if ( ++p == pe )
		goto _test_eof263;
case 263:
#line 3674 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr383;
		case 44: goto st261;
		case 46: goto st261;
	}
	goto st152;
st264:
	if ( ++p == pe )
		goto _test_eof264;
case 264:
	switch( (*p) ) {
		case 32: goto st265;
		case 35: goto tr216;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr388;
	} else if ( (*p) >= 9 )
		goto st265;
	goto st152;
st265:
	if ( ++p == pe )
		goto _test_eof265;
case 265:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr388;
	goto st152;
tr388:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st266;
st266:
	if ( ++p == pe )
		goto _test_eof266;
case 266:
#line 3710 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr389;
	goto st152;
tr389:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st267;
st267:
	if ( ++p == pe )
		goto _test_eof267;
case 267:
#line 3722 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 58: goto st268;
	}
	goto st152;
st268:
	if ( ++p == pe )
		goto _test_eof268;
case 268:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr391;
	goto st152;
tr391:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st269;
st269:
	if ( ++p == pe )
		goto _test_eof269;
case 269:
#line 3743 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr392;
	goto st152;
tr392:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st270;
st270:
	if ( ++p == pe )
		goto _test_eof270;
case 270:
#line 3755 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 58: goto st271;
	}
	goto st152;
st271:
	if ( ++p == pe )
		goto _test_eof271;
case 271:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr394;
	goto st152;
tr394:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st272;
st272:
	if ( ++p == pe )
		goto _test_eof272;
case 272:
#line 3776 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr395;
	goto st152;
tr395:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st273;
st273:
	if ( ++p == pe )
		goto _test_eof273;
case 273:
#line 3788 "command.cpp"
	if ( (*p) == 35 )
		goto tr396;
	goto st152;
tr367:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st274;
st274:
	if ( ++p == pe )
		goto _test_eof274;
case 274:
#line 3798 "command.cpp"
	switch( (*p) ) {
		case 32: goto st275;
		case 35: goto tr216;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr398;
	} else if ( (*p) >= 9 )
		goto st275;
	goto st152;
st275:
	if ( ++p == pe )
		goto _test_eof275;
case 275:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr398;
	goto st152;
tr398:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st276;
st276:
	if ( ++p == pe )
		goto _test_eof276;
case 276:
#line 3824 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr399;
	goto st152;
tr399:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st277;
st277:
	if ( ++p == pe )
		goto _test_eof277;
case 277:
#line 3836 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 47: goto st278;
		case 58: goto st278;
	}
	goto st152;
st278:
	if ( ++p == pe )
		goto _test_eof278;
case 278:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr401;
	goto st152;
tr401:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st279;
st279:
	if ( ++p == pe )
		goto _test_eof279;
case 279:
#line 3858 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr402;
	goto st152;
tr402:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 293 "command.rl"
	{deg=deg*3600+min*60;}
	goto st280;
st280:
	if ( ++p == pe )
		goto _test_eof280;
case 280:
#line 3871 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr216;
		case 46: goto st281;
		case 47: goto st283;
		case 58: goto st283;
	}
	goto st152;
st281:
	if ( ++p == pe )
		goto _test_eof281;
case 281:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr405;
	goto st152;
tr405:
#line 287 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st282;
st282:
	if ( ++p == pe )
		goto _test_eof282;
case 282:
#line 3894 "command.cpp"
	if ( (*p) == 35 )
		goto tr406;
	goto st152;
st283:
	if ( ++p == pe )
		goto _test_eof283;
case 283:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr407;
	goto st152;
tr407:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st284;
st284:
	if ( ++p == pe )
		goto _test_eof284;
case 284:
#line 3913 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr408;
	goto st152;
tr408:
#line 251 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 294 "command.rl"
	{deg+=sec;}
	goto st285;
st285:
	if ( ++p == pe )
		goto _test_eof285;
case 285:
#line 3926 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr406;
		case 44: goto st286;
		case 46: goto st286;
		case 58: goto st282;
	}
	goto st152;
st286:
	if ( ++p == pe )
		goto _test_eof286;
case 286:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st287;
	goto st152;
st287:
	if ( ++p == pe )
		goto _test_eof287;
case 287:
	switch( (*p) ) {
		case 35: goto tr406;
		case 58: goto st282;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st288;
	goto st152;
st288:
	if ( ++p == pe )
		goto _test_eof288;
case 288:
	switch( (*p) ) {
		case 35: goto tr406;
		case 58: goto st282;
	}
	goto st152;
tr368:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st289;
st289:
	if ( ++p == pe )
		goto _test_eof289;
case 289:
#line 3969 "command.cpp"
	switch( (*p) ) {
		case 32: goto st290;
		case 35: goto tr216;
		case 43: goto st291;
		case 45: goto tr415;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr416;
	} else if ( (*p) >= 9 )
		goto st290;
	goto st152;
st290:
	if ( ++p == pe )
		goto _test_eof290;
case 290:
	switch( (*p) ) {
		case 32: goto st291;
		case 35: goto tr216;
		case 43: goto st291;
		case 45: goto tr415;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr416;
	} else if ( (*p) >= 9 )
		goto st291;
	goto st152;
tr415:
#line 256 "command.rl"
	{ neg=-1;}
	goto st291;
st291:
	if ( ++p == pe )
		goto _test_eof291;
case 291:
#line 4004 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr416;
	goto st152;
tr416:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st292;
st292:
	if ( ++p == pe )
		goto _test_eof292;
case 292:
#line 4016 "command.cpp"
	if ( (*p) == 35 )
		goto tr418;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr419;
		} else if ( (*p) >= 33 )
			goto st293;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st293;
		} else if ( (*p) >= 91 )
			goto st293;
	} else
		goto st293;
	goto st152;
st293:
	if ( ++p == pe )
		goto _test_eof293;
case 293:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr420;
	goto st152;
tr420:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st294;
st294:
	if ( ++p == pe )
		goto _test_eof294;
case 294:
#line 4049 "command.cpp"
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr421;
	goto st152;
tr421:
#line 250 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 293 "command.rl"
	{deg=deg*3600+min*60;}
	goto st295;
st295:
	if ( ++p == pe )
		goto _test_eof295;
case 295:
#line 4062 "command.cpp"
	if ( (*p) == 35 )
		goto tr422;
	goto st283;
tr422:
#line 290 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st330;
st330:
	if ( ++p == pe )
		goto _test_eof330;
case 330:
#line 4084 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr470;
		case 35: goto tr216;
		case 58: goto st154;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr407;
	goto st152;
tr418:
#line 351 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	goto st331;
st331:
	if ( ++p == pe )
		goto _test_eof331;
case 331:
#line 4108 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr470;
		case 35: goto tr216;
		case 58: goto st154;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr420;
	goto st152;
tr419:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st296;
st296:
	if ( ++p == pe )
		goto _test_eof296;
case 296:
#line 4123 "command.cpp"
	if ( (*p) == 35 )
		goto tr418;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr423;
		} else if ( (*p) >= 33 )
			goto st293;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st293;
		} else if ( (*p) >= 91 )
			goto st293;
	} else
		goto st293;
	goto st152;
tr423:
#line 249 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st297;
st297:
	if ( ++p == pe )
		goto _test_eof297;
case 297:
#line 4147 "command.cpp"
	if ( (*p) == 35 )
		goto tr418;
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st293;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st293;
		} else if ( (*p) >= 91 )
			goto st293;
	} else
		goto st293;
	goto st152;
st298:
	if ( ++p == pe )
		goto _test_eof298;
case 298:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr424;
	goto st152;
st299:
	if ( ++p == pe )
		goto _test_eof299;
case 299:
	switch( (*p) ) {
		case 35: goto tr216;
		case 65: goto tr425;
		case 68: goto tr425;
		case 70: goto tr425;
		case 82: goto st301;
		case 84: goto tr425;
		case 97: goto tr425;
		case 100: goto tr211;
		case 102: goto tr211;
		case 103: goto tr425;
		case 106: goto tr425;
		case 110: goto tr211;
		case 119: goto tr211;
		case 122: goto tr425;
	}
	if ( 115 <= (*p) && (*p) <= 116 )
		goto tr211;
	goto st152;
tr425:
#line 295 "command.rl"
	{stcmd=(*p);}
	goto st300;
st300:
	if ( ++p == pe )
		goto _test_eof300;
case 300:
#line 4200 "command.cpp"
	if ( (*p) == 35 )
		goto tr427;
	goto st152;
st301:
	if ( ++p == pe )
		goto _test_eof301;
case 301:
	switch( (*p) ) {
		case 35: goto tr216;
		case 82: goto st302;
	}
	goto st152;
st302:
	if ( ++p == pe )
		goto _test_eof302;
case 302:
	if ( (*p) == 35 )
		goto tr429;
	goto st152;
st303:
	if ( ++p == pe )
		goto _test_eof303;
case 303:
	switch( (*p) ) {
		case 35: goto tr216;
		case 72: goto st304;
		case 80: goto st305;
		case 83: goto st306;
	}
	goto st152;
st304:
	if ( ++p == pe )
		goto _test_eof304;
case 304:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr433;
	goto st152;
st305:
	if ( ++p == pe )
		goto _test_eof305;
case 305:
	if ( (*p) == 35 )
		goto tr434;
	goto st152;
st306:
	if ( ++p == pe )
		goto _test_eof306;
case 306:
	if ( (*p) == 35 )
		goto tr435;
	goto st152;
st307:
	if ( ++p == pe )
		goto _test_eof307;
case 307:
	switch( (*p) ) {
		case 35: goto tr216;
		case 70: goto st308;
		case 72: goto st309;
		case 83: goto st310;
		case 97: goto st311;
		case 110: goto st312;
		case 115: goto st314;
	}
	goto st152;
st308:
	if ( ++p == pe )
		goto _test_eof308;
case 308:
	if ( (*p) == 35 )
		goto tr442;
	goto st152;
st309:
	if ( ++p == pe )
		goto _test_eof309;
case 309:
	if ( (*p) == 35 )
		goto tr443;
	goto st152;
st310:
	if ( ++p == pe )
		goto _test_eof310;
case 310:
	if ( (*p) == 35 )
		goto tr444;
	goto st152;
st311:
	if ( ++p == pe )
		goto _test_eof311;
case 311:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr445;
	goto st152;
st312:
	if ( ++p == pe )
		goto _test_eof312;
case 312:
	switch( (*p) ) {
		case 35: goto tr216;
		case 107: goto st313;
	}
	goto st152;
st313:
	if ( ++p == pe )
		goto _test_eof313;
case 313:
	if ( (*p) == 35 )
		goto tr216;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr447;
	goto st152;
st314:
	if ( ++p == pe )
		goto _test_eof314;
case 314:
	switch( (*p) ) {
		case 35: goto tr216;
		case 101: goto tr448;
		case 119: goto tr448;
	}
	goto st152;
st315:
	if ( ++p == pe )
		goto _test_eof315;
case 315:
	switch( (*p) ) {
		case 72: goto st316;
		case 80: goto st317;
		case 83: goto st318;
	}
	goto st0;
st316:
	if ( ++p == pe )
		goto _test_eof316;
case 316:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr452;
	goto st0;
st317:
	if ( ++p == pe )
		goto _test_eof317;
case 317:
	if ( (*p) == 35 )
		goto tr453;
	goto st0;
st318:
	if ( ++p == pe )
		goto _test_eof318;
case 318:
	if ( (*p) == 35 )
		goto tr454;
	goto st0;
st319:
	if ( ++p == pe )
		goto _test_eof319;
case 319:
	switch( (*p) ) {
		case 70: goto st320;
		case 72: goto st321;
		case 83: goto st322;
		case 97: goto st323;
		case 110: goto st324;
		case 115: goto st326;
	}
	goto st0;
st320:
	if ( ++p == pe )
		goto _test_eof320;
case 320:
	if ( (*p) == 35 )
		goto tr461;
	goto st0;
st321:
	if ( ++p == pe )
		goto _test_eof321;
case 321:
	if ( (*p) == 35 )
		goto tr462;
	goto st0;
st322:
	if ( ++p == pe )
		goto _test_eof322;
case 322:
	if ( (*p) == 35 )
		goto tr463;
	goto st0;
st323:
	if ( ++p == pe )
		goto _test_eof323;
case 323:
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr464;
	goto st0;
st324:
	if ( ++p == pe )
		goto _test_eof324;
case 324:
	if ( (*p) == 107 )
		goto st325;
	goto st0;
st325:
	if ( ++p == pe )
		goto _test_eof325;
case 325:
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr466;
	goto st0;
st326:
	if ( ++p == pe )
		goto _test_eof326;
case 326:
	switch( (*p) ) {
		case 101: goto tr467;
		case 119: goto tr467;
	}
	goto st0;
	}
	_test_eof327: cs = 327; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof130: cs = 130; goto _test_eof; 
	_test_eof131: cs = 131; goto _test_eof; 
	_test_eof132: cs = 132; goto _test_eof; 
	_test_eof133: cs = 133; goto _test_eof; 
	_test_eof134: cs = 134; goto _test_eof; 
	_test_eof135: cs = 135; goto _test_eof; 
	_test_eof136: cs = 136; goto _test_eof; 
	_test_eof137: cs = 137; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof328: cs = 328; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
	_test_eof149: cs = 149; goto _test_eof; 
	_test_eof150: cs = 150; goto _test_eof; 
	_test_eof151: cs = 151; goto _test_eof; 
	_test_eof152: cs = 152; goto _test_eof; 
	_test_eof329: cs = 329; goto _test_eof; 
	_test_eof153: cs = 153; goto _test_eof; 
	_test_eof154: cs = 154; goto _test_eof; 
	_test_eof155: cs = 155; goto _test_eof; 
	_test_eof156: cs = 156; goto _test_eof; 
	_test_eof157: cs = 157; goto _test_eof; 
	_test_eof158: cs = 158; goto _test_eof; 
	_test_eof159: cs = 159; goto _test_eof; 
	_test_eof160: cs = 160; goto _test_eof; 
	_test_eof161: cs = 161; goto _test_eof; 
	_test_eof162: cs = 162; goto _test_eof; 
	_test_eof163: cs = 163; goto _test_eof; 
	_test_eof164: cs = 164; goto _test_eof; 
	_test_eof165: cs = 165; goto _test_eof; 
	_test_eof166: cs = 166; goto _test_eof; 
	_test_eof167: cs = 167; goto _test_eof; 
	_test_eof168: cs = 168; goto _test_eof; 
	_test_eof169: cs = 169; goto _test_eof; 
	_test_eof170: cs = 170; goto _test_eof; 
	_test_eof171: cs = 171; goto _test_eof; 
	_test_eof172: cs = 172; goto _test_eof; 
	_test_eof173: cs = 173; goto _test_eof; 
	_test_eof174: cs = 174; goto _test_eof; 
	_test_eof175: cs = 175; goto _test_eof; 
	_test_eof176: cs = 176; goto _test_eof; 
	_test_eof177: cs = 177; goto _test_eof; 
	_test_eof178: cs = 178; goto _test_eof; 
	_test_eof179: cs = 179; goto _test_eof; 
	_test_eof180: cs = 180; goto _test_eof; 
	_test_eof181: cs = 181; goto _test_eof; 
	_test_eof182: cs = 182; goto _test_eof; 
	_test_eof183: cs = 183; goto _test_eof; 
	_test_eof184: cs = 184; goto _test_eof; 
	_test_eof185: cs = 185; goto _test_eof; 
	_test_eof186: cs = 186; goto _test_eof; 
	_test_eof187: cs = 187; goto _test_eof; 
	_test_eof188: cs = 188; goto _test_eof; 
	_test_eof189: cs = 189; goto _test_eof; 
	_test_eof190: cs = 190; goto _test_eof; 
	_test_eof191: cs = 191; goto _test_eof; 
	_test_eof192: cs = 192; goto _test_eof; 
	_test_eof193: cs = 193; goto _test_eof; 
	_test_eof194: cs = 194; goto _test_eof; 
	_test_eof195: cs = 195; goto _test_eof; 
	_test_eof196: cs = 196; goto _test_eof; 
	_test_eof197: cs = 197; goto _test_eof; 
	_test_eof198: cs = 198; goto _test_eof; 
	_test_eof199: cs = 199; goto _test_eof; 
	_test_eof200: cs = 200; goto _test_eof; 
	_test_eof201: cs = 201; goto _test_eof; 
	_test_eof202: cs = 202; goto _test_eof; 
	_test_eof203: cs = 203; goto _test_eof; 
	_test_eof204: cs = 204; goto _test_eof; 
	_test_eof205: cs = 205; goto _test_eof; 
	_test_eof206: cs = 206; goto _test_eof; 
	_test_eof207: cs = 207; goto _test_eof; 
	_test_eof208: cs = 208; goto _test_eof; 
	_test_eof209: cs = 209; goto _test_eof; 
	_test_eof210: cs = 210; goto _test_eof; 
	_test_eof211: cs = 211; goto _test_eof; 
	_test_eof212: cs = 212; goto _test_eof; 
	_test_eof213: cs = 213; goto _test_eof; 
	_test_eof214: cs = 214; goto _test_eof; 
	_test_eof215: cs = 215; goto _test_eof; 
	_test_eof216: cs = 216; goto _test_eof; 
	_test_eof217: cs = 217; goto _test_eof; 
	_test_eof218: cs = 218; goto _test_eof; 
	_test_eof219: cs = 219; goto _test_eof; 
	_test_eof220: cs = 220; goto _test_eof; 
	_test_eof221: cs = 221; goto _test_eof; 
	_test_eof222: cs = 222; goto _test_eof; 
	_test_eof223: cs = 223; goto _test_eof; 
	_test_eof224: cs = 224; goto _test_eof; 
	_test_eof225: cs = 225; goto _test_eof; 
	_test_eof226: cs = 226; goto _test_eof; 
	_test_eof227: cs = 227; goto _test_eof; 
	_test_eof228: cs = 228; goto _test_eof; 
	_test_eof229: cs = 229; goto _test_eof; 
	_test_eof230: cs = 230; goto _test_eof; 
	_test_eof231: cs = 231; goto _test_eof; 
	_test_eof232: cs = 232; goto _test_eof; 
	_test_eof233: cs = 233; goto _test_eof; 
	_test_eof234: cs = 234; goto _test_eof; 
	_test_eof235: cs = 235; goto _test_eof; 
	_test_eof236: cs = 236; goto _test_eof; 
	_test_eof237: cs = 237; goto _test_eof; 
	_test_eof238: cs = 238; goto _test_eof; 
	_test_eof239: cs = 239; goto _test_eof; 
	_test_eof240: cs = 240; goto _test_eof; 
	_test_eof241: cs = 241; goto _test_eof; 
	_test_eof242: cs = 242; goto _test_eof; 
	_test_eof243: cs = 243; goto _test_eof; 
	_test_eof244: cs = 244; goto _test_eof; 
	_test_eof245: cs = 245; goto _test_eof; 
	_test_eof246: cs = 246; goto _test_eof; 
	_test_eof247: cs = 247; goto _test_eof; 
	_test_eof248: cs = 248; goto _test_eof; 
	_test_eof249: cs = 249; goto _test_eof; 
	_test_eof250: cs = 250; goto _test_eof; 
	_test_eof251: cs = 251; goto _test_eof; 
	_test_eof252: cs = 252; goto _test_eof; 
	_test_eof253: cs = 253; goto _test_eof; 
	_test_eof254: cs = 254; goto _test_eof; 
	_test_eof255: cs = 255; goto _test_eof; 
	_test_eof256: cs = 256; goto _test_eof; 
	_test_eof257: cs = 257; goto _test_eof; 
	_test_eof258: cs = 258; goto _test_eof; 
	_test_eof259: cs = 259; goto _test_eof; 
	_test_eof260: cs = 260; goto _test_eof; 
	_test_eof261: cs = 261; goto _test_eof; 
	_test_eof262: cs = 262; goto _test_eof; 
	_test_eof263: cs = 263; goto _test_eof; 
	_test_eof264: cs = 264; goto _test_eof; 
	_test_eof265: cs = 265; goto _test_eof; 
	_test_eof266: cs = 266; goto _test_eof; 
	_test_eof267: cs = 267; goto _test_eof; 
	_test_eof268: cs = 268; goto _test_eof; 
	_test_eof269: cs = 269; goto _test_eof; 
	_test_eof270: cs = 270; goto _test_eof; 
	_test_eof271: cs = 271; goto _test_eof; 
	_test_eof272: cs = 272; goto _test_eof; 
	_test_eof273: cs = 273; goto _test_eof; 
	_test_eof274: cs = 274; goto _test_eof; 
	_test_eof275: cs = 275; goto _test_eof; 
	_test_eof276: cs = 276; goto _test_eof; 
	_test_eof277: cs = 277; goto _test_eof; 
	_test_eof278: cs = 278; goto _test_eof; 
	_test_eof279: cs = 279; goto _test_eof; 
	_test_eof280: cs = 280; goto _test_eof; 
	_test_eof281: cs = 281; goto _test_eof; 
	_test_eof282: cs = 282; goto _test_eof; 
	_test_eof283: cs = 283; goto _test_eof; 
	_test_eof284: cs = 284; goto _test_eof; 
	_test_eof285: cs = 285; goto _test_eof; 
	_test_eof286: cs = 286; goto _test_eof; 
	_test_eof287: cs = 287; goto _test_eof; 
	_test_eof288: cs = 288; goto _test_eof; 
	_test_eof289: cs = 289; goto _test_eof; 
	_test_eof290: cs = 290; goto _test_eof; 
	_test_eof291: cs = 291; goto _test_eof; 
	_test_eof292: cs = 292; goto _test_eof; 
	_test_eof293: cs = 293; goto _test_eof; 
	_test_eof294: cs = 294; goto _test_eof; 
	_test_eof295: cs = 295; goto _test_eof; 
	_test_eof330: cs = 330; goto _test_eof; 
	_test_eof331: cs = 331; goto _test_eof; 
	_test_eof296: cs = 296; goto _test_eof; 
	_test_eof297: cs = 297; goto _test_eof; 
	_test_eof298: cs = 298; goto _test_eof; 
	_test_eof299: cs = 299; goto _test_eof; 
	_test_eof300: cs = 300; goto _test_eof; 
	_test_eof301: cs = 301; goto _test_eof; 
	_test_eof302: cs = 302; goto _test_eof; 
	_test_eof303: cs = 303; goto _test_eof; 
	_test_eof304: cs = 304; goto _test_eof; 
	_test_eof305: cs = 305; goto _test_eof; 
	_test_eof306: cs = 306; goto _test_eof; 
	_test_eof307: cs = 307; goto _test_eof; 
	_test_eof308: cs = 308; goto _test_eof; 
	_test_eof309: cs = 309; goto _test_eof; 
	_test_eof310: cs = 310; goto _test_eof; 
	_test_eof311: cs = 311; goto _test_eof; 
	_test_eof312: cs = 312; goto _test_eof; 
	_test_eof313: cs = 313; goto _test_eof; 
	_test_eof314: cs = 314; goto _test_eof; 
	_test_eof315: cs = 315; goto _test_eof; 
	_test_eof316: cs = 316; goto _test_eof; 
	_test_eof317: cs = 317; goto _test_eof; 
	_test_eof318: cs = 318; goto _test_eof; 
	_test_eof319: cs = 319; goto _test_eof; 
	_test_eof320: cs = 320; goto _test_eof; 
	_test_eof321: cs = 321; goto _test_eof; 
	_test_eof322: cs = 322; goto _test_eof; 
	_test_eof323: cs = 323; goto _test_eof; 
	_test_eof324: cs = 324; goto _test_eof; 
	_test_eof325: cs = 325; goto _test_eof; 
	_test_eof326: cs = 326; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 449 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
		return 0;
};



