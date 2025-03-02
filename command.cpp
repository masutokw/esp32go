
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
  
}


}
void conf(void)
{
	
	sprintf(tmessage,"%d\r\n%d\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.4f\r\n%.6f\r\n%.6f\r\n%d\r\n%d\r\n%d\r\n%d\r\n%.0f\r\n%.0f\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
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
static const int command_start = 311;
static const int command_first_final = 311;
static const int command_error = 0;

static const int command_en_main = 311;


#line 214 "command.rl"





    
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
tr19:
#line 293 "command.rl"
	{;}
	goto st311;
tr20:
#line 291 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	goto st311;
tr21:
#line 292 "command.rl"
	{telescope->track=1;}
	goto st311;
tr23:
#line 252 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	goto st311;
tr25:
#line 294 "command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	goto st311;
tr34:
#line 279 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
	goto st311;
tr36:
#line 277 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
	goto st311;
tr37:
#line 278 "command.rl"
	{gotofocuser(0,focuspeed_low);}
	goto st311;
tr39:
#line 276 "command.rl"
	{gotofocuser(0,focuspeed);}
	goto st311;
tr47:
#line 281 "command.rl"
	{gotofocuser(focus_counter);}
	goto st311;
tr48:
#line 285 "command.rl"
	{sprintf(tmessage,"%d#",focus_motor.state<stop);APPEND;}
	goto st311;
tr58:
#line 283 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
	goto st311;
tr66:
#line 280 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
	goto st311;
tr67:
#line 282 "command.rl"
	{stopfocuser();}
	goto st311;
tr68:
#line 284 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st311;
tr88:
#line 242 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	goto st311;
tr89:
#line 245 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	goto st311;
tr90:
#line 239 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	goto st311;
tr91:
#line 272 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	goto st311;
tr92:
#line 297 "command.rl"
	{sprintf(tmessage,"%s#", telescope->is_tracking ? "1":"0");APPEND;}
	goto st311;
tr93:
#line 259 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	goto st311;
tr94:
#line 246 "command.rl"
	{ lxprintsite();}
	goto st311;
tr95:
#line 237 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	goto st311;
tr96:
#line 260 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
	goto st311;
tr97:
#line 275 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	goto st311;
tr103:
#line 298 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
	goto st311;
tr104:
#line 302 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	goto st311;
tr105:
#line 299 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
	goto st311;
tr106:
#line 300 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	goto st311;
tr107:
#line 301 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
	goto st311;
tr108:
#line 241 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	goto st311;
tr109:
#line 237 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 239 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 241 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 242 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 297 "command.rl"
	{sprintf(tmessage,"%s#", telescope->is_tracking ? "1":"0");APPEND;}
	goto st311;
tr110:
#line 274 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	goto st311;
tr111:
#line 244 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	goto st311;
tr112:
#line 248 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	goto st311;
tr113:
#line 296 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
	goto st311;
tr114:
#line 243 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	goto st311;
tr115:
#line 249 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	goto st311;
tr121:
#line 303 "command.rl"
	{setwifipad(ip3,ip2);}
	goto st311;
tr128:
#line 231 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
	goto st311;
tr129:
#line 229 "command.rl"
	{mount_move(telescope,stcmd);}
	goto st311;
tr135:
#line 230 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	goto st311;
tr136:
#line 235 "command.rl"
	{mount_stop(telescope,stcmd);}
	goto st311;
tr139:
#line 236 "command.rl"
	{select_rate(telescope,stcmd); }
	goto st311;
tr155:
#line 268 "command.rl"
	{set_date(min,deg,sec);}
	goto st311;
tr160:
#line 270 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 247 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	goto st311;
tr173:
#line 273 "command.rl"
	{set_time(deg,min,sec);}
	goto st311;
tr183:
#line 261 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st311;
tr204:
#line 304 "command.rl"
	{appcmd(stcmd);APPEND;}
	goto st311;
tr426:
#line 286 "command.rl"
	{mount_goto_home(telescope);}
	goto st311;
tr433:
#line 289 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
	goto st311;
tr434:
#line 287 "command.rl"
	{mount_home_set(telescope);}
	goto st311;
tr435:
#line 288 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
	goto st311;
st311:
	if ( ++p == pe )
		goto _test_eof311;
case 311:
#line 198 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr440;
		case 35: goto st1;
		case 58: goto st2;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr440:
#line 269 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 214 "command.cpp"
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
		case 81: goto st85;
		case 82: goto st87;
		case 83: goto st90;
		case 97: goto st142;
		case 99: goto st144;
		case 104: goto st301;
		case 112: goto st303;
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
		goto tr19;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 35 )
		goto tr20;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 35 )
		goto tr21;
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
		case 35: goto tr23;
		case 82: goto st9;
	}
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 35 )
		goto tr23;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 35 )
		goto tr25;
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
		case 35: goto tr34;
		case 43: goto st13;
	}
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 35 )
		goto tr36;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 35: goto tr37;
		case 45: goto st15;
	}
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 35 )
		goto tr39;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 43: goto st17;
		case 45: goto tr41;
	}
	goto st0;
tr41:
#line 228 "command.rl"
	{ neg=-1;}
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 362 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr42;
	goto st0;
tr42:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 372 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr43;
	goto st0;
tr43:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 382 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr44;
	goto st0;
tr44:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 392 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr45;
	goto st0;
tr45:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 402 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr46;
	goto st0;
tr46:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 412 "command.cpp"
	if ( (*p) == 35 )
		goto tr47;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 35 )
		goto tr48;
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
		case 45: goto tr52;
	}
	goto st0;
tr52:
#line 228 "command.rl"
	{ neg=-1;}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 452 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr53;
	goto st0;
tr53:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 462 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr54;
	goto st0;
tr54:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 472 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr55;
	goto st0;
tr55:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 482 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr56;
	goto st0;
tr56:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 492 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr57;
	goto st0;
tr57:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 502 "command.cpp"
	if ( (*p) == 35 )
		goto tr58;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 43: goto st34;
		case 45: goto tr60;
	}
	goto st0;
tr60:
#line 228 "command.rl"
	{ neg=-1;}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 521 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr61;
	goto st0;
tr61:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 531 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr62;
	goto st0;
tr62:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 541 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr63;
	goto st0;
tr63:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 551 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr64;
	goto st0;
tr64:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 561 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr65;
	goto st0;
tr65:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 571 "command.cpp"
	if ( (*p) == 35 )
		goto tr66;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) == 35 )
		goto tr67;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 35 )
		goto tr68;
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
		case 97: goto st60;
		case 99: goto st61;
		case 100: goto st62;
		case 103: goto st63;
		case 107: goto st64;
		case 114: goto st65;
		case 116: goto st66;
	}
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 35 )
		goto tr88;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 35 )
		goto tr89;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 35 )
		goto tr90;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 35 )
		goto tr91;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 35 )
		goto tr92;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 35 )
		goto tr93;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 35 )
		goto tr94;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 35 )
		goto tr95;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 35 )
		goto tr96;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( (*p) == 35 )
		goto tr97;
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
		goto tr103;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 35 )
		goto tr104;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 35 )
		goto tr105;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 35 )
		goto tr106;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 35 )
		goto tr107;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 35 )
		goto tr108;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 35 )
		goto tr109;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 35 )
		goto tr110;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 35 )
		goto tr111;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 35 )
		goto tr112;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 35 )
		goto tr113;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 35 )
		goto tr114;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 35 )
		goto tr115;
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
		goto tr117;
	goto st0;
tr117:
#line 225 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 808 "command.cpp"
	if ( (*p) == 46 )
		goto st70;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr119;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr120;
	goto st0;
tr120:
#line 226 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 827 "command.cpp"
	if ( (*p) == 35 )
		goto tr121;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr122;
	goto st0;
tr122:
#line 226 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 839 "command.cpp"
	if ( (*p) == 35 )
		goto tr121;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr123;
	goto st0;
tr123:
#line 226 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 851 "command.cpp"
	if ( (*p) == 35 )
		goto tr121;
	goto st0;
tr119:
#line 225 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 861 "command.cpp"
	if ( (*p) == 46 )
		goto st70;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr124;
	goto st0;
tr124:
#line 225 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st75;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
#line 873 "command.cpp"
	if ( (*p) == 46 )
		goto st70;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	switch( (*p) ) {
		case 83: goto st77;
		case 101: goto tr126;
		case 103: goto st79;
		case 104: goto tr126;
		case 110: goto tr126;
		case 115: goto tr126;
		case 119: goto tr126;
	}
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( (*p) == 35 )
		goto tr128;
	goto st0;
tr126:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
#line 904 "command.cpp"
	if ( (*p) == 35 )
		goto tr129;
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	switch( (*p) ) {
		case 101: goto tr130;
		case 110: goto tr130;
		case 115: goto tr130;
		case 119: goto tr130;
	}
	goto st0;
tr130:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 925 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr131;
	goto st0;
tr131:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
#line 935 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr132;
	goto st0;
tr132:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st82;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
#line 945 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr133;
	goto st0;
tr133:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st83;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
#line 955 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr134;
	goto st0;
tr134:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 965 "command.cpp"
	if ( (*p) == 35 )
		goto tr135;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 35: goto tr136;
		case 101: goto tr137;
		case 110: goto tr137;
		case 115: goto tr137;
		case 119: goto tr137;
	}
	goto st0;
tr137:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
#line 987 "command.cpp"
	if ( (*p) == 35 )
		goto tr136;
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 67: goto tr138;
		case 71: goto tr138;
		case 77: goto tr138;
		case 83: goto tr138;
	}
	goto st0;
tr138:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st88;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
#line 1008 "command.cpp"
	if ( (*p) == 35 )
		goto tr139;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st89;
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( (*p) == 35 )
		goto tr139;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	switch( (*p) ) {
		case 67: goto st91;
		case 71: goto st101;
		case 76: goto st108;
		case 83: goto tr144;
		case 97: goto tr145;
		case 100: goto tr145;
		case 103: goto tr145;
		case 114: goto tr144;
		case 116: goto tr145;
		case 122: goto tr145;
	}
	goto st0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( (*p) == 32 )
		goto st92;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr147;
	} else if ( (*p) >= 9 )
		goto st92;
	goto st0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr147;
	goto st0;
tr147:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st93;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
#line 1063 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr148;
	goto st0;
tr148:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st94;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
#line 1073 "command.cpp"
	if ( (*p) == 47 )
		goto st95;
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr150;
	goto st0;
tr150:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st96;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
#line 1090 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr151;
	goto st0;
tr151:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st97;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
#line 1100 "command.cpp"
	if ( (*p) == 47 )
		goto st98;
	goto st0;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr153;
	goto st0;
tr153:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st99;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
#line 1117 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr154;
	goto st0;
tr154:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st100;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
#line 1127 "command.cpp"
	if ( (*p) == 35 )
		goto tr155;
	goto st0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	switch( (*p) ) {
		case 32: goto st102;
		case 43: goto st103;
		case 45: goto tr158;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st102;
	goto st0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	switch( (*p) ) {
		case 43: goto st103;
		case 45: goto tr158;
	}
	goto st0;
tr158:
#line 228 "command.rl"
	{ neg=-1;}
	goto st103;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
#line 1158 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr159;
	goto st0;
tr159:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st104;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
#line 1168 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr160;
		case 44: goto st105;
		case 46: goto st105;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr162;
	goto st0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st106;
	goto st0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	if ( (*p) == 35 )
		goto tr160;
	goto st0;
tr162:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st107;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
#line 1197 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr160;
		case 44: goto st105;
		case 46: goto st105;
	}
	goto st0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( (*p) == 32 )
		goto st109;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr165;
	} else if ( (*p) >= 9 )
		goto st109;
	goto st0;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr165;
	goto st0;
tr165:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st110;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
#line 1229 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr166;
	goto st0;
tr166:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st111;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
#line 1239 "command.cpp"
	if ( (*p) == 58 )
		goto st112;
	goto st0;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr168;
	goto st0;
tr168:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st113;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
#line 1256 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr169;
	goto st0;
tr169:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st114;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
#line 1266 "command.cpp"
	if ( (*p) == 58 )
		goto st115;
	goto st0;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr171;
	goto st0;
tr171:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st116;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
#line 1283 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr172;
	goto st0;
tr172:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st117;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
#line 1293 "command.cpp"
	if ( (*p) == 35 )
		goto tr173;
	goto st0;
tr144:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
#line 1303 "command.cpp"
	if ( (*p) == 32 )
		goto st119;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr175;
	} else if ( (*p) >= 9 )
		goto st119;
	goto st0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr175;
	goto st0;
tr175:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st120;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
#line 1325 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr176;
	goto st0;
tr176:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st121;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
#line 1335 "command.cpp"
	switch( (*p) ) {
		case 47: goto st122;
		case 58: goto st122;
	}
	goto st0;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr178;
	goto st0;
tr178:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st123;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
#line 1354 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr179;
	goto st0;
tr179:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 264 "command.rl"
	{deg=deg*3600+min*60;}
	goto st124;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
#line 1365 "command.cpp"
	switch( (*p) ) {
		case 46: goto st125;
		case 47: goto st127;
		case 58: goto st127;
	}
	goto st0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr182;
	goto st0;
tr182:
#line 258 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st126;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
#line 1385 "command.cpp"
	if ( (*p) == 35 )
		goto tr183;
	goto st0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr184;
	goto st0;
tr184:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st128;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
#line 1402 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr185;
	goto st0;
tr185:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 265 "command.rl"
	{deg+=sec;}
	goto st129;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
#line 1413 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr183;
		case 44: goto st130;
		case 46: goto st130;
		case 58: goto st126;
	}
	goto st0;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st131;
	goto st0;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
	switch( (*p) ) {
		case 35: goto tr183;
		case 58: goto st126;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st132;
	goto st0;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	switch( (*p) ) {
		case 35: goto tr183;
		case 58: goto st126;
	}
	goto st0;
tr145:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st133;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
#line 1454 "command.cpp"
	switch( (*p) ) {
		case 32: goto st134;
		case 43: goto st135;
		case 45: goto tr192;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr193;
	} else if ( (*p) >= 9 )
		goto st134;
	goto st0;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
	switch( (*p) ) {
		case 32: goto st135;
		case 43: goto st135;
		case 45: goto tr192;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr193;
	} else if ( (*p) >= 9 )
		goto st135;
	goto st0;
tr192:
#line 228 "command.rl"
	{ neg=-1;}
	goto st135;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
#line 1487 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr193;
	goto st0;
tr193:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st136;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
#line 1497 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr195;
		} else if ( (*p) >= 33 )
			goto st137;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st137;
		} else if ( (*p) >= 91 )
			goto st137;
	} else
		goto st137;
	goto st0;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr196;
	goto st0;
tr196:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st138;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
#line 1526 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr197;
	goto st0;
tr197:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 264 "command.rl"
	{deg=deg*3600+min*60;}
	goto st139;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
#line 1537 "command.cpp"
	if ( (*p) == 35 )
		goto tr198;
	goto st127;
tr198:
#line 261 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st312;
st312:
	if ( ++p == pe )
		goto _test_eof312;
case 312:
#line 1549 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr440;
		case 35: goto st1;
		case 58: goto st2;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr184;
	goto st0;
tr195:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st140;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
#line 1564 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr199;
		} else if ( (*p) >= 33 )
			goto st137;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st137;
		} else if ( (*p) >= 91 )
			goto st137;
	} else
		goto st137;
	goto st0;
tr199:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st141;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
#line 1586 "command.cpp"
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st137;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st137;
		} else if ( (*p) >= 91 )
			goto st137;
	} else
		goto st137;
	goto st0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr200;
	goto st0;
tr200:
#line 305 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st143;
tr436:
#line 321 "command.rl"
	{setflipmode((*p));}
	goto st143;
tr438:
#line 320 "command.rl"
	{setnunchuk((*p));}
	goto st143;
tr439:
#line 290 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st143;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
#line 1629 "command.cpp"
	if ( (*p) == 35 )
		goto st311;
	goto st0;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
	switch( (*p) ) {
		case 65: goto tr202;
		case 97: goto tr202;
		case 103: goto tr202;
		case 106: goto tr202;
		case 110: goto tr203;
		case 115: goto tr203;
		case 119: goto tr203;
		case 122: goto tr202;
	}
	goto st0;
tr202:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st145;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
#line 1654 "command.cpp"
	if ( (*p) == 35 )
		goto tr204;
	goto st0;
tr407:
#line 305 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st146;
tr203:
#line 267 "command.rl"
	{stcmd=(*p);mark=p;mark++;}
	goto st146;
tr421:
#line 321 "command.rl"
	{setflipmode((*p));}
	goto st146;
tr423:
#line 320 "command.rl"
	{setnunchuk((*p));}
	goto st146;
tr424:
#line 290 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st146;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
#line 1684 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	goto st146;
tr206:
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr225:
#line 293 "command.rl"
	{;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr226:
#line 291 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr227:
#line 292 "command.rl"
	{telescope->track=1;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr229:
#line 252 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr231:
#line 294 "command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr240:
#line 279 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr242:
#line 277 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr243:
#line 278 "command.rl"
	{gotofocuser(0,focuspeed_low);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr245:
#line 276 "command.rl"
	{gotofocuser(0,focuspeed);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr253:
#line 281 "command.rl"
	{gotofocuser(focus_counter);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr254:
#line 285 "command.rl"
	{sprintf(tmessage,"%d#",focus_motor.state<stop);APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr264:
#line 283 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr272:
#line 280 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr273:
#line 282 "command.rl"
	{stopfocuser();}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr274:
#line 284 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr294:
#line 242 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr295:
#line 245 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr296:
#line 239 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr297:
#line 272 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr298:
#line 297 "command.rl"
	{sprintf(tmessage,"%s#", telescope->is_tracking ? "1":"0");APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr299:
#line 259 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr300:
#line 246 "command.rl"
	{ lxprintsite();}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr301:
#line 237 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr302:
#line 260 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr303:
#line 275 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr309:
#line 298 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr310:
#line 302 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr311:
#line 299 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr312:
#line 300 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr313:
#line 301 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr314:
#line 241 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr315:
#line 237 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 239 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 241 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 242 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 297 "command.rl"
	{sprintf(tmessage,"%s#", telescope->is_tracking ? "1":"0");APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr316:
#line 274 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr317:
#line 244 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr318:
#line 248 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr319:
#line 296 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr320:
#line 243 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr321:
#line 249 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr327:
#line 303 "command.rl"
	{setwifipad(ip3,ip2);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr334:
#line 231 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr335:
#line 229 "command.rl"
	{mount_move(telescope,stcmd);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr341:
#line 230 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr342:
#line 235 "command.rl"
	{mount_stop(telescope,stcmd);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr345:
#line 236 "command.rl"
	{select_rate(telescope,stcmd); }
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr361:
#line 268 "command.rl"
	{set_date(min,deg,sec);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr366:
#line 270 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 247 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr379:
#line 273 "command.rl"
	{set_time(deg,min,sec);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr389:
#line 261 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr409:
#line 304 "command.rl"
	{appcmd(stcmd);APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr411:
#line 286 "command.rl"
	{mount_goto_home(telescope);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr418:
#line 289 "command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr419:
#line 287 "command.rl"
	{mount_home_set(telescope);}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
tr420:
#line 288 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st313;
st313:
	if ( ++p == pe )
		goto _test_eof313;
case 313:
#line 2195 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr442;
		case 35: goto tr206;
		case 58: goto st148;
	}
	goto st146;
tr442:
#line 269 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st147;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
#line 2208 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 58: goto st148;
	}
	goto st146;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
	switch( (*p) ) {
		case 35: goto tr206;
		case 65: goto st149;
		case 67: goto st153;
		case 68: goto st156;
		case 70: goto st157;
		case 71: goto st188;
		case 73: goto st213;
		case 77: goto st222;
		case 81: goto st231;
		case 82: goto st233;
		case 83: goto st236;
		case 97: goto st288;
		case 99: goto st289;
		case 104: goto st291;
		case 112: goto st293;
	}
	goto st146;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
	switch( (*p) ) {
		case 35: goto tr206;
		case 65: goto st150;
		case 76: goto st151;
		case 80: goto st152;
	}
	goto st146;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	if ( (*p) == 35 )
		goto tr225;
	goto st146;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	if ( (*p) == 35 )
		goto tr226;
	goto st146;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	if ( (*p) == 35 )
		goto tr227;
	goto st146;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
	switch( (*p) ) {
		case 35: goto tr206;
		case 77: goto st154;
	}
	goto st146;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	switch( (*p) ) {
		case 35: goto tr229;
		case 82: goto st155;
	}
	goto st146;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	if ( (*p) == 35 )
		goto tr229;
	goto st146;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	if ( (*p) == 35 )
		goto tr231;
	goto st146;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	switch( (*p) ) {
		case 35: goto tr206;
		case 43: goto st158;
		case 45: goto st160;
		case 65: goto st162;
		case 66: goto st169;
		case 76: goto st170;
		case 80: goto st179;
		case 81: goto st186;
		case 112: goto st187;
	}
	goto st146;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	switch( (*p) ) {
		case 35: goto tr240;
		case 43: goto st159;
	}
	goto st146;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	if ( (*p) == 35 )
		goto tr242;
	goto st146;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
	switch( (*p) ) {
		case 35: goto tr243;
		case 45: goto st161;
	}
	goto st146;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
	if ( (*p) == 35 )
		goto tr245;
	goto st146;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
	switch( (*p) ) {
		case 35: goto tr206;
		case 43: goto st163;
		case 45: goto tr247;
	}
	goto st146;
tr247:
#line 228 "command.rl"
	{ neg=-1;}
	goto st163;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
#line 2364 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr248;
	goto st146;
tr248:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st164;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
#line 2376 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr249;
	goto st146;
tr249:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st165;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
#line 2388 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr250;
	goto st146;
tr250:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st166;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
#line 2400 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr251;
	goto st146;
tr251:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st167;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
#line 2412 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr252;
	goto st146;
tr252:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st168;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
#line 2424 "command.cpp"
	if ( (*p) == 35 )
		goto tr253;
	goto st146;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
	if ( (*p) == 35 )
		goto tr254;
	goto st146;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
	switch( (*p) ) {
		case 35: goto tr206;
		case 83: goto st171;
	}
	goto st146;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
	switch( (*p) ) {
		case 35: goto tr206;
		case 49: goto st172;
	}
	goto st146;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
	switch( (*p) ) {
		case 35: goto tr206;
		case 43: goto st173;
		case 45: goto tr258;
	}
	goto st146;
tr258:
#line 228 "command.rl"
	{ neg=-1;}
	goto st173;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
#line 2469 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr259;
	goto st146;
tr259:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st174;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
#line 2481 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr260;
	goto st146;
tr260:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st175;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
#line 2493 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr261;
	goto st146;
tr261:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st176;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
#line 2505 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr262;
	goto st146;
tr262:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st177;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
#line 2517 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr263;
	goto st146;
tr263:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st178;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
#line 2529 "command.cpp"
	if ( (*p) == 35 )
		goto tr264;
	goto st146;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
	switch( (*p) ) {
		case 35: goto tr206;
		case 43: goto st180;
		case 45: goto tr266;
	}
	goto st146;
tr266:
#line 228 "command.rl"
	{ neg=-1;}
	goto st180;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
#line 2549 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr267;
	goto st146;
tr267:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st181;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
#line 2561 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr268;
	goto st146;
tr268:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st182;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
#line 2573 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr269;
	goto st146;
tr269:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st183;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
#line 2585 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr270;
	goto st146;
tr270:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st184;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
#line 2597 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr271;
	goto st146;
tr271:
#line 227 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st185;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
#line 2609 "command.cpp"
	if ( (*p) == 35 )
		goto tr272;
	goto st146;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
	if ( (*p) == 35 )
		goto tr273;
	goto st146;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
	if ( (*p) == 35 )
		goto tr274;
	goto st146;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
	switch( (*p) ) {
		case 35: goto tr206;
		case 65: goto st189;
		case 67: goto st190;
		case 68: goto st191;
		case 71: goto st192;
		case 75: goto st193;
		case 76: goto st194;
		case 77: goto st195;
		case 82: goto st196;
		case 83: goto st197;
		case 84: goto st198;
		case 86: goto st199;
		case 90: goto st205;
		case 97: goto st206;
		case 99: goto st207;
		case 100: goto st208;
		case 103: goto st209;
		case 107: goto st210;
		case 114: goto st211;
		case 116: goto st212;
	}
	goto st146;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
	if ( (*p) == 35 )
		goto tr294;
	goto st146;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
	if ( (*p) == 35 )
		goto tr295;
	goto st146;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
	if ( (*p) == 35 )
		goto tr296;
	goto st146;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
	if ( (*p) == 35 )
		goto tr297;
	goto st146;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	if ( (*p) == 35 )
		goto tr298;
	goto st146;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	if ( (*p) == 35 )
		goto tr299;
	goto st146;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	if ( (*p) == 35 )
		goto tr300;
	goto st146;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
	if ( (*p) == 35 )
		goto tr301;
	goto st146;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
	if ( (*p) == 35 )
		goto tr302;
	goto st146;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	if ( (*p) == 35 )
		goto tr303;
	goto st146;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	switch( (*p) ) {
		case 35: goto tr206;
		case 68: goto st200;
		case 70: goto st201;
		case 78: goto st202;
		case 80: goto st203;
		case 84: goto st204;
	}
	goto st146;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	if ( (*p) == 35 )
		goto tr309;
	goto st146;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	if ( (*p) == 35 )
		goto tr310;
	goto st146;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	if ( (*p) == 35 )
		goto tr311;
	goto st146;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
	if ( (*p) == 35 )
		goto tr312;
	goto st146;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
	if ( (*p) == 35 )
		goto tr313;
	goto st146;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
	if ( (*p) == 35 )
		goto tr314;
	goto st146;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
	if ( (*p) == 35 )
		goto tr315;
	goto st146;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	if ( (*p) == 35 )
		goto tr316;
	goto st146;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	if ( (*p) == 35 )
		goto tr317;
	goto st146;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
	if ( (*p) == 35 )
		goto tr318;
	goto st146;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
	if ( (*p) == 35 )
		goto tr319;
	goto st146;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	if ( (*p) == 35 )
		goto tr320;
	goto st146;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	if ( (*p) == 35 )
		goto tr321;
	goto st146;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
	switch( (*p) ) {
		case 35: goto tr206;
		case 80: goto st214;
	}
	goto st146;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr323;
	goto st146;
tr323:
#line 225 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st215;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
#line 2852 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 46: goto st216;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr325;
	goto st146;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr326;
	goto st146;
tr326:
#line 226 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st217;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
#line 2875 "command.cpp"
	if ( (*p) == 35 )
		goto tr327;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr328;
	goto st146;
tr328:
#line 226 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st218;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
#line 2887 "command.cpp"
	if ( (*p) == 35 )
		goto tr327;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr329;
	goto st146;
tr329:
#line 226 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st219;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
#line 2899 "command.cpp"
	if ( (*p) == 35 )
		goto tr327;
	goto st146;
tr325:
#line 225 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st220;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
#line 2909 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 46: goto st216;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr330;
	goto st146;
tr330:
#line 225 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st221;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
#line 2923 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 46: goto st216;
	}
	goto st146;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
	switch( (*p) ) {
		case 35: goto tr206;
		case 83: goto st223;
		case 101: goto tr332;
		case 103: goto st225;
		case 104: goto tr332;
		case 110: goto tr332;
		case 115: goto tr332;
		case 119: goto tr332;
	}
	goto st146;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
	if ( (*p) == 35 )
		goto tr334;
	goto st146;
tr332:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st224;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
#line 2957 "command.cpp"
	if ( (*p) == 35 )
		goto tr335;
	goto st146;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
	switch( (*p) ) {
		case 35: goto tr206;
		case 101: goto tr336;
		case 110: goto tr336;
		case 115: goto tr336;
		case 119: goto tr336;
	}
	goto st146;
tr336:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st226;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
#line 2979 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr337;
	goto st146;
tr337:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st227;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
#line 2991 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr338;
	goto st146;
tr338:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st228;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
#line 3003 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr339;
	goto st146;
tr339:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st229;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
#line 3015 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr340;
	goto st146;
tr340:
#line 224 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st230;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
#line 3027 "command.cpp"
	if ( (*p) == 35 )
		goto tr341;
	goto st146;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
	switch( (*p) ) {
		case 35: goto tr342;
		case 101: goto tr343;
		case 110: goto tr343;
		case 115: goto tr343;
		case 119: goto tr343;
	}
	goto st146;
tr343:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st232;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
#line 3049 "command.cpp"
	if ( (*p) == 35 )
		goto tr342;
	goto st146;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	switch( (*p) ) {
		case 35: goto tr206;
		case 67: goto tr344;
		case 71: goto tr344;
		case 77: goto tr344;
		case 83: goto tr344;
	}
	goto st146;
tr344:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st234;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
#line 3071 "command.cpp"
	if ( (*p) == 35 )
		goto tr345;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st235;
	goto st146;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
	if ( (*p) == 35 )
		goto tr345;
	goto st146;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
	switch( (*p) ) {
		case 35: goto tr206;
		case 67: goto st237;
		case 71: goto st247;
		case 76: goto st254;
		case 83: goto tr350;
		case 97: goto tr351;
		case 100: goto tr351;
		case 103: goto tr351;
		case 114: goto tr350;
		case 116: goto tr351;
		case 122: goto tr351;
	}
	goto st146;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
	switch( (*p) ) {
		case 32: goto st238;
		case 35: goto tr206;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr353;
	} else if ( (*p) >= 9 )
		goto st238;
	goto st146;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr353;
	goto st146;
tr353:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st239;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
#line 3131 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr354;
	goto st146;
tr354:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st240;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
#line 3143 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 47: goto st241;
	}
	goto st146;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr356;
	goto st146;
tr356:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st242;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
#line 3164 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr357;
	goto st146;
tr357:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st243;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
#line 3176 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 47: goto st244;
	}
	goto st146;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr359;
	goto st146;
tr359:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st245;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
#line 3197 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr360;
	goto st146;
tr360:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st246;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
#line 3209 "command.cpp"
	if ( (*p) == 35 )
		goto tr361;
	goto st146;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
	switch( (*p) ) {
		case 32: goto st248;
		case 35: goto tr206;
		case 43: goto st249;
		case 45: goto tr364;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st248;
	goto st146;
st248:
	if ( ++p == pe )
		goto _test_eof248;
case 248:
	switch( (*p) ) {
		case 35: goto tr206;
		case 43: goto st249;
		case 45: goto tr364;
	}
	goto st146;
tr364:
#line 228 "command.rl"
	{ neg=-1;}
	goto st249;
st249:
	if ( ++p == pe )
		goto _test_eof249;
case 249:
#line 3242 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr365;
	goto st146;
tr365:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st250;
st250:
	if ( ++p == pe )
		goto _test_eof250;
case 250:
#line 3254 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr366;
		case 44: goto st251;
		case 46: goto st251;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr368;
	goto st146;
st251:
	if ( ++p == pe )
		goto _test_eof251;
case 251:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st252;
	goto st146;
st252:
	if ( ++p == pe )
		goto _test_eof252;
case 252:
	if ( (*p) == 35 )
		goto tr366;
	goto st146;
tr368:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st253;
st253:
	if ( ++p == pe )
		goto _test_eof253;
case 253:
#line 3285 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr366;
		case 44: goto st251;
		case 46: goto st251;
	}
	goto st146;
st254:
	if ( ++p == pe )
		goto _test_eof254;
case 254:
	switch( (*p) ) {
		case 32: goto st255;
		case 35: goto tr206;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr371;
	} else if ( (*p) >= 9 )
		goto st255;
	goto st146;
st255:
	if ( ++p == pe )
		goto _test_eof255;
case 255:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr371;
	goto st146;
tr371:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st256;
st256:
	if ( ++p == pe )
		goto _test_eof256;
case 256:
#line 3321 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr372;
	goto st146;
tr372:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st257;
st257:
	if ( ++p == pe )
		goto _test_eof257;
case 257:
#line 3333 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 58: goto st258;
	}
	goto st146;
st258:
	if ( ++p == pe )
		goto _test_eof258;
case 258:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr374;
	goto st146;
tr374:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st259;
st259:
	if ( ++p == pe )
		goto _test_eof259;
case 259:
#line 3354 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr375;
	goto st146;
tr375:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st260;
st260:
	if ( ++p == pe )
		goto _test_eof260;
case 260:
#line 3366 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 58: goto st261;
	}
	goto st146;
st261:
	if ( ++p == pe )
		goto _test_eof261;
case 261:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr377;
	goto st146;
tr377:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st262;
st262:
	if ( ++p == pe )
		goto _test_eof262;
case 262:
#line 3387 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr378;
	goto st146;
tr378:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st263;
st263:
	if ( ++p == pe )
		goto _test_eof263;
case 263:
#line 3399 "command.cpp"
	if ( (*p) == 35 )
		goto tr379;
	goto st146;
tr350:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st264;
st264:
	if ( ++p == pe )
		goto _test_eof264;
case 264:
#line 3409 "command.cpp"
	switch( (*p) ) {
		case 32: goto st265;
		case 35: goto tr206;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr381;
	} else if ( (*p) >= 9 )
		goto st265;
	goto st146;
st265:
	if ( ++p == pe )
		goto _test_eof265;
case 265:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr381;
	goto st146;
tr381:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st266;
st266:
	if ( ++p == pe )
		goto _test_eof266;
case 266:
#line 3435 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr382;
	goto st146;
tr382:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st267;
st267:
	if ( ++p == pe )
		goto _test_eof267;
case 267:
#line 3447 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 47: goto st268;
		case 58: goto st268;
	}
	goto st146;
st268:
	if ( ++p == pe )
		goto _test_eof268;
case 268:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr384;
	goto st146;
tr384:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st269;
st269:
	if ( ++p == pe )
		goto _test_eof269;
case 269:
#line 3469 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr385;
	goto st146;
tr385:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 264 "command.rl"
	{deg=deg*3600+min*60;}
	goto st270;
st270:
	if ( ++p == pe )
		goto _test_eof270;
case 270:
#line 3482 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr206;
		case 46: goto st271;
		case 47: goto st273;
		case 58: goto st273;
	}
	goto st146;
st271:
	if ( ++p == pe )
		goto _test_eof271;
case 271:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr388;
	goto st146;
tr388:
#line 258 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st272;
st272:
	if ( ++p == pe )
		goto _test_eof272;
case 272:
#line 3505 "command.cpp"
	if ( (*p) == 35 )
		goto tr389;
	goto st146;
st273:
	if ( ++p == pe )
		goto _test_eof273;
case 273:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr390;
	goto st146;
tr390:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st274;
st274:
	if ( ++p == pe )
		goto _test_eof274;
case 274:
#line 3524 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr391;
	goto st146;
tr391:
#line 223 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 265 "command.rl"
	{deg+=sec;}
	goto st275;
st275:
	if ( ++p == pe )
		goto _test_eof275;
case 275:
#line 3537 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr389;
		case 44: goto st276;
		case 46: goto st276;
		case 58: goto st272;
	}
	goto st146;
st276:
	if ( ++p == pe )
		goto _test_eof276;
case 276:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st277;
	goto st146;
st277:
	if ( ++p == pe )
		goto _test_eof277;
case 277:
	switch( (*p) ) {
		case 35: goto tr389;
		case 58: goto st272;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st278;
	goto st146;
st278:
	if ( ++p == pe )
		goto _test_eof278;
case 278:
	switch( (*p) ) {
		case 35: goto tr389;
		case 58: goto st272;
	}
	goto st146;
tr351:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st279;
st279:
	if ( ++p == pe )
		goto _test_eof279;
case 279:
#line 3580 "command.cpp"
	switch( (*p) ) {
		case 32: goto st280;
		case 35: goto tr206;
		case 43: goto st281;
		case 45: goto tr398;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr399;
	} else if ( (*p) >= 9 )
		goto st280;
	goto st146;
st280:
	if ( ++p == pe )
		goto _test_eof280;
case 280:
	switch( (*p) ) {
		case 32: goto st281;
		case 35: goto tr206;
		case 43: goto st281;
		case 45: goto tr398;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr399;
	} else if ( (*p) >= 9 )
		goto st281;
	goto st146;
tr398:
#line 228 "command.rl"
	{ neg=-1;}
	goto st281;
st281:
	if ( ++p == pe )
		goto _test_eof281;
case 281:
#line 3615 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr399;
	goto st146;
tr399:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st282;
st282:
	if ( ++p == pe )
		goto _test_eof282;
case 282:
#line 3627 "command.cpp"
	if ( (*p) == 35 )
		goto tr401;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr402;
		} else if ( (*p) >= 33 )
			goto st283;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st283;
		} else if ( (*p) >= 91 )
			goto st283;
	} else
		goto st283;
	goto st146;
st283:
	if ( ++p == pe )
		goto _test_eof283;
case 283:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr403;
	goto st146;
tr403:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st284;
st284:
	if ( ++p == pe )
		goto _test_eof284;
case 284:
#line 3660 "command.cpp"
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr404;
	goto st146;
tr404:
#line 222 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 264 "command.rl"
	{deg=deg*3600+min*60;}
	goto st285;
st285:
	if ( ++p == pe )
		goto _test_eof285;
case 285:
#line 3673 "command.cpp"
	if ( (*p) == 35 )
		goto tr405;
	goto st273;
tr405:
#line 261 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st314;
st314:
	if ( ++p == pe )
		goto _test_eof314;
case 314:
#line 3691 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr442;
		case 35: goto tr206;
		case 58: goto st148;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr390;
	goto st146;
tr401:
#line 314 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st315;
st315:
	if ( ++p == pe )
		goto _test_eof315;
case 315:
#line 3711 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr442;
		case 35: goto tr206;
		case 58: goto st148;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr403;
	goto st146;
tr402:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st286;
st286:
	if ( ++p == pe )
		goto _test_eof286;
case 286:
#line 3726 "command.cpp"
	if ( (*p) == 35 )
		goto tr401;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr406;
		} else if ( (*p) >= 33 )
			goto st283;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st283;
		} else if ( (*p) >= 91 )
			goto st283;
	} else
		goto st283;
	goto st146;
tr406:
#line 221 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st287;
st287:
	if ( ++p == pe )
		goto _test_eof287;
case 287:
#line 3750 "command.cpp"
	if ( (*p) == 35 )
		goto tr401;
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st283;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st283;
		} else if ( (*p) >= 91 )
			goto st283;
	} else
		goto st283;
	goto st146;
st288:
	if ( ++p == pe )
		goto _test_eof288;
case 288:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr407;
	goto st146;
st289:
	if ( ++p == pe )
		goto _test_eof289;
case 289:
	switch( (*p) ) {
		case 35: goto tr206;
		case 65: goto tr408;
		case 97: goto tr408;
		case 103: goto tr408;
		case 106: goto tr408;
		case 110: goto tr203;
		case 115: goto tr203;
		case 119: goto tr203;
		case 122: goto tr408;
	}
	goto st146;
tr408:
#line 266 "command.rl"
	{stcmd=(*p);}
	goto st290;
st290:
	if ( ++p == pe )
		goto _test_eof290;
case 290:
#line 3796 "command.cpp"
	if ( (*p) == 35 )
		goto tr409;
	goto st146;
st291:
	if ( ++p == pe )
		goto _test_eof291;
case 291:
	switch( (*p) ) {
		case 35: goto tr206;
		case 80: goto st292;
	}
	goto st146;
st292:
	if ( ++p == pe )
		goto _test_eof292;
case 292:
	if ( (*p) == 35 )
		goto tr411;
	goto st146;
st293:
	if ( ++p == pe )
		goto _test_eof293;
case 293:
	switch( (*p) ) {
		case 35: goto tr206;
		case 70: goto st294;
		case 72: goto st295;
		case 83: goto st296;
		case 97: goto st297;
		case 110: goto st298;
		case 115: goto st300;
	}
	goto st146;
st294:
	if ( ++p == pe )
		goto _test_eof294;
case 294:
	if ( (*p) == 35 )
		goto tr418;
	goto st146;
st295:
	if ( ++p == pe )
		goto _test_eof295;
case 295:
	if ( (*p) == 35 )
		goto tr419;
	goto st146;
st296:
	if ( ++p == pe )
		goto _test_eof296;
case 296:
	if ( (*p) == 35 )
		goto tr420;
	goto st146;
st297:
	if ( ++p == pe )
		goto _test_eof297;
case 297:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr421;
	goto st146;
st298:
	if ( ++p == pe )
		goto _test_eof298;
case 298:
	switch( (*p) ) {
		case 35: goto tr206;
		case 107: goto st299;
	}
	goto st146;
st299:
	if ( ++p == pe )
		goto _test_eof299;
case 299:
	if ( (*p) == 35 )
		goto tr206;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr423;
	goto st146;
st300:
	if ( ++p == pe )
		goto _test_eof300;
case 300:
	switch( (*p) ) {
		case 35: goto tr206;
		case 101: goto tr424;
		case 119: goto tr424;
	}
	goto st146;
st301:
	if ( ++p == pe )
		goto _test_eof301;
case 301:
	if ( (*p) == 80 )
		goto st302;
	goto st0;
st302:
	if ( ++p == pe )
		goto _test_eof302;
case 302:
	if ( (*p) == 35 )
		goto tr426;
	goto st0;
st303:
	if ( ++p == pe )
		goto _test_eof303;
case 303:
	switch( (*p) ) {
		case 70: goto st304;
		case 72: goto st305;
		case 83: goto st306;
		case 97: goto st307;
		case 110: goto st308;
		case 115: goto st310;
	}
	goto st0;
st304:
	if ( ++p == pe )
		goto _test_eof304;
case 304:
	if ( (*p) == 35 )
		goto tr433;
	goto st0;
st305:
	if ( ++p == pe )
		goto _test_eof305;
case 305:
	if ( (*p) == 35 )
		goto tr434;
	goto st0;
st306:
	if ( ++p == pe )
		goto _test_eof306;
case 306:
	if ( (*p) == 35 )
		goto tr435;
	goto st0;
st307:
	if ( ++p == pe )
		goto _test_eof307;
case 307:
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr436;
	goto st0;
st308:
	if ( ++p == pe )
		goto _test_eof308;
case 308:
	if ( (*p) == 107 )
		goto st309;
	goto st0;
st309:
	if ( ++p == pe )
		goto _test_eof309;
case 309:
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr438;
	goto st0;
st310:
	if ( ++p == pe )
		goto _test_eof310;
case 310:
	switch( (*p) ) {
		case 101: goto tr439;
		case 119: goto tr439;
	}
	goto st0;
	}
	_test_eof311: cs = 311; goto _test_eof; 
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
	_test_eof312: cs = 312; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof313: cs = 313; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
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
	_test_eof314: cs = 314; goto _test_eof; 
	_test_eof315: cs = 315; goto _test_eof; 
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

	_test_eof: {}
	_out: {}
	}

#line 404 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
		return 0;
};



