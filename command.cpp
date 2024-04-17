
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
extern int  focusmax,focuspeed;
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
	
	sprintf(tmessage,"%d\r\n%d\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.2f\r\n%.0f\r\n%.0f\r\n%.0f\r\n%.4f\r\n%.6f\r\n%.6f\r\n%d\r\n%d\r\n%d\r\n%d\r\n%.0f\r\n%.0f\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
          telescope->azmotor->maxcounter, telescope->altmotor->maxcounter,
          telescope->rate[0][0], telescope->rate[1][0], telescope->rate[2][0], telescope->rate[3][0],
          telescope->rate[0][1], telescope->rate[1][1], telescope->rate[2][1], telescope->rate[3][1],
		  telescope->prescaler,
          telescope->longitude, telescope->lat, telescope->time_zone,
		  focusmax, focuspeed_low, focuspeed,
		 telescope->azmotor->acceleration / SEC_TO_RAD, telescope->altmotor->acceleration / SEC_TO_RAD,
		 telescope->azmotor->backlash, telescope->altmotor->backlash,
		  telescope->mount_mode ,telescope->track, telescope->autoflip, telescope->azmotor->cw,
		  telescope->altmotor->cw, focusvolt * focusinv,  telescope->azmotor->active, telescope->altmotor->active
		 );
			 
		
          
         	 
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
    sprintf(tmessage,"1");APPEND;
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
static const int command_start = 303;
static const int command_first_final = 303;
static const int command_error = 0;

static const int command_en_main = 303;


#line 208 "command.rl"





    
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
#line 285 "command.rl"
	{;}
	goto st303;
tr20:
#line 283 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	goto st303;
tr21:
#line 284 "command.rl"
	{telescope->track=1;}
	goto st303;
tr23:
#line 246 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	goto st303;
tr25:
#line 286 "command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	goto st303;
tr34:
#line 272 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
	goto st303;
tr36:
#line 270 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
	goto st303;
tr37:
#line 271 "command.rl"
	{gotofocuser(0,focuspeed_low);}
	goto st303;
tr39:
#line 269 "command.rl"
	{gotofocuser(0,focuspeed);}
	goto st303;
tr47:
#line 274 "command.rl"
	{gotofocuser(focus_counter);}
	goto st303;
tr48:
#line 278 "command.rl"
	{sprintf(tmessage,"%d#",focus_motor.state<stop);APPEND;}
	goto st303;
tr58:
#line 276 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
	goto st303;
tr66:
#line 273 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
	goto st303;
tr67:
#line 275 "command.rl"
	{stopfocuser();}
	goto st303;
tr68:
#line 277 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st303;
tr86:
#line 236 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	goto st303;
tr87:
#line 239 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	goto st303;
tr88:
#line 233 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	goto st303;
tr89:
#line 265 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	goto st303;
tr90:
#line 253 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	goto st303;
tr91:
#line 240 "command.rl"
	{ lxprintsite();}
	goto st303;
tr92:
#line 231 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	goto st303;
tr93:
#line 254 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
	goto st303;
tr94:
#line 268 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	goto st303;
tr100:
#line 289 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
	goto st303;
tr101:
#line 293 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	goto st303;
tr102:
#line 290 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
	goto st303;
tr103:
#line 291 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	goto st303;
tr104:
#line 292 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
	goto st303;
tr105:
#line 235 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	goto st303;
tr106:
#line 267 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	goto st303;
tr107:
#line 238 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	goto st303;
tr108:
#line 242 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	goto st303;
tr109:
#line 288 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
	goto st303;
tr110:
#line 237 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	goto st303;
tr111:
#line 243 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	goto st303;
tr117:
#line 294 "command.rl"
	{setwifipad(ip3,ip2);}
	goto st303;
tr124:
#line 225 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
	goto st303;
tr125:
#line 223 "command.rl"
	{mount_move(telescope,stcmd);}
	goto st303;
tr131:
#line 224 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	goto st303;
tr132:
#line 229 "command.rl"
	{mount_stop(telescope,stcmd);}
	goto st303;
tr135:
#line 230 "command.rl"
	{select_rate(telescope,stcmd); }
	goto st303;
tr151:
#line 262 "command.rl"
	{set_date(min,deg,sec);}
	goto st303;
tr156:
#line 264 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 241 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	goto st303;
tr169:
#line 266 "command.rl"
	{set_time(deg,min,sec);}
	goto st303;
tr179:
#line 255 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st303;
tr200:
#line 295 "command.rl"
	{appcmd(stcmd);APPEND;}
	goto st303;
tr414:
#line 279 "command.rl"
	{mount_goto_home(telescope);}
	goto st303;
tr419:
#line 280 "command.rl"
	{mount_home_set(telescope);}
	goto st303;
tr420:
#line 281 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
	goto st303;
st303:
	if ( ++p == pe )
		goto _test_eof303;
case 303:
#line 183 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr424;
		case 35: goto st1;
		case 58: goto st2;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr424:
#line 263 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 199 "command.cpp"
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
		case 73: goto st65;
		case 77: goto st74;
		case 81: goto st83;
		case 82: goto st85;
		case 83: goto st88;
		case 97: goto st140;
		case 99: goto st142;
		case 104: goto st295;
		case 112: goto st297;
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
#line 222 "command.rl"
	{ neg=-1;}
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 347 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr42;
	goto st0;
tr42:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 357 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr43;
	goto st0;
tr43:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 367 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr44;
	goto st0;
tr44:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 377 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr45;
	goto st0;
tr45:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 387 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr46;
	goto st0;
tr46:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 397 "command.cpp"
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
#line 222 "command.rl"
	{ neg=-1;}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 437 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr53;
	goto st0;
tr53:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 447 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr54;
	goto st0;
tr54:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 457 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr55;
	goto st0;
tr55:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 467 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr56;
	goto st0;
tr56:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 477 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr57;
	goto st0;
tr57:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 487 "command.cpp"
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
#line 222 "command.rl"
	{ neg=-1;}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 506 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr61;
	goto st0;
tr61:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 516 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr62;
	goto st0;
tr62:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 526 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr63;
	goto st0;
tr63:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 536 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr64;
	goto st0;
tr64:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 546 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr65;
	goto st0;
tr65:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 556 "command.cpp"
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
		case 76: goto st47;
		case 77: goto st48;
		case 82: goto st49;
		case 83: goto st50;
		case 84: goto st51;
		case 86: goto st52;
		case 90: goto st58;
		case 99: goto st59;
		case 100: goto st60;
		case 103: goto st61;
		case 107: goto st62;
		case 114: goto st63;
		case 116: goto st64;
	}
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 35 )
		goto tr86;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 35 )
		goto tr87;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 35 )
		goto tr88;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 35 )
		goto tr89;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 35 )
		goto tr90;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 35 )
		goto tr91;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 35 )
		goto tr92;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 35 )
		goto tr93;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 35 )
		goto tr94;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 68: goto st53;
		case 70: goto st54;
		case 78: goto st55;
		case 80: goto st56;
		case 84: goto st57;
	}
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) == 35 )
		goto tr100;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 35 )
		goto tr101;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 35 )
		goto tr102;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 35 )
		goto tr103;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 35 )
		goto tr104;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 35 )
		goto tr105;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 35 )
		goto tr106;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 35 )
		goto tr107;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 35 )
		goto tr108;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 35 )
		goto tr109;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 35 )
		goto tr110;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 35 )
		goto tr111;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 80 )
		goto st66;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr113;
	goto st0;
tr113:
#line 219 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 777 "command.cpp"
	if ( (*p) == 46 )
		goto st68;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr115;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr116;
	goto st0;
tr116:
#line 220 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 796 "command.cpp"
	if ( (*p) == 35 )
		goto tr117;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr118;
	goto st0;
tr118:
#line 220 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 808 "command.cpp"
	if ( (*p) == 35 )
		goto tr117;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr119;
	goto st0;
tr119:
#line 220 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 820 "command.cpp"
	if ( (*p) == 35 )
		goto tr117;
	goto st0;
tr115:
#line 219 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 830 "command.cpp"
	if ( (*p) == 46 )
		goto st68;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr120;
	goto st0;
tr120:
#line 219 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 842 "command.cpp"
	if ( (*p) == 46 )
		goto st68;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	switch( (*p) ) {
		case 83: goto st75;
		case 101: goto tr122;
		case 103: goto st77;
		case 104: goto tr122;
		case 110: goto tr122;
		case 115: goto tr122;
		case 119: goto tr122;
	}
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( (*p) == 35 )
		goto tr124;
	goto st0;
tr122:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st76;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
#line 873 "command.cpp"
	if ( (*p) == 35 )
		goto tr125;
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	switch( (*p) ) {
		case 101: goto tr126;
		case 110: goto tr126;
		case 115: goto tr126;
		case 119: goto tr126;
	}
	goto st0;
tr126:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
#line 894 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr127;
	goto st0;
tr127:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st79;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
#line 904 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr128;
	goto st0;
tr128:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 914 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr129;
	goto st0;
tr129:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
#line 924 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr130;
	goto st0;
tr130:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st82;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
#line 934 "command.cpp"
	if ( (*p) == 35 )
		goto tr131;
	goto st0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	switch( (*p) ) {
		case 35: goto tr132;
		case 101: goto tr133;
		case 110: goto tr133;
		case 115: goto tr133;
		case 119: goto tr133;
	}
	goto st0;
tr133:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 956 "command.cpp"
	if ( (*p) == 35 )
		goto tr132;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 67: goto tr134;
		case 71: goto tr134;
		case 77: goto tr134;
		case 83: goto tr134;
	}
	goto st0;
tr134:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
#line 977 "command.cpp"
	if ( (*p) == 35 )
		goto tr135;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st87;
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	if ( (*p) == 35 )
		goto tr135;
	goto st0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	switch( (*p) ) {
		case 67: goto st89;
		case 71: goto st99;
		case 76: goto st106;
		case 83: goto tr140;
		case 97: goto tr141;
		case 100: goto tr141;
		case 103: goto tr141;
		case 114: goto tr140;
		case 116: goto tr141;
		case 122: goto tr141;
	}
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( (*p) == 32 )
		goto st90;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr143;
	} else if ( (*p) >= 9 )
		goto st90;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr143;
	goto st0;
tr143:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st91;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
#line 1032 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr144;
	goto st0;
tr144:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st92;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
#line 1042 "command.cpp"
	if ( (*p) == 47 )
		goto st93;
	goto st0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr146;
	goto st0;
tr146:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st94;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
#line 1059 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr147;
	goto st0;
tr147:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st95;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
#line 1069 "command.cpp"
	if ( (*p) == 47 )
		goto st96;
	goto st0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr149;
	goto st0;
tr149:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st97;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
#line 1086 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr150;
	goto st0;
tr150:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st98;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
#line 1096 "command.cpp"
	if ( (*p) == 35 )
		goto tr151;
	goto st0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	switch( (*p) ) {
		case 32: goto st100;
		case 43: goto st101;
		case 45: goto tr154;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st100;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	switch( (*p) ) {
		case 43: goto st101;
		case 45: goto tr154;
	}
	goto st0;
tr154:
#line 222 "command.rl"
	{ neg=-1;}
	goto st101;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
#line 1127 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr155;
	goto st0;
tr155:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
#line 1137 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr156;
		case 44: goto st103;
		case 46: goto st103;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr158;
	goto st0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st104;
	goto st0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	if ( (*p) == 35 )
		goto tr156;
	goto st0;
tr158:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
#line 1166 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr156;
		case 44: goto st103;
		case 46: goto st103;
	}
	goto st0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	if ( (*p) == 32 )
		goto st107;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr161;
	} else if ( (*p) >= 9 )
		goto st107;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr161;
	goto st0;
tr161:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st108;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
#line 1198 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr162;
	goto st0;
tr162:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 1208 "command.cpp"
	if ( (*p) == 58 )
		goto st110;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr164;
	goto st0;
tr164:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st111;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
#line 1225 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr165;
	goto st0;
tr165:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1235 "command.cpp"
	if ( (*p) == 58 )
		goto st113;
	goto st0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr167;
	goto st0;
tr167:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st114;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
#line 1252 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr168;
	goto st0;
tr168:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st115;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
#line 1262 "command.cpp"
	if ( (*p) == 35 )
		goto tr169;
	goto st0;
tr140:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st116;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
#line 1272 "command.cpp"
	if ( (*p) == 32 )
		goto st117;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr171;
	} else if ( (*p) >= 9 )
		goto st117;
	goto st0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr171;
	goto st0;
tr171:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
#line 1294 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr172;
	goto st0;
tr172:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st119;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
#line 1304 "command.cpp"
	switch( (*p) ) {
		case 47: goto st120;
		case 58: goto st120;
	}
	goto st0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr174;
	goto st0;
tr174:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st121;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
#line 1323 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr175;
	goto st0;
tr175:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 258 "command.rl"
	{deg=deg*3600+min*60;}
	goto st122;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
#line 1334 "command.cpp"
	switch( (*p) ) {
		case 46: goto st123;
		case 47: goto st125;
		case 58: goto st125;
	}
	goto st0;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr178;
	goto st0;
tr178:
#line 252 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st124;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
#line 1354 "command.cpp"
	if ( (*p) == 35 )
		goto tr179;
	goto st0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr180;
	goto st0;
tr180:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st126;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
#line 1371 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr181;
	goto st0;
tr181:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 259 "command.rl"
	{deg+=sec;}
	goto st127;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
#line 1382 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr179;
		case 44: goto st128;
		case 46: goto st128;
		case 58: goto st124;
	}
	goto st0;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st129;
	goto st0;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
	switch( (*p) ) {
		case 35: goto tr179;
		case 58: goto st124;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st130;
	goto st0;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	switch( (*p) ) {
		case 35: goto tr179;
		case 58: goto st124;
	}
	goto st0;
tr141:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st131;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
#line 1423 "command.cpp"
	switch( (*p) ) {
		case 32: goto st132;
		case 43: goto st133;
		case 45: goto tr188;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr189;
	} else if ( (*p) >= 9 )
		goto st132;
	goto st0;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	switch( (*p) ) {
		case 32: goto st133;
		case 43: goto st133;
		case 45: goto tr188;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr189;
	} else if ( (*p) >= 9 )
		goto st133;
	goto st0;
tr188:
#line 222 "command.rl"
	{ neg=-1;}
	goto st133;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
#line 1456 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr189;
	goto st0;
tr189:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st134;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
#line 1466 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr191;
		} else if ( (*p) >= 33 )
			goto st135;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st135;
		} else if ( (*p) >= 91 )
			goto st135;
	} else
		goto st135;
	goto st0;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr192;
	goto st0;
tr192:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st136;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
#line 1495 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr193;
	goto st0;
tr193:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 258 "command.rl"
	{deg=deg*3600+min*60;}
	goto st137;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
#line 1506 "command.cpp"
	if ( (*p) == 35 )
		goto tr194;
	goto st125;
tr194:
#line 255 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st304;
st304:
	if ( ++p == pe )
		goto _test_eof304;
case 304:
#line 1518 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr424;
		case 35: goto st1;
		case 58: goto st2;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr180;
	goto st0;
tr191:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st138;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
#line 1533 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr195;
		} else if ( (*p) >= 33 )
			goto st135;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st135;
		} else if ( (*p) >= 91 )
			goto st135;
	} else
		goto st135;
	goto st0;
tr195:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st139;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
#line 1555 "command.cpp"
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st135;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st135;
		} else if ( (*p) >= 91 )
			goto st135;
	} else
		goto st135;
	goto st0;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr196;
	goto st0;
tr196:
#line 296 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st141;
tr422:
#line 311 "command.rl"
	{setnunchuk((*p));}
	goto st141;
tr423:
#line 282 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st141;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
#line 1595 "command.cpp"
	if ( (*p) == 35 )
		goto st303;
	goto st0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	switch( (*p) ) {
		case 65: goto tr198;
		case 97: goto tr198;
		case 103: goto tr198;
		case 106: goto tr198;
		case 110: goto tr199;
		case 115: goto tr199;
		case 119: goto tr199;
		case 122: goto tr198;
	}
	goto st0;
tr198:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st143;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
#line 1620 "command.cpp"
	if ( (*p) == 35 )
		goto tr200;
	goto st0;
tr399:
#line 296 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	goto st144;
tr199:
#line 261 "command.rl"
	{stcmd=(*p);mark=p;mark++;}
	goto st144;
tr411:
#line 311 "command.rl"
	{setnunchuk((*p));}
	goto st144;
tr412:
#line 282 "command.rl"
	{meridianflip(telescope,(*p)=='w');}
	goto st144;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
#line 1647 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	goto st144;
tr202:
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr221:
#line 285 "command.rl"
	{;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr222:
#line 283 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr223:
#line 284 "command.rl"
	{telescope->track=1;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr225:
#line 246 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr227:
#line 286 "command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr236:
#line 272 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr238:
#line 270 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr239:
#line 271 "command.rl"
	{gotofocuser(0,focuspeed_low);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr241:
#line 269 "command.rl"
	{gotofocuser(0,focuspeed);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr249:
#line 274 "command.rl"
	{gotofocuser(focus_counter);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr250:
#line 278 "command.rl"
	{sprintf(tmessage,"%d#",focus_motor.state<stop);APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr260:
#line 276 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr268:
#line 273 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr269:
#line 275 "command.rl"
	{stopfocuser();}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr270:
#line 277 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr288:
#line 236 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr289:
#line 239 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr290:
#line 233 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr291:
#line 265 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr292:
#line 253 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr293:
#line 240 "command.rl"
	{ lxprintsite();}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr294:
#line 231 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr295:
#line 254 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr296:
#line 268 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr302:
#line 289 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr303:
#line 293 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr304:
#line 290 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr305:
#line 291 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr306:
#line 292 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr307:
#line 235 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr308:
#line 267 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr309:
#line 238 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr310:
#line 242 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr311:
#line 288 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr312:
#line 237 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr313:
#line 243 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr319:
#line 294 "command.rl"
	{setwifipad(ip3,ip2);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr326:
#line 225 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr327:
#line 223 "command.rl"
	{mount_move(telescope,stcmd);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr333:
#line 224 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr334:
#line 229 "command.rl"
	{mount_stop(telescope,stcmd);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr337:
#line 230 "command.rl"
	{select_rate(telescope,stcmd); }
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr353:
#line 262 "command.rl"
	{set_date(min,deg,sec);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr358:
#line 264 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 241 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr371:
#line 266 "command.rl"
	{set_time(deg,min,sec);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr381:
#line 255 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr401:
#line 295 "command.rl"
	{appcmd(stcmd);APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr403:
#line 279 "command.rl"
	{mount_goto_home(telescope);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr408:
#line 280 "command.rl"
	{mount_home_set(telescope);}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
tr409:
#line 281 "command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st305;
st305:
	if ( ++p == pe )
		goto _test_eof305;
case 305:
#line 2125 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr426;
		case 35: goto tr202;
		case 58: goto st146;
	}
	goto st144;
tr426:
#line 263 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st145;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
#line 2138 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 58: goto st146;
	}
	goto st144;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	switch( (*p) ) {
		case 35: goto tr202;
		case 65: goto st147;
		case 67: goto st151;
		case 68: goto st154;
		case 70: goto st155;
		case 71: goto st186;
		case 73: goto st209;
		case 77: goto st218;
		case 81: goto st227;
		case 82: goto st229;
		case 83: goto st232;
		case 97: goto st284;
		case 99: goto st285;
		case 104: goto st287;
		case 112: goto st289;
	}
	goto st144;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
	switch( (*p) ) {
		case 35: goto tr202;
		case 65: goto st148;
		case 76: goto st149;
		case 80: goto st150;
	}
	goto st144;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
	if ( (*p) == 35 )
		goto tr221;
	goto st144;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
	if ( (*p) == 35 )
		goto tr222;
	goto st144;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	if ( (*p) == 35 )
		goto tr223;
	goto st144;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	switch( (*p) ) {
		case 35: goto tr202;
		case 77: goto st152;
	}
	goto st144;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	switch( (*p) ) {
		case 35: goto tr225;
		case 82: goto st153;
	}
	goto st144;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
	if ( (*p) == 35 )
		goto tr225;
	goto st144;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	if ( (*p) == 35 )
		goto tr227;
	goto st144;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	switch( (*p) ) {
		case 35: goto tr202;
		case 43: goto st156;
		case 45: goto st158;
		case 65: goto st160;
		case 66: goto st167;
		case 76: goto st168;
		case 80: goto st177;
		case 81: goto st184;
		case 112: goto st185;
	}
	goto st144;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	switch( (*p) ) {
		case 35: goto tr236;
		case 43: goto st157;
	}
	goto st144;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	if ( (*p) == 35 )
		goto tr238;
	goto st144;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	switch( (*p) ) {
		case 35: goto tr239;
		case 45: goto st159;
	}
	goto st144;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	if ( (*p) == 35 )
		goto tr241;
	goto st144;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
	switch( (*p) ) {
		case 35: goto tr202;
		case 43: goto st161;
		case 45: goto tr243;
	}
	goto st144;
tr243:
#line 222 "command.rl"
	{ neg=-1;}
	goto st161;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
#line 2294 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr244;
	goto st144;
tr244:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st162;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
#line 2306 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr245;
	goto st144;
tr245:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st163;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
#line 2318 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr246;
	goto st144;
tr246:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st164;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
#line 2330 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr247;
	goto st144;
tr247:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st165;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
#line 2342 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr248;
	goto st144;
tr248:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st166;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
#line 2354 "command.cpp"
	if ( (*p) == 35 )
		goto tr249;
	goto st144;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
	if ( (*p) == 35 )
		goto tr250;
	goto st144;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
	switch( (*p) ) {
		case 35: goto tr202;
		case 83: goto st169;
	}
	goto st144;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
	switch( (*p) ) {
		case 35: goto tr202;
		case 49: goto st170;
	}
	goto st144;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
	switch( (*p) ) {
		case 35: goto tr202;
		case 43: goto st171;
		case 45: goto tr254;
	}
	goto st144;
tr254:
#line 222 "command.rl"
	{ neg=-1;}
	goto st171;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
#line 2399 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr255;
	goto st144;
tr255:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st172;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
#line 2411 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr256;
	goto st144;
tr256:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st173;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
#line 2423 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr257;
	goto st144;
tr257:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st174;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
#line 2435 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr258;
	goto st144;
tr258:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st175;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
#line 2447 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr259;
	goto st144;
tr259:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st176;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
#line 2459 "command.cpp"
	if ( (*p) == 35 )
		goto tr260;
	goto st144;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
	switch( (*p) ) {
		case 35: goto tr202;
		case 43: goto st178;
		case 45: goto tr262;
	}
	goto st144;
tr262:
#line 222 "command.rl"
	{ neg=-1;}
	goto st178;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
#line 2479 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr263;
	goto st144;
tr263:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st179;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
#line 2491 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr264;
	goto st144;
tr264:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st180;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
#line 2503 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr265;
	goto st144;
tr265:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st181;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
#line 2515 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr266;
	goto st144;
tr266:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st182;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
#line 2527 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr267;
	goto st144;
tr267:
#line 221 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st183;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
#line 2539 "command.cpp"
	if ( (*p) == 35 )
		goto tr268;
	goto st144;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
	if ( (*p) == 35 )
		goto tr269;
	goto st144;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
	if ( (*p) == 35 )
		goto tr270;
	goto st144;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
	switch( (*p) ) {
		case 35: goto tr202;
		case 65: goto st187;
		case 67: goto st188;
		case 68: goto st189;
		case 71: goto st190;
		case 76: goto st191;
		case 77: goto st192;
		case 82: goto st193;
		case 83: goto st194;
		case 84: goto st195;
		case 86: goto st196;
		case 90: goto st202;
		case 99: goto st203;
		case 100: goto st204;
		case 103: goto st205;
		case 107: goto st206;
		case 114: goto st207;
		case 116: goto st208;
	}
	goto st144;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
	if ( (*p) == 35 )
		goto tr288;
	goto st144;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
	if ( (*p) == 35 )
		goto tr289;
	goto st144;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
	if ( (*p) == 35 )
		goto tr290;
	goto st144;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
	if ( (*p) == 35 )
		goto tr291;
	goto st144;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
	if ( (*p) == 35 )
		goto tr292;
	goto st144;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
	if ( (*p) == 35 )
		goto tr293;
	goto st144;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	if ( (*p) == 35 )
		goto tr294;
	goto st144;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	if ( (*p) == 35 )
		goto tr295;
	goto st144;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	if ( (*p) == 35 )
		goto tr296;
	goto st144;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
	switch( (*p) ) {
		case 35: goto tr202;
		case 68: goto st197;
		case 70: goto st198;
		case 78: goto st199;
		case 80: goto st200;
		case 84: goto st201;
	}
	goto st144;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
	if ( (*p) == 35 )
		goto tr302;
	goto st144;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	if ( (*p) == 35 )
		goto tr303;
	goto st144;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	if ( (*p) == 35 )
		goto tr304;
	goto st144;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	if ( (*p) == 35 )
		goto tr305;
	goto st144;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	if ( (*p) == 35 )
		goto tr306;
	goto st144;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	if ( (*p) == 35 )
		goto tr307;
	goto st144;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
	if ( (*p) == 35 )
		goto tr308;
	goto st144;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
	if ( (*p) == 35 )
		goto tr309;
	goto st144;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
	if ( (*p) == 35 )
		goto tr310;
	goto st144;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
	if ( (*p) == 35 )
		goto tr311;
	goto st144;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	if ( (*p) == 35 )
		goto tr312;
	goto st144;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	if ( (*p) == 35 )
		goto tr313;
	goto st144;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
	switch( (*p) ) {
		case 35: goto tr202;
		case 80: goto st210;
	}
	goto st144;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr315;
	goto st144;
tr315:
#line 219 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st211;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
#line 2766 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 46: goto st212;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr317;
	goto st144;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr318;
	goto st144;
tr318:
#line 220 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st213;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
#line 2789 "command.cpp"
	if ( (*p) == 35 )
		goto tr319;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr320;
	goto st144;
tr320:
#line 220 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st214;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
#line 2801 "command.cpp"
	if ( (*p) == 35 )
		goto tr319;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr321;
	goto st144;
tr321:
#line 220 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st215;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
#line 2813 "command.cpp"
	if ( (*p) == 35 )
		goto tr319;
	goto st144;
tr317:
#line 219 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st216;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
#line 2823 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 46: goto st212;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr322;
	goto st144;
tr322:
#line 219 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st217;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
#line 2837 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 46: goto st212;
	}
	goto st144;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
	switch( (*p) ) {
		case 35: goto tr202;
		case 83: goto st219;
		case 101: goto tr324;
		case 103: goto st221;
		case 104: goto tr324;
		case 110: goto tr324;
		case 115: goto tr324;
		case 119: goto tr324;
	}
	goto st144;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
	if ( (*p) == 35 )
		goto tr326;
	goto st144;
tr324:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st220;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
#line 2871 "command.cpp"
	if ( (*p) == 35 )
		goto tr327;
	goto st144;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
	switch( (*p) ) {
		case 35: goto tr202;
		case 101: goto tr328;
		case 110: goto tr328;
		case 115: goto tr328;
		case 119: goto tr328;
	}
	goto st144;
tr328:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st222;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
#line 2893 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr329;
	goto st144;
tr329:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st223;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
#line 2905 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr330;
	goto st144;
tr330:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st224;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
#line 2917 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr331;
	goto st144;
tr331:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st225;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
#line 2929 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr332;
	goto st144;
tr332:
#line 218 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st226;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
#line 2941 "command.cpp"
	if ( (*p) == 35 )
		goto tr333;
	goto st144;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
	switch( (*p) ) {
		case 35: goto tr334;
		case 101: goto tr335;
		case 110: goto tr335;
		case 115: goto tr335;
		case 119: goto tr335;
	}
	goto st144;
tr335:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st228;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
#line 2963 "command.cpp"
	if ( (*p) == 35 )
		goto tr334;
	goto st144;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
	switch( (*p) ) {
		case 35: goto tr202;
		case 67: goto tr336;
		case 71: goto tr336;
		case 77: goto tr336;
		case 83: goto tr336;
	}
	goto st144;
tr336:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st230;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
#line 2985 "command.cpp"
	if ( (*p) == 35 )
		goto tr337;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st231;
	goto st144;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
	if ( (*p) == 35 )
		goto tr337;
	goto st144;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
	switch( (*p) ) {
		case 35: goto tr202;
		case 67: goto st233;
		case 71: goto st243;
		case 76: goto st250;
		case 83: goto tr342;
		case 97: goto tr343;
		case 100: goto tr343;
		case 103: goto tr343;
		case 114: goto tr342;
		case 116: goto tr343;
		case 122: goto tr343;
	}
	goto st144;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	switch( (*p) ) {
		case 32: goto st234;
		case 35: goto tr202;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr345;
	} else if ( (*p) >= 9 )
		goto st234;
	goto st144;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr345;
	goto st144;
tr345:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st235;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
#line 3045 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr346;
	goto st144;
tr346:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st236;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
#line 3057 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 47: goto st237;
	}
	goto st144;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr348;
	goto st144;
tr348:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st238;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
#line 3078 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr349;
	goto st144;
tr349:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st239;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
#line 3090 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 47: goto st240;
	}
	goto st144;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr351;
	goto st144;
tr351:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st241;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
#line 3111 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr352;
	goto st144;
tr352:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st242;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
#line 3123 "command.cpp"
	if ( (*p) == 35 )
		goto tr353;
	goto st144;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
	switch( (*p) ) {
		case 32: goto st244;
		case 35: goto tr202;
		case 43: goto st245;
		case 45: goto tr356;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st244;
	goto st144;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
	switch( (*p) ) {
		case 35: goto tr202;
		case 43: goto st245;
		case 45: goto tr356;
	}
	goto st144;
tr356:
#line 222 "command.rl"
	{ neg=-1;}
	goto st245;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
#line 3156 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr357;
	goto st144;
tr357:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st246;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
#line 3168 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr358;
		case 44: goto st247;
		case 46: goto st247;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr360;
	goto st144;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st248;
	goto st144;
st248:
	if ( ++p == pe )
		goto _test_eof248;
case 248:
	if ( (*p) == 35 )
		goto tr358;
	goto st144;
tr360:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st249;
st249:
	if ( ++p == pe )
		goto _test_eof249;
case 249:
#line 3199 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr358;
		case 44: goto st247;
		case 46: goto st247;
	}
	goto st144;
st250:
	if ( ++p == pe )
		goto _test_eof250;
case 250:
	switch( (*p) ) {
		case 32: goto st251;
		case 35: goto tr202;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr363;
	} else if ( (*p) >= 9 )
		goto st251;
	goto st144;
st251:
	if ( ++p == pe )
		goto _test_eof251;
case 251:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr363;
	goto st144;
tr363:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st252;
st252:
	if ( ++p == pe )
		goto _test_eof252;
case 252:
#line 3235 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr364;
	goto st144;
tr364:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st253;
st253:
	if ( ++p == pe )
		goto _test_eof253;
case 253:
#line 3247 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 58: goto st254;
	}
	goto st144;
st254:
	if ( ++p == pe )
		goto _test_eof254;
case 254:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr366;
	goto st144;
tr366:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st255;
st255:
	if ( ++p == pe )
		goto _test_eof255;
case 255:
#line 3268 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr367;
	goto st144;
tr367:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st256;
st256:
	if ( ++p == pe )
		goto _test_eof256;
case 256:
#line 3280 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 58: goto st257;
	}
	goto st144;
st257:
	if ( ++p == pe )
		goto _test_eof257;
case 257:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr369;
	goto st144;
tr369:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st258;
st258:
	if ( ++p == pe )
		goto _test_eof258;
case 258:
#line 3301 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr370;
	goto st144;
tr370:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st259;
st259:
	if ( ++p == pe )
		goto _test_eof259;
case 259:
#line 3313 "command.cpp"
	if ( (*p) == 35 )
		goto tr371;
	goto st144;
tr342:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st260;
st260:
	if ( ++p == pe )
		goto _test_eof260;
case 260:
#line 3323 "command.cpp"
	switch( (*p) ) {
		case 32: goto st261;
		case 35: goto tr202;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr373;
	} else if ( (*p) >= 9 )
		goto st261;
	goto st144;
st261:
	if ( ++p == pe )
		goto _test_eof261;
case 261:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr373;
	goto st144;
tr373:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st262;
st262:
	if ( ++p == pe )
		goto _test_eof262;
case 262:
#line 3349 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr374;
	goto st144;
tr374:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st263;
st263:
	if ( ++p == pe )
		goto _test_eof263;
case 263:
#line 3361 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 47: goto st264;
		case 58: goto st264;
	}
	goto st144;
st264:
	if ( ++p == pe )
		goto _test_eof264;
case 264:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr376;
	goto st144;
tr376:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st265;
st265:
	if ( ++p == pe )
		goto _test_eof265;
case 265:
#line 3383 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr377;
	goto st144;
tr377:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 258 "command.rl"
	{deg=deg*3600+min*60;}
	goto st266;
st266:
	if ( ++p == pe )
		goto _test_eof266;
case 266:
#line 3396 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr202;
		case 46: goto st267;
		case 47: goto st269;
		case 58: goto st269;
	}
	goto st144;
st267:
	if ( ++p == pe )
		goto _test_eof267;
case 267:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr380;
	goto st144;
tr380:
#line 252 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st268;
st268:
	if ( ++p == pe )
		goto _test_eof268;
case 268:
#line 3419 "command.cpp"
	if ( (*p) == 35 )
		goto tr381;
	goto st144;
st269:
	if ( ++p == pe )
		goto _test_eof269;
case 269:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr382;
	goto st144;
tr382:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st270;
st270:
	if ( ++p == pe )
		goto _test_eof270;
case 270:
#line 3438 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr383;
	goto st144;
tr383:
#line 217 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 259 "command.rl"
	{deg+=sec;}
	goto st271;
st271:
	if ( ++p == pe )
		goto _test_eof271;
case 271:
#line 3451 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr381;
		case 44: goto st272;
		case 46: goto st272;
		case 58: goto st268;
	}
	goto st144;
st272:
	if ( ++p == pe )
		goto _test_eof272;
case 272:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st273;
	goto st144;
st273:
	if ( ++p == pe )
		goto _test_eof273;
case 273:
	switch( (*p) ) {
		case 35: goto tr381;
		case 58: goto st268;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st274;
	goto st144;
st274:
	if ( ++p == pe )
		goto _test_eof274;
case 274:
	switch( (*p) ) {
		case 35: goto tr381;
		case 58: goto st268;
	}
	goto st144;
tr343:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st275;
st275:
	if ( ++p == pe )
		goto _test_eof275;
case 275:
#line 3494 "command.cpp"
	switch( (*p) ) {
		case 32: goto st276;
		case 35: goto tr202;
		case 43: goto st277;
		case 45: goto tr390;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr391;
	} else if ( (*p) >= 9 )
		goto st276;
	goto st144;
st276:
	if ( ++p == pe )
		goto _test_eof276;
case 276:
	switch( (*p) ) {
		case 32: goto st277;
		case 35: goto tr202;
		case 43: goto st277;
		case 45: goto tr390;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr391;
	} else if ( (*p) >= 9 )
		goto st277;
	goto st144;
tr390:
#line 222 "command.rl"
	{ neg=-1;}
	goto st277;
st277:
	if ( ++p == pe )
		goto _test_eof277;
case 277:
#line 3529 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr391;
	goto st144;
tr391:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st278;
st278:
	if ( ++p == pe )
		goto _test_eof278;
case 278:
#line 3541 "command.cpp"
	if ( (*p) == 35 )
		goto tr393;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr394;
		} else if ( (*p) >= 33 )
			goto st279;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st279;
		} else if ( (*p) >= 91 )
			goto st279;
	} else
		goto st279;
	goto st144;
st279:
	if ( ++p == pe )
		goto _test_eof279;
case 279:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr395;
	goto st144;
tr395:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st280;
st280:
	if ( ++p == pe )
		goto _test_eof280;
case 280:
#line 3574 "command.cpp"
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr396;
	goto st144;
tr396:
#line 216 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 258 "command.rl"
	{deg=deg*3600+min*60;}
	goto st281;
st281:
	if ( ++p == pe )
		goto _test_eof281;
case 281:
#line 3587 "command.cpp"
	if ( (*p) == 35 )
		goto tr397;
	goto st269;
tr397:
#line 255 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st306;
st306:
	if ( ++p == pe )
		goto _test_eof306;
case 306:
#line 3605 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr426;
		case 35: goto tr202;
		case 58: goto st146;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr382;
	goto st144;
tr393:
#line 305 "command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						}
						}
	goto st307;
st307:
	if ( ++p == pe )
		goto _test_eof307;
case 307:
#line 3625 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr426;
		case 35: goto tr202;
		case 58: goto st146;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr395;
	goto st144;
tr394:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st282;
st282:
	if ( ++p == pe )
		goto _test_eof282;
case 282:
#line 3640 "command.cpp"
	if ( (*p) == 35 )
		goto tr393;
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr398;
		} else if ( (*p) >= 33 )
			goto st279;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st279;
		} else if ( (*p) >= 91 )
			goto st279;
	} else
		goto st279;
	goto st144;
tr398:
#line 215 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st283;
st283:
	if ( ++p == pe )
		goto _test_eof283;
case 283:
#line 3664 "command.cpp"
	if ( (*p) == 35 )
		goto tr393;
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st279;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st279;
		} else if ( (*p) >= 91 )
			goto st279;
	} else
		goto st279;
	goto st144;
st284:
	if ( ++p == pe )
		goto _test_eof284;
case 284:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr399;
	goto st144;
st285:
	if ( ++p == pe )
		goto _test_eof285;
case 285:
	switch( (*p) ) {
		case 35: goto tr202;
		case 65: goto tr400;
		case 97: goto tr400;
		case 103: goto tr400;
		case 106: goto tr400;
		case 110: goto tr199;
		case 115: goto tr199;
		case 119: goto tr199;
		case 122: goto tr400;
	}
	goto st144;
tr400:
#line 260 "command.rl"
	{stcmd=(*p);}
	goto st286;
st286:
	if ( ++p == pe )
		goto _test_eof286;
case 286:
#line 3710 "command.cpp"
	if ( (*p) == 35 )
		goto tr401;
	goto st144;
st287:
	if ( ++p == pe )
		goto _test_eof287;
case 287:
	switch( (*p) ) {
		case 35: goto tr202;
		case 80: goto st288;
	}
	goto st144;
st288:
	if ( ++p == pe )
		goto _test_eof288;
case 288:
	if ( (*p) == 35 )
		goto tr403;
	goto st144;
st289:
	if ( ++p == pe )
		goto _test_eof289;
case 289:
	switch( (*p) ) {
		case 35: goto tr202;
		case 72: goto st290;
		case 83: goto st291;
		case 110: goto st292;
		case 115: goto st294;
	}
	goto st144;
st290:
	if ( ++p == pe )
		goto _test_eof290;
case 290:
	if ( (*p) == 35 )
		goto tr408;
	goto st144;
st291:
	if ( ++p == pe )
		goto _test_eof291;
case 291:
	if ( (*p) == 35 )
		goto tr409;
	goto st144;
st292:
	if ( ++p == pe )
		goto _test_eof292;
case 292:
	switch( (*p) ) {
		case 35: goto tr202;
		case 107: goto st293;
	}
	goto st144;
st293:
	if ( ++p == pe )
		goto _test_eof293;
case 293:
	if ( (*p) == 35 )
		goto tr202;
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr411;
	goto st144;
st294:
	if ( ++p == pe )
		goto _test_eof294;
case 294:
	switch( (*p) ) {
		case 35: goto tr202;
		case 101: goto tr412;
		case 119: goto tr412;
	}
	goto st144;
st295:
	if ( ++p == pe )
		goto _test_eof295;
case 295:
	if ( (*p) == 80 )
		goto st296;
	goto st0;
st296:
	if ( ++p == pe )
		goto _test_eof296;
case 296:
	if ( (*p) == 35 )
		goto tr414;
	goto st0;
st297:
	if ( ++p == pe )
		goto _test_eof297;
case 297:
	switch( (*p) ) {
		case 72: goto st298;
		case 83: goto st299;
		case 110: goto st300;
		case 115: goto st302;
	}
	goto st0;
st298:
	if ( ++p == pe )
		goto _test_eof298;
case 298:
	if ( (*p) == 35 )
		goto tr419;
	goto st0;
st299:
	if ( ++p == pe )
		goto _test_eof299;
case 299:
	if ( (*p) == 35 )
		goto tr420;
	goto st0;
st300:
	if ( ++p == pe )
		goto _test_eof300;
case 300:
	if ( (*p) == 107 )
		goto st301;
	goto st0;
st301:
	if ( ++p == pe )
		goto _test_eof301;
case 301:
	if ( 48 <= (*p) && (*p) <= 49 )
		goto tr422;
	goto st0;
st302:
	if ( ++p == pe )
		goto _test_eof302;
case 302:
	switch( (*p) ) {
		case 101: goto tr423;
		case 119: goto tr423;
	}
	goto st0;
	}
	_test_eof303: cs = 303; goto _test_eof; 
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
	_test_eof304: cs = 304; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof305: cs = 305; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
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
	_test_eof306: cs = 306; goto _test_eof; 
	_test_eof307: cs = 307; goto _test_eof; 
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
	_test_eof296: cs = 296; goto _test_eof; 
	_test_eof297: cs = 297; goto _test_eof; 
	_test_eof298: cs = 298; goto _test_eof; 
	_test_eof299: cs = 299; goto _test_eof; 
	_test_eof300: cs = 300; goto _test_eof; 
	_test_eof301: cs = 301; goto _test_eof; 
	_test_eof302: cs = 302; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 391 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
		return 0;
};



