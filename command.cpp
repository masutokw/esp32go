
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
char response [200];
//char tmessage[50];
char tmessage[300];
extern c_star st_now, st_target, st_current;
extern char volatile sync_target;
extern stepper focus_motor;
extern int  focusmax,focuspeed,dcfocus;
extern int  focuspeed_low,focusvolt,focusinv;
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
void conf_tmc(const char *filename);
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
  case 'T':conf_tmc(TMC_FILE);
   break;
  
}


}
void conf_tmc(const char *filename){
File f = SPIFFS.open(filename,"r");
  if (!f) exit;
     f.read((uint8_t *)tmessage,f.size()+1);
	 f.close();   
   }
void conf(void)
{
	
	sprintf(tmessage,"%d\r\n%d\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.4f\r\n%.6f\r\n%.6f\r\n%d\r\n%d\r\n%d\r\n%d\r\n%.0f\r\n%.0f\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n#\r\n",
          telescope->azmotor->maxcounter, telescope->altmotor->maxcounter,
          telescope->rate[0][0], telescope->rate[1][0], telescope->rate[2][0], telescope->rate[3][0],
          telescope->rate[0][1], telescope->rate[1][1], telescope->rate[2][1], telescope->rate[3][1],
		  telescope->prescaler,
          telescope->longitude, telescope->lat, telescope->time_zone,
		  focusmax, focuspeed_low, focuspeed,
		 telescope->azmotor->acceleration / SEC_TO_RAD, telescope->altmotor->acceleration / SEC_TO_RAD,
		 telescope->azmotor->backlash, telescope->altmotor->backlash,
		  telescope->mount_mode ,telescope->track, telescope->autoflip, telescope->azmotor->cw,
		  telescope->altmotor->cw, focusvolt * focusinv,  telescope->azmotor->active, telescope->altmotor->active,
		  dcfocus);
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
static const int command_start = 315;
static const int command_first_final = 315;
static const int command_error = 0;

static const int command_en_main = 315;


#line 222 "command.rl"





    
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
#line 302 "command.rl"
	{;}
	goto st315;
tr21:
#line 300 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	goto st315;
tr22:
#line 301 "command.rl"
	{telescope->track=1;}
	goto st315;
tr24:
#line 260 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	goto st315;
tr26:
#line 303 "command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	goto st315;
tr35:
#line 287 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
	goto st315;
tr37:
#line 285 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
	goto st315;
tr38:
#line 286 "command.rl"
	{gotofocuser(0,focuspeed_low);}
	goto st315;
tr40:
#line 284 "command.rl"
	{gotofocuser(0,focuspeed);}
	goto st315;
tr48:
#line 289 "command.rl"
	{gotofocuser(focus_counter);}
	goto st315;
tr49:
#line 293 "command.rl"
	{sprintf(tmessage,"%d#",focus_motor.state<stop);APPEND;}
	goto st315;
tr59:
#line 291 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
	goto st315;
tr67:
#line 288 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
	goto st315;
tr68:
#line 290 "command.rl"
	{stopfocuser();}
	goto st315;
tr69:
#line 292 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st315;
tr89:
#line 250 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	goto st315;
tr90:
#line 253 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	goto st315;
tr91:
#line 247 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	goto st315;
tr92:
#line 280 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	goto st315;
tr93:
#line 306 "command.rl"
	{sprintf(tmessage, "%d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2));APPEND;}
	goto st315;
tr94:
#line 267 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	goto st315;
tr95:
#line 254 "command.rl"
	{ lxprintsite();}
	goto st315;
tr96:
#line 245 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	goto st315;
tr97:
#line 268 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
	goto st315;
tr98:
#line 283 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	goto st315;
tr104:
#line 307 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
	goto st315;
tr105:
#line 311 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	goto st315;
tr106:
#line 308 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
	goto st315;
tr107:
#line 309 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	goto st315;
tr108:
#line 310 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
	goto st315;
tr109:
#line 249 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	goto st315;
tr110:
#line 282 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	goto st315;
tr111:
#line 252 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	goto st315;
tr112:
#line 256 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	goto st315;
tr113:
#line 305 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
	goto st315;
tr114:
#line 251 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	goto st315;
tr115:
#line 257 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	goto st315;
tr116:
#line 245 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 247 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 249 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 250 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 306 "command.rl"
	{sprintf(tmessage, "%d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2));APPEND;}
#line 292 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st315;
tr122:
#line 312 "command.rl"
	{setwifipad(ip3,ip2);}
	goto st315;
tr129:
#line 239 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
	goto st315;
tr130:
#line 237 "command.rl"
	{mount_move(telescope,stcmd);}
	goto st315;
tr136:
#line 238 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	goto st315;
tr138:
#line 295 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->parked? "1" : "0"));APPEND;}
	goto st315;
tr139:
#line 243 "command.rl"
	{mount_stop(telescope,stcmd);}
	goto st315;
tr142:
#line 244 "command.rl"
	{select_rate(telescope,stcmd); }
	goto st315;
tr158:
#line 276 "command.rl"
	{set_date(min,deg,sec);}
	goto st315;
tr163:
#line 278 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 255 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	goto st315;
tr176:
#line 281 "command.rl"
	{set_time(deg,min,sec);}
	goto st315;
tr186:
#line 269 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st315;
tr207:
#line 313 "command.rl"
	{appcmd(stcmd);APPEND;}
	goto st315;
tr432:
#line 294 "command.rl"
	{mount_goto_home(telescope);}
	goto st315;
tr439:
#line 298 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
	goto st315;
tr440:
#line 296 "command.rl"
	{mount_home_set(telescope);}
	goto st315;
tr441:
#line 297 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
	goto st315;
st315:
	if ( ++p == pe )
		goto _test_eof315;
case 315:
#line 202 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr446;
		case 35: goto st1;
		case 58: goto st2;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr446:
#line 277 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 218 "command.cpp"
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
		case 70: goto st11;
		case 71: goto st42;
		case 73: goto st67;
		case 77: goto st76;
		case 80: goto st85;
		case 81: goto st87;
		case 82: goto st89;
		case 83: goto st92;
		case 97: goto st144;
		case 99: goto st146;
		case 104: goto st305;
		case 112: goto st307;
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
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 43: goto st12;
		case 45: goto st14;
		case 65: goto st16;
		case 66: goto st23;
		case 76: goto st24;
		case 80: goto st33;
		case 81: goto st40;
		case 112: goto st41;
	}
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 35: goto tr35;
		case 43: goto st13;
	}
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 35 )
		goto tr37;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 35: goto tr38;
		case 45: goto st15;
	}
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 35 )
		goto tr40;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 43: goto st17;
		case 45: goto tr42;
	}
	goto st0;
tr42:
#line 236 "command.rl"
	{ neg=-1;}
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 367 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr43;
	goto st0;
tr43:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 377 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr44;
	goto st0;
tr44:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 387 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr45;
	goto st0;
tr45:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 397 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr46;
	goto st0;
tr46:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 407 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr47;
	goto st0;
tr47:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 417 "command.cpp"
	if ( (*p) == 35 )
		goto tr48;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 35 )
		goto tr49;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( (*p) == 83 )
		goto st25;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( (*p) == 49 )
		goto st26;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	switch( (*p) ) {
		case 43: goto st27;
		case 45: goto tr53;
	}
	goto st0;
tr53:
#line 236 "command.rl"
	{ neg=-1;}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 457 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr54;
	goto st0;
tr54:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 467 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr55;
	goto st0;
tr55:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 477 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr56;
	goto st0;
tr56:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 487 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr57;
	goto st0;
tr57:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 497 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr58;
	goto st0;
tr58:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 507 "command.cpp"
	if ( (*p) == 35 )
		goto tr59;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 43: goto st34;
		case 45: goto tr61;
	}
	goto st0;
tr61:
#line 236 "command.rl"
	{ neg=-1;}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 526 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr62;
	goto st0;
tr62:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 536 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr63;
	goto st0;
tr63:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 546 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr64;
	goto st0;
tr64:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 556 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr65;
	goto st0;
tr65:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 566 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr66;
	goto st0;
tr66:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 576 "command.cpp"
	if ( (*p) == 35 )
		goto tr67;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) == 35 )
		goto tr68;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 35 )
		goto tr69;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 65: goto st43;
		case 67: goto st44;
		case 68: goto st45;
		case 71: goto st46;
		case 75: goto st47;
		case 76: goto st48;
		case 77: goto st49;
		case 82: goto st50;
		case 83: goto st51;
		case 84: goto st52;
		case 86: goto st53;
		case 90: goto st59;
		case 99: goto st60;
		case 100: goto st61;
		case 103: goto st62;
		case 107: goto st63;
		case 114: goto st64;
		case 116: goto st65;
		case 120: goto st66;
	}
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 35 )
		goto tr89;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 35 )
		goto tr90;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 35 )
		goto tr91;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 35 )
		goto tr92;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 35 )
		goto tr93;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 35 )
		goto tr94;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 35 )
		goto tr95;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 35 )
		goto tr96;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 35 )
		goto tr97;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( (*p) == 35 )
		goto tr98;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	switch( (*p) ) {
		case 68: goto st54;
		case 70: goto st55;
		case 78: goto st56;
		case 80: goto st57;
		case 84: goto st58;
	}
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 35 )
		goto tr104;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 35 )
		goto tr105;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 35 )
		goto tr106;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 35 )
		goto tr107;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 35 )
		goto tr108;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 35 )
		goto tr109;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 35 )
		goto tr110;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 35 )
		goto tr111;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 35 )
		goto tr112;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 35 )
		goto tr113;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 35 )
		goto tr114;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 35 )
		goto tr115;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 35 )
		goto tr116;
	goto st0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( (*p) == 80 )
		goto st68;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr118;
	goto st0;
tr118:
#line 233 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 813 "command.cpp"
	if ( (*p) == 46 )
		goto st70;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr120;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr121;
	goto st0;
tr121:
#line 234 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 832 "command.cpp"
	if ( (*p) == 35 )
		goto tr122;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr123;
	goto st0;
tr123:
#line 234 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 844 "command.cpp"
	if ( (*p) == 35 )
		goto tr122;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr124;
	goto st0;
tr124:
#line 234 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 856 "command.cpp"
	if ( (*p) == 35 )
		goto tr122;
	goto st0;
tr120:
#line 233 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 866 "command.cpp"
	if ( (*p) == 46 )
		goto st70;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr125;
	goto st0;
tr125:
#line 233 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st75;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
#line 878 "command.cpp"
	if ( (*p) == 46 )
		goto st70;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	switch( (*p) ) {
		case 83: goto st77;
		case 101: goto tr127;
		case 103: goto st79;
		case 104: goto tr127;
		case 110: goto tr127;
		case 115: goto tr127;
		case 119: goto tr127;
	}
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( (*p) == 35 )
		goto tr129;
	goto st0;
tr127:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
#line 909 "command.cpp"
	if ( (*p) == 35 )
		goto tr130;
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	switch( (*p) ) {
		case 101: goto tr131;
		case 110: goto tr131;
		case 115: goto tr131;
		case 119: goto tr131;
	}
	goto st0;
tr131:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 930 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr132;
	goto st0;
tr132:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
#line 940 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr133;
	goto st0;
tr133:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st82;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
#line 950 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr134;
	goto st0;
tr134:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st83;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
#line 960 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr135;
	goto st0;
tr135:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 970 "command.cpp"
	if ( (*p) == 35 )
		goto tr136;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( (*p) == 80 )
		goto st86;
	goto st0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	if ( (*p) == 35 )
		goto tr138;
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 35: goto tr139;
		case 101: goto tr140;
		case 110: goto tr140;
		case 115: goto tr140;
		case 119: goto tr140;
	}
	goto st0;
tr140:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st88;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
#line 1006 "command.cpp"
	if ( (*p) == 35 )
		goto tr139;
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	switch( (*p) ) {
		case 67: goto tr141;
		case 71: goto tr141;
		case 77: goto tr141;
		case 83: goto tr141;
	}
	goto st0;
tr141:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st90;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
#line 1027 "command.cpp"
	if ( (*p) == 35 )
		goto tr142;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st91;
	goto st0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( (*p) == 35 )
		goto tr142;
	goto st0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	switch( (*p) ) {
		case 67: goto st93;
		case 71: goto st103;
		case 76: goto st110;
		case 83: goto tr147;
		case 97: goto tr148;
		case 100: goto tr148;
		case 103: goto tr148;
		case 114: goto tr147;
		case 116: goto tr148;
		case 122: goto tr148;
	}
	goto st0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	if ( (*p) == 32 )
		goto st94;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr150;
	} else if ( (*p) >= 9 )
		goto st94;
	goto st0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr150;
	goto st0;
tr150:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st95;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
#line 1082 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr151;
	goto st0;
tr151:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st96;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
#line 1092 "command.cpp"
	if ( (*p) == 47 )
		goto st97;
	goto st0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr153;
	goto st0;
tr153:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st98;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
#line 1109 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr154;
	goto st0;
tr154:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st99;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
#line 1119 "command.cpp"
	if ( (*p) == 47 )
		goto st100;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr156;
	goto st0;
tr156:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st101;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
#line 1136 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr157;
	goto st0;
tr157:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
#line 1146 "command.cpp"
	if ( (*p) == 35 )
		goto tr158;
	goto st0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	switch( (*p) ) {
		case 32: goto st104;
		case 43: goto st105;
		case 45: goto tr161;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st104;
	goto st0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	switch( (*p) ) {
		case 43: goto st105;
		case 45: goto tr161;
	}
	goto st0;
tr161:
#line 236 "command.rl"
	{ neg=-1;}
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
#line 1177 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr162;
	goto st0;
tr162:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st106;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
#line 1187 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr163;
		case 44: goto st107;
		case 46: goto st107;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr165;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st108;
	goto st0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( (*p) == 35 )
		goto tr163;
	goto st0;
tr165:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 1216 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr163;
		case 44: goto st107;
		case 46: goto st107;
	}
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( (*p) == 32 )
		goto st111;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr168;
	} else if ( (*p) >= 9 )
		goto st111;
	goto st0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr168;
	goto st0;
tr168:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1248 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr169;
	goto st0;
tr169:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st113;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
#line 1258 "command.cpp"
	if ( (*p) == 58 )
		goto st114;
	goto st0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr171;
	goto st0;
tr171:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st115;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
#line 1275 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr172;
	goto st0;
tr172:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st116;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
#line 1285 "command.cpp"
	if ( (*p) == 58 )
		goto st117;
	goto st0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr174;
	goto st0;
tr174:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
#line 1302 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr175;
	goto st0;
tr175:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st119;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
#line 1312 "command.cpp"
	if ( (*p) == 35 )
		goto tr176;
	goto st0;
tr147:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st120;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
#line 1322 "command.cpp"
	if ( (*p) == 32 )
		goto st121;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr178;
	} else if ( (*p) >= 9 )
		goto st121;
	goto st0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr178;
	goto st0;
tr178:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st122;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
#line 1344 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr179;
	goto st0;
tr179:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st123;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
#line 1354 "command.cpp"
	switch( (*p) ) {
		case 47: goto st124;
		case 58: goto st124;
	}
	goto st0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr181;
	goto st0;
tr181:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st125;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
#line 1373 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr182;
	goto st0;
tr182:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 272 "command.rl"
	{deg=deg*3600+min*60;}
	goto st126;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
#line 1384 "command.cpp"
	switch( (*p) ) {
		case 46: goto st127;
		case 47: goto st129;
		case 58: goto st129;
	}
	goto st0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr185;
	goto st0;
tr185:
#line 266 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st128;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
#line 1404 "command.cpp"
	if ( (*p) == 35 )
		goto tr186;
	goto st0;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr187;
	goto st0;
tr187:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st130;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
#line 1421 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr188;
	goto st0;
tr188:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 273 "command.rl"
	{deg+=sec;}
	goto st131;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
#line 1432 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr186;
		case 44: goto st132;
		case 46: goto st132;
		case 58: goto st128;
	}
	goto st0;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st133;
	goto st0;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
	switch( (*p) ) {
		case 35: goto tr186;
		case 58: goto st128;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st134;
	goto st0;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
	switch( (*p) ) {
		case 35: goto tr186;
		case 58: goto st128;
	}
	goto st0;
tr148:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st135;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
#line 1473 "command.cpp"
	switch( (*p) ) {
		case 32: goto st136;
		case 43: goto st137;
		case 45: goto tr195;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr196;
	} else if ( (*p) >= 9 )
		goto st136;
	goto st0;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
	switch( (*p) ) {
		case 32: goto st137;
		case 43: goto st137;
		case 45: goto tr195;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr196;
	} else if ( (*p) >= 9 )
		goto st137;
	goto st0;
tr195:
#line 236 "command.rl"
	{ neg=-1;}
	goto st137;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
#line 1506 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr196;
	goto st0;
tr196:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st138;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
#line 1516 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr198;
		} else if ( (*p) >= 33 )
			goto st139;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st139;
		} else if ( (*p) >= 91 )
			goto st139;
	} else
		goto st139;
	goto st0;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr199;
	goto st0;
tr199:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st140;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
#line 1545 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr200;
	goto st0;
tr200:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 272 "command.rl"
	{deg=deg*3600+min*60;}
	goto st141;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
#line 1556 "command.cpp"
	if ( (*p) == 35 )
		goto tr201;
	goto st129;
tr201:
#line 269 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st316;
st316:
	if ( ++p == pe )
		goto _test_eof316;
case 316:
#line 1568 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr446;
		case 35: goto st1;
		case 58: goto st2;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr187;
	goto st0;
tr198:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st142;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
#line 1583 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr202;
		} else if ( (*p) >= 33 )
			goto st139;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st139;
		} else if ( (*p) >= 91 )
			goto st139;
	} else
		goto st139;
	goto st0;
tr202:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st143;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
#line 1605 "command.cpp"
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st139;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st139;
		} else if ( (*p) >= 91 )
			goto st139;
	} else
		goto st139;
	goto st0;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr203;
	goto st0;
tr203:
#line 314 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st145;
tr442:
#line 331 "command.rl"
	{setflipmode((*p));}
	goto st145;
tr444:
#line 330 "command.rl"
	{setnunchuk((*p));}
	goto st145;
tr445:
#line 299 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st145;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
#line 1648 "command.cpp"
	if ( (*p) == 35 )
		goto st315;
	goto st0;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	switch( (*p) ) {
		case 65: goto tr205;
		case 84: goto tr205;
		case 97: goto tr205;
		case 103: goto tr205;
		case 106: goto tr205;
		case 110: goto tr206;
		case 119: goto tr206;
		case 122: goto tr205;
	}
	if ( 115 <= (*p) && (*p) <= 116 )
		goto tr206;
	goto st0;
tr205:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st147;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
#line 1675 "command.cpp"
	if ( (*p) == 35 )
		goto tr207;
	goto st0;
tr413:
#line 314 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st148;
tr206:
#line 275 "command.rl"
	{stcmd=(*p);mark=p;mark++;}
	goto st148;
tr427:
#line 331 "command.rl"
	{setflipmode((*p));}
	goto st148;
tr429:
#line 330 "command.rl"
	{setnunchuk((*p));}
	goto st148;
tr430:
#line 299 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st148;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
#line 1705 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	goto st148;
tr209:
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr229:
#line 302 "command.rl"
	{;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr230:
#line 300 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr231:
#line 301 "command.rl"
	{telescope->track=1;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr233:
#line 260 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr235:
#line 303 "command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr244:
#line 287 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr246:
#line 285 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr247:
#line 286 "command.rl"
	{gotofocuser(0,focuspeed_low);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr249:
#line 284 "command.rl"
	{gotofocuser(0,focuspeed);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr257:
#line 289 "command.rl"
	{gotofocuser(focus_counter);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr258:
#line 293 "command.rl"
	{sprintf(tmessage,"%d#",focus_motor.state<stop);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr268:
#line 291 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr276:
#line 288 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr277:
#line 290 "command.rl"
	{stopfocuser();}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr278:
#line 292 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr298:
#line 250 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr299:
#line 253 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr300:
#line 247 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr301:
#line 280 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr302:
#line 306 "command.rl"
	{sprintf(tmessage, "%d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2));APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr303:
#line 267 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr304:
#line 254 "command.rl"
	{ lxprintsite();}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr305:
#line 245 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr306:
#line 268 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr307:
#line 283 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr313:
#line 307 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr314:
#line 311 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr315:
#line 308 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr316:
#line 309 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr317:
#line 310 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr318:
#line 249 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr319:
#line 282 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr320:
#line 252 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr321:
#line 256 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr322:
#line 305 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr323:
#line 251 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr324:
#line 257 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr325:
#line 245 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 247 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 249 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 250 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 306 "command.rl"
	{sprintf(tmessage, "%d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2));APPEND;}
#line 292 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr331:
#line 312 "command.rl"
	{setwifipad(ip3,ip2);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr338:
#line 239 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr339:
#line 237 "command.rl"
	{mount_move(telescope,stcmd);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr345:
#line 238 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr347:
#line 295 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->parked? "1" : "0"));APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr348:
#line 243 "command.rl"
	{mount_stop(telescope,stcmd);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr351:
#line 244 "command.rl"
	{select_rate(telescope,stcmd); }
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr367:
#line 276 "command.rl"
	{set_date(min,deg,sec);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr372:
#line 278 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 255 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr385:
#line 281 "command.rl"
	{set_time(deg,min,sec);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr395:
#line 269 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr415:
#line 313 "command.rl"
	{appcmd(stcmd);APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr417:
#line 294 "command.rl"
	{mount_goto_home(telescope);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr424:
#line 298 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr425:
#line 296 "command.rl"
	{mount_home_set(telescope);}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
tr426:
#line 297 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st317;
st317:
	if ( ++p == pe )
		goto _test_eof317;
case 317:
#line 2281 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr448;
		case 35: goto tr209;
		case 58: goto st150;
	}
	goto st148;
tr448:
#line 277 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st149;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
#line 2294 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 58: goto st150;
	}
	goto st148;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	switch( (*p) ) {
		case 35: goto tr209;
		case 65: goto st151;
		case 67: goto st155;
		case 68: goto st158;
		case 70: goto st159;
		case 71: goto st190;
		case 73: goto st215;
		case 77: goto st224;
		case 80: goto st233;
		case 81: goto st235;
		case 82: goto st237;
		case 83: goto st240;
		case 97: goto st292;
		case 99: goto st293;
		case 104: goto st295;
		case 112: goto st297;
	}
	goto st148;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	switch( (*p) ) {
		case 35: goto tr209;
		case 65: goto st152;
		case 76: goto st153;
		case 80: goto st154;
	}
	goto st148;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	if ( (*p) == 35 )
		goto tr229;
	goto st148;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
	if ( (*p) == 35 )
		goto tr230;
	goto st148;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	if ( (*p) == 35 )
		goto tr231;
	goto st148;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	switch( (*p) ) {
		case 35: goto tr209;
		case 77: goto st156;
	}
	goto st148;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	switch( (*p) ) {
		case 35: goto tr233;
		case 82: goto st157;
	}
	goto st148;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	if ( (*p) == 35 )
		goto tr233;
	goto st148;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	if ( (*p) == 35 )
		goto tr235;
	goto st148;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	switch( (*p) ) {
		case 35: goto tr209;
		case 43: goto st160;
		case 45: goto st162;
		case 65: goto st164;
		case 66: goto st171;
		case 76: goto st172;
		case 80: goto st181;
		case 81: goto st188;
		case 112: goto st189;
	}
	goto st148;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
	switch( (*p) ) {
		case 35: goto tr244;
		case 43: goto st161;
	}
	goto st148;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
	if ( (*p) == 35 )
		goto tr246;
	goto st148;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
	switch( (*p) ) {
		case 35: goto tr247;
		case 45: goto st163;
	}
	goto st148;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
	if ( (*p) == 35 )
		goto tr249;
	goto st148;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
	switch( (*p) ) {
		case 35: goto tr209;
		case 43: goto st165;
		case 45: goto tr251;
	}
	goto st148;
tr251:
#line 236 "command.rl"
	{ neg=-1;}
	goto st165;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
#line 2451 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr252;
	goto st148;
tr252:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st166;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
#line 2463 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr253;
	goto st148;
tr253:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st167;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
#line 2475 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr254;
	goto st148;
tr254:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st168;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
#line 2487 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr255;
	goto st148;
tr255:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st169;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
#line 2499 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr256;
	goto st148;
tr256:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st170;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
#line 2511 "command.cpp"
	if ( (*p) == 35 )
		goto tr257;
	goto st148;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
	if ( (*p) == 35 )
		goto tr258;
	goto st148;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
	switch( (*p) ) {
		case 35: goto tr209;
		case 83: goto st173;
	}
	goto st148;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
	switch( (*p) ) {
		case 35: goto tr209;
		case 49: goto st174;
	}
	goto st148;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
	switch( (*p) ) {
		case 35: goto tr209;
		case 43: goto st175;
		case 45: goto tr262;
	}
	goto st148;
tr262:
#line 236 "command.rl"
	{ neg=-1;}
	goto st175;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
#line 2556 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr263;
	goto st148;
tr263:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st176;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
#line 2568 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr264;
	goto st148;
tr264:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st177;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
#line 2580 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr265;
	goto st148;
tr265:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st178;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
#line 2592 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr266;
	goto st148;
tr266:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st179;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
#line 2604 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr267;
	goto st148;
tr267:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st180;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
#line 2616 "command.cpp"
	if ( (*p) == 35 )
		goto tr268;
	goto st148;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
	switch( (*p) ) {
		case 35: goto tr209;
		case 43: goto st182;
		case 45: goto tr270;
	}
	goto st148;
tr270:
#line 236 "command.rl"
	{ neg=-1;}
	goto st182;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
#line 2636 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr271;
	goto st148;
tr271:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st183;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
#line 2648 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr272;
	goto st148;
tr272:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st184;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
#line 2660 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr273;
	goto st148;
tr273:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st185;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
#line 2672 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr274;
	goto st148;
tr274:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st186;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
#line 2684 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr275;
	goto st148;
tr275:
#line 235 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st187;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
#line 2696 "command.cpp"
	if ( (*p) == 35 )
		goto tr276;
	goto st148;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
	if ( (*p) == 35 )
		goto tr277;
	goto st148;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
	if ( (*p) == 35 )
		goto tr278;
	goto st148;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
	switch( (*p) ) {
		case 35: goto tr209;
		case 65: goto st191;
		case 67: goto st192;
		case 68: goto st193;
		case 71: goto st194;
		case 75: goto st195;
		case 76: goto st196;
		case 77: goto st197;
		case 82: goto st198;
		case 83: goto st199;
		case 84: goto st200;
		case 86: goto st201;
		case 90: goto st207;
		case 99: goto st208;
		case 100: goto st209;
		case 103: goto st210;
		case 107: goto st211;
		case 114: goto st212;
		case 116: goto st213;
		case 120: goto st214;
	}
	goto st148;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
	if ( (*p) == 35 )
		goto tr298;
	goto st148;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
	if ( (*p) == 35 )
		goto tr299;
	goto st148;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	if ( (*p) == 35 )
		goto tr300;
	goto st148;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	if ( (*p) == 35 )
		goto tr301;
	goto st148;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	if ( (*p) == 35 )
		goto tr302;
	goto st148;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
	if ( (*p) == 35 )
		goto tr303;
	goto st148;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
	if ( (*p) == 35 )
		goto tr304;
	goto st148;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	if ( (*p) == 35 )
		goto tr305;
	goto st148;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	if ( (*p) == 35 )
		goto tr306;
	goto st148;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	if ( (*p) == 35 )
		goto tr307;
	goto st148;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	switch( (*p) ) {
		case 35: goto tr209;
		case 68: goto st202;
		case 70: goto st203;
		case 78: goto st204;
		case 80: goto st205;
		case 84: goto st206;
	}
	goto st148;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	if ( (*p) == 35 )
		goto tr313;
	goto st148;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
	if ( (*p) == 35 )
		goto tr314;
	goto st148;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
	if ( (*p) == 35 )
		goto tr315;
	goto st148;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
	if ( (*p) == 35 )
		goto tr316;
	goto st148;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
	if ( (*p) == 35 )
		goto tr317;
	goto st148;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	if ( (*p) == 35 )
		goto tr318;
	goto st148;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	if ( (*p) == 35 )
		goto tr319;
	goto st148;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
	if ( (*p) == 35 )
		goto tr320;
	goto st148;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
	if ( (*p) == 35 )
		goto tr321;
	goto st148;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	if ( (*p) == 35 )
		goto tr322;
	goto st148;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	if ( (*p) == 35 )
		goto tr323;
	goto st148;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
	if ( (*p) == 35 )
		goto tr324;
	goto st148;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
	if ( (*p) == 35 )
		goto tr325;
	goto st148;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
	switch( (*p) ) {
		case 35: goto tr209;
		case 80: goto st216;
	}
	goto st148;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr327;
	goto st148;
tr327:
#line 233 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st217;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
#line 2939 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 46: goto st218;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr329;
	goto st148;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr330;
	goto st148;
tr330:
#line 234 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st219;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
#line 2962 "command.cpp"
	if ( (*p) == 35 )
		goto tr331;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr332;
	goto st148;
tr332:
#line 234 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st220;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
#line 2974 "command.cpp"
	if ( (*p) == 35 )
		goto tr331;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr333;
	goto st148;
tr333:
#line 234 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st221;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
#line 2986 "command.cpp"
	if ( (*p) == 35 )
		goto tr331;
	goto st148;
tr329:
#line 233 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st222;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
#line 2996 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 46: goto st218;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr334;
	goto st148;
tr334:
#line 233 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st223;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
#line 3010 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 46: goto st218;
	}
	goto st148;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
	switch( (*p) ) {
		case 35: goto tr209;
		case 83: goto st225;
		case 101: goto tr336;
		case 103: goto st227;
		case 104: goto tr336;
		case 110: goto tr336;
		case 115: goto tr336;
		case 119: goto tr336;
	}
	goto st148;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
	if ( (*p) == 35 )
		goto tr338;
	goto st148;
tr336:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st226;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
#line 3044 "command.cpp"
	if ( (*p) == 35 )
		goto tr339;
	goto st148;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
	switch( (*p) ) {
		case 35: goto tr209;
		case 101: goto tr340;
		case 110: goto tr340;
		case 115: goto tr340;
		case 119: goto tr340;
	}
	goto st148;
tr340:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st228;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
#line 3066 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr341;
	goto st148;
tr341:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st229;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
#line 3078 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr342;
	goto st148;
tr342:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st230;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
#line 3090 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr343;
	goto st148;
tr343:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st231;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
#line 3102 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr344;
	goto st148;
tr344:
#line 232 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st232;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
#line 3114 "command.cpp"
	if ( (*p) == 35 )
		goto tr345;
	goto st148;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	switch( (*p) ) {
		case 35: goto tr209;
		case 80: goto st234;
	}
	goto st148;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
	if ( (*p) == 35 )
		goto tr347;
	goto st148;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
	switch( (*p) ) {
		case 35: goto tr348;
		case 101: goto tr349;
		case 110: goto tr349;
		case 115: goto tr349;
		case 119: goto tr349;
	}
	goto st148;
tr349:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st236;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
#line 3152 "command.cpp"
	if ( (*p) == 35 )
		goto tr348;
	goto st148;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
	switch( (*p) ) {
		case 35: goto tr209;
		case 67: goto tr350;
		case 71: goto tr350;
		case 77: goto tr350;
		case 83: goto tr350;
	}
	goto st148;
tr350:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st238;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
#line 3174 "command.cpp"
	if ( (*p) == 35 )
		goto tr351;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st239;
	goto st148;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
	if ( (*p) == 35 )
		goto tr351;
	goto st148;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
	switch( (*p) ) {
		case 35: goto tr209;
		case 67: goto st241;
		case 71: goto st251;
		case 76: goto st258;
		case 83: goto tr356;
		case 97: goto tr357;
		case 100: goto tr357;
		case 103: goto tr357;
		case 114: goto tr356;
		case 116: goto tr357;
		case 122: goto tr357;
	}
	goto st148;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
	switch( (*p) ) {
		case 32: goto st242;
		case 35: goto tr209;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr359;
	} else if ( (*p) >= 9 )
		goto st242;
	goto st148;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr359;
	goto st148;
tr359:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st243;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
#line 3234 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr360;
	goto st148;
tr360:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st244;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
#line 3246 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 47: goto st245;
	}
	goto st148;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr362;
	goto st148;
tr362:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st246;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
#line 3267 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr363;
	goto st148;
tr363:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st247;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
#line 3279 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 47: goto st248;
	}
	goto st148;
st248:
	if ( ++p == pe )
		goto _test_eof248;
case 248:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr365;
	goto st148;
tr365:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st249;
st249:
	if ( ++p == pe )
		goto _test_eof249;
case 249:
#line 3300 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr366;
	goto st148;
tr366:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st250;
st250:
	if ( ++p == pe )
		goto _test_eof250;
case 250:
#line 3312 "command.cpp"
	if ( (*p) == 35 )
		goto tr367;
	goto st148;
st251:
	if ( ++p == pe )
		goto _test_eof251;
case 251:
	switch( (*p) ) {
		case 32: goto st252;
		case 35: goto tr209;
		case 43: goto st253;
		case 45: goto tr370;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st252;
	goto st148;
st252:
	if ( ++p == pe )
		goto _test_eof252;
case 252:
	switch( (*p) ) {
		case 35: goto tr209;
		case 43: goto st253;
		case 45: goto tr370;
	}
	goto st148;
tr370:
#line 236 "command.rl"
	{ neg=-1;}
	goto st253;
st253:
	if ( ++p == pe )
		goto _test_eof253;
case 253:
#line 3345 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr371;
	goto st148;
tr371:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st254;
st254:
	if ( ++p == pe )
		goto _test_eof254;
case 254:
#line 3357 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr372;
		case 44: goto st255;
		case 46: goto st255;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr374;
	goto st148;
st255:
	if ( ++p == pe )
		goto _test_eof255;
case 255:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st256;
	goto st148;
st256:
	if ( ++p == pe )
		goto _test_eof256;
case 256:
	if ( (*p) == 35 )
		goto tr372;
	goto st148;
tr374:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st257;
st257:
	if ( ++p == pe )
		goto _test_eof257;
case 257:
#line 3388 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr372;
		case 44: goto st255;
		case 46: goto st255;
	}
	goto st148;
st258:
	if ( ++p == pe )
		goto _test_eof258;
case 258:
	switch( (*p) ) {
		case 32: goto st259;
		case 35: goto tr209;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr377;
	} else if ( (*p) >= 9 )
		goto st259;
	goto st148;
st259:
	if ( ++p == pe )
		goto _test_eof259;
case 259:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr377;
	goto st148;
tr377:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st260;
st260:
	if ( ++p == pe )
		goto _test_eof260;
case 260:
#line 3424 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr378;
	goto st148;
tr378:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st261;
st261:
	if ( ++p == pe )
		goto _test_eof261;
case 261:
#line 3436 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 58: goto st262;
	}
	goto st148;
st262:
	if ( ++p == pe )
		goto _test_eof262;
case 262:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr380;
	goto st148;
tr380:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st263;
st263:
	if ( ++p == pe )
		goto _test_eof263;
case 263:
#line 3457 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr381;
	goto st148;
tr381:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st264;
st264:
	if ( ++p == pe )
		goto _test_eof264;
case 264:
#line 3469 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 58: goto st265;
	}
	goto st148;
st265:
	if ( ++p == pe )
		goto _test_eof265;
case 265:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr383;
	goto st148;
tr383:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st266;
st266:
	if ( ++p == pe )
		goto _test_eof266;
case 266:
#line 3490 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr384;
	goto st148;
tr384:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st267;
st267:
	if ( ++p == pe )
		goto _test_eof267;
case 267:
#line 3502 "command.cpp"
	if ( (*p) == 35 )
		goto tr385;
	goto st148;
tr356:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st268;
st268:
	if ( ++p == pe )
		goto _test_eof268;
case 268:
#line 3512 "command.cpp"
	switch( (*p) ) {
		case 32: goto st269;
		case 35: goto tr209;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr387;
	} else if ( (*p) >= 9 )
		goto st269;
	goto st148;
st269:
	if ( ++p == pe )
		goto _test_eof269;
case 269:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr387;
	goto st148;
tr387:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st270;
st270:
	if ( ++p == pe )
		goto _test_eof270;
case 270:
#line 3538 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr388;
	goto st148;
tr388:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st271;
st271:
	if ( ++p == pe )
		goto _test_eof271;
case 271:
#line 3550 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 47: goto st272;
		case 58: goto st272;
	}
	goto st148;
st272:
	if ( ++p == pe )
		goto _test_eof272;
case 272:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr390;
	goto st148;
tr390:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st273;
st273:
	if ( ++p == pe )
		goto _test_eof273;
case 273:
#line 3572 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr391;
	goto st148;
tr391:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 272 "command.rl"
	{deg=deg*3600+min*60;}
	goto st274;
st274:
	if ( ++p == pe )
		goto _test_eof274;
case 274:
#line 3585 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr209;
		case 46: goto st275;
		case 47: goto st277;
		case 58: goto st277;
	}
	goto st148;
st275:
	if ( ++p == pe )
		goto _test_eof275;
case 275:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr394;
	goto st148;
tr394:
#line 266 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st276;
st276:
	if ( ++p == pe )
		goto _test_eof276;
case 276:
#line 3608 "command.cpp"
	if ( (*p) == 35 )
		goto tr395;
	goto st148;
st277:
	if ( ++p == pe )
		goto _test_eof277;
case 277:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr396;
	goto st148;
tr396:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st278;
st278:
	if ( ++p == pe )
		goto _test_eof278;
case 278:
#line 3627 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr397;
	goto st148;
tr397:
#line 231 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 273 "command.rl"
	{deg+=sec;}
	goto st279;
st279:
	if ( ++p == pe )
		goto _test_eof279;
case 279:
#line 3640 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr395;
		case 44: goto st280;
		case 46: goto st280;
		case 58: goto st276;
	}
	goto st148;
st280:
	if ( ++p == pe )
		goto _test_eof280;
case 280:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st281;
	goto st148;
st281:
	if ( ++p == pe )
		goto _test_eof281;
case 281:
	switch( (*p) ) {
		case 35: goto tr395;
		case 58: goto st276;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st282;
	goto st148;
st282:
	if ( ++p == pe )
		goto _test_eof282;
case 282:
	switch( (*p) ) {
		case 35: goto tr395;
		case 58: goto st276;
	}
	goto st148;
tr357:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st283;
st283:
	if ( ++p == pe )
		goto _test_eof283;
case 283:
#line 3683 "command.cpp"
	switch( (*p) ) {
		case 32: goto st284;
		case 35: goto tr209;
		case 43: goto st285;
		case 45: goto tr404;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr405;
	} else if ( (*p) >= 9 )
		goto st284;
	goto st148;
st284:
	if ( ++p == pe )
		goto _test_eof284;
case 284:
	switch( (*p) ) {
		case 32: goto st285;
		case 35: goto tr209;
		case 43: goto st285;
		case 45: goto tr404;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr405;
	} else if ( (*p) >= 9 )
		goto st285;
	goto st148;
tr404:
#line 236 "command.rl"
	{ neg=-1;}
	goto st285;
st285:
	if ( ++p == pe )
		goto _test_eof285;
case 285:
#line 3718 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr405;
	goto st148;
tr405:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st286;
st286:
	if ( ++p == pe )
		goto _test_eof286;
case 286:
#line 3730 "command.cpp"
	if ( (*p) == 35 )
		goto tr407;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr408;
		} else if ( (*p) >= 33 )
			goto st287;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st287;
		} else if ( (*p) >= 91 )
			goto st287;
	} else
		goto st287;
	goto st148;
st287:
	if ( ++p == pe )
		goto _test_eof287;
case 287:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr409;
	goto st148;
tr409:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st288;
st288:
	if ( ++p == pe )
		goto _test_eof288;
case 288:
#line 3763 "command.cpp"
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr410;
	goto st148;
tr410:
#line 230 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 272 "command.rl"
	{deg=deg*3600+min*60;}
	goto st289;
st289:
	if ( ++p == pe )
		goto _test_eof289;
case 289:
#line 3776 "command.cpp"
	if ( (*p) == 35 )
		goto tr411;
	goto st277;
tr411:
#line 269 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st318;
st318:
	if ( ++p == pe )
		goto _test_eof318;
case 318:
#line 3795 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr448;
		case 35: goto tr209;
		case 58: goto st150;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr396;
	goto st148;
tr407:
#line 323 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						}
						}
	goto st319;
st319:
	if ( ++p == pe )
		goto _test_eof319;
case 319:
#line 3816 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr448;
		case 35: goto tr209;
		case 58: goto st150;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr409;
	goto st148;
tr408:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st290;
st290:
	if ( ++p == pe )
		goto _test_eof290;
case 290:
#line 3831 "command.cpp"
	if ( (*p) == 35 )
		goto tr407;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr412;
		} else if ( (*p) >= 33 )
			goto st287;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st287;
		} else if ( (*p) >= 91 )
			goto st287;
	} else
		goto st287;
	goto st148;
tr412:
#line 229 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st291;
st291:
	if ( ++p == pe )
		goto _test_eof291;
case 291:
#line 3855 "command.cpp"
	if ( (*p) == 35 )
		goto tr407;
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st287;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st287;
		} else if ( (*p) >= 91 )
			goto st287;
	} else
		goto st287;
	goto st148;
st292:
	if ( ++p == pe )
		goto _test_eof292;
case 292:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr413;
	goto st148;
st293:
	if ( ++p == pe )
		goto _test_eof293;
case 293:
	switch( (*p) ) {
		case 35: goto tr209;
		case 65: goto tr414;
		case 84: goto tr414;
		case 97: goto tr414;
		case 103: goto tr414;
		case 106: goto tr414;
		case 110: goto tr206;
		case 119: goto tr206;
		case 122: goto tr414;
	}
	if ( 115 <= (*p) && (*p) <= 116 )
		goto tr206;
	goto st148;
tr414:
#line 274 "command.rl"
	{stcmd=(*p);}
	goto st294;
st294:
	if ( ++p == pe )
		goto _test_eof294;
case 294:
#line 3903 "command.cpp"
	if ( (*p) == 35 )
		goto tr415;
	goto st148;
st295:
	if ( ++p == pe )
		goto _test_eof295;
case 295:
	switch( (*p) ) {
		case 35: goto tr209;
		case 80: goto st296;
	}
	goto st148;
st296:
	if ( ++p == pe )
		goto _test_eof296;
case 296:
	if ( (*p) == 35 )
		goto tr417;
	goto st148;
st297:
	if ( ++p == pe )
		goto _test_eof297;
case 297:
	switch( (*p) ) {
		case 35: goto tr209;
		case 70: goto st298;
		case 72: goto st299;
		case 83: goto st300;
		case 97: goto st301;
		case 110: goto st302;
		case 115: goto st304;
	}
	goto st148;
st298:
	if ( ++p == pe )
		goto _test_eof298;
case 298:
	if ( (*p) == 35 )
		goto tr424;
	goto st148;
st299:
	if ( ++p == pe )
		goto _test_eof299;
case 299:
	if ( (*p) == 35 )
		goto tr425;
	goto st148;
st300:
	if ( ++p == pe )
		goto _test_eof300;
case 300:
	if ( (*p) == 35 )
		goto tr426;
	goto st148;
st301:
	if ( ++p == pe )
		goto _test_eof301;
case 301:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr427;
	goto st148;
st302:
	if ( ++p == pe )
		goto _test_eof302;
case 302:
	switch( (*p) ) {
		case 35: goto tr209;
		case 107: goto st303;
	}
	goto st148;
st303:
	if ( ++p == pe )
		goto _test_eof303;
case 303:
	if ( (*p) == 35 )
		goto tr209;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr429;
	goto st148;
st304:
	if ( ++p == pe )
		goto _test_eof304;
case 304:
	switch( (*p) ) {
		case 35: goto tr209;
		case 101: goto tr430;
		case 119: goto tr430;
	}
	goto st148;
st305:
	if ( ++p == pe )
		goto _test_eof305;
case 305:
	if ( (*p) == 80 )
		goto st306;
	goto st0;
st306:
	if ( ++p == pe )
		goto _test_eof306;
case 306:
	if ( (*p) == 35 )
		goto tr432;
	goto st0;
st307:
	if ( ++p == pe )
		goto _test_eof307;
case 307:
	switch( (*p) ) {
		case 70: goto st308;
		case 72: goto st309;
		case 83: goto st310;
		case 97: goto st311;
		case 110: goto st312;
		case 115: goto st314;
	}
	goto st0;
st308:
	if ( ++p == pe )
		goto _test_eof308;
case 308:
	if ( (*p) == 35 )
		goto tr439;
	goto st0;
st309:
	if ( ++p == pe )
		goto _test_eof309;
case 309:
	if ( (*p) == 35 )
		goto tr440;
	goto st0;
st310:
	if ( ++p == pe )
		goto _test_eof310;
case 310:
	if ( (*p) == 35 )
		goto tr441;
	goto st0;
st311:
	if ( ++p == pe )
		goto _test_eof311;
case 311:
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr442;
	goto st0;
st312:
	if ( ++p == pe )
		goto _test_eof312;
case 312:
	if ( (*p) == 107 )
		goto st313;
	goto st0;
st313:
	if ( ++p == pe )
		goto _test_eof313;
case 313:
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr444;
	goto st0;
st314:
	if ( ++p == pe )
		goto _test_eof314;
case 314:
	switch( (*p) ) {
		case 101: goto tr445;
		case 119: goto tr445;
	}
	goto st0;
	}
	_test_eof315: cs = 315; goto _test_eof; 
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
	_test_eof316: cs = 316; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
	_test_eof317: cs = 317; goto _test_eof; 
	_test_eof149: cs = 149; goto _test_eof; 
	_test_eof150: cs = 150; goto _test_eof; 
	_test_eof151: cs = 151; goto _test_eof; 
	_test_eof152: cs = 152; goto _test_eof; 
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
	_test_eof318: cs = 318; goto _test_eof; 
	_test_eof319: cs = 319; goto _test_eof; 
	_test_eof290: cs = 290; goto _test_eof; 
	_test_eof291: cs = 291; goto _test_eof; 
	_test_eof292: cs = 292; goto _test_eof; 
	_test_eof293: cs = 293; goto _test_eof; 
	_test_eof294: cs = 294; goto _test_eof; 
	_test_eof295: cs = 295; goto _test_eof; 
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

	_test_eof: {}
	_out: {}
	}

#line 414 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
		return 0;
};



