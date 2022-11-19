
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

char response [200];
char tmessage[50];
extern c_star st_now, st_target, st_current;
extern char volatile sync_target;
extern stepper focus_motor;
extern int  focuspeed;
extern int  focuspeed_low;
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

void lxprintsite(void)
{
    sprintf(tmessage,"Site Name#");APPEND;
};


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
	int pulse=0;
	int focus_counter=0;
    
#line 2 "command.cpp"
static const int command_start = 128;
static const int command_first_final = 128;
static const int command_error = 0;

static const int command_en_main = 128;


#line 139 "command.rl"





    
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
tr15:
#line 209 "command.rl"
	{;}
	goto st128;
tr17:
#line 207 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	goto st128;
tr28:
#line 189 "command.rl"
	{set_date(min,deg,sec);}
	goto st128;
tr29:
#line 175 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	goto st128;
tr31:
#line 211 "command.rl"
	{if (telescope->azmotor->slewing || telescope->altmotor->slewing) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	goto st128;
tr39:
#line 199 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
	goto st128;
tr41:
#line 197 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
	goto st128;
tr42:
#line 198 "command.rl"
	{gotofocuser(0,focuspeed_low);}
	goto st128;
tr44:
#line 196 "command.rl"
	{gotofocuser(0,focuspeed);}
	goto st128;
tr52:
#line 201 "command.rl"
	{gotofocuser(focus_counter);}
	goto st128;
tr62:
#line 203 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
	goto st128;
tr70:
#line 200 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
	goto st128;
tr71:
#line 202 "command.rl"
	{stopfocuser();}
	goto st128;
tr72:
#line 204 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st128;
tr92:
#line 191 "command.rl"
	{ telescope->time_zone=deg;}
#line 170 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	goto st128;
tr96:
#line 165 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	goto st128;
tr97:
#line 168 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	goto st128;
tr98:
#line 162 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	goto st128;
tr99:
#line 192 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	goto st128;
tr100:
#line 182 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	goto st128;
tr101:
#line 169 "command.rl"
	{ lxprintsite();}
	goto st128;
tr102:
#line 160 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	goto st128;
tr103:
#line 195 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	goto st128;
tr109:
#line 212 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
	goto st128;
tr110:
#line 216 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	goto st128;
tr111:
#line 213 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
	goto st128;
tr112:
#line 214 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	goto st128;
tr113:
#line 215 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
	goto st128;
tr114:
#line 164 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	goto st128;
tr115:
#line 194 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	goto st128;
tr116:
#line 167 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	goto st128;
tr117:
#line 171 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	goto st128;
tr118:
#line 166 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	goto st128;
tr119:
#line 172 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	goto st128;
tr129:
#line 193 "command.rl"
	{set_time(deg,min,sec);}
	goto st128;
tr133:
#line 154 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
	goto st128;
tr134:
#line 152 "command.rl"
	{mount_move(telescope,stcmd);}
	goto st128;
tr140:
#line 153 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	goto st128;
tr141:
#line 208 "command.rl"
	{telescope->track=1;}
	goto st128;
tr142:
#line 158 "command.rl"
	{mount_stop(telescope,stcmd);}
	goto st128;
tr145:
#line 159 "command.rl"
	{select_rate(telescope,stcmd); }
	goto st128;
tr158:
#line 183 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st128;
tr175:
#line 205 "command.rl"
	{mount_goto_home(telescope);}
	goto st128;
tr177:
#line 206 "command.rl"
	{mount_home_set(telescope);}
	goto st128;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
#line 165 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr178;
		case 35: goto st1;
		case 58: goto st2;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr178:
#line 190 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 181 "command.cpp"
	if ( (*p) == 58 )
		goto st2;
	goto st0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 65: goto st3;
		case 67: goto st5;
		case 68: goto st17;
		case 70: goto st18;
		case 71: goto st48;
		case 76: goto st75;
		case 77: goto st85;
		case 80: goto st94;
		case 81: goto st95;
		case 82: goto st97;
		case 83: goto st100;
		case 104: goto st124;
		case 112: goto st126;
	}
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 35: goto tr15;
		case 76: goto st4;
	}
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 35 )
		goto tr17;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 32: goto st6;
		case 77: goto st15;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr19;
	} else if ( (*p) >= 9 )
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr19;
	goto st0;
tr19:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 248 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr21;
	goto st0;
tr21:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 258 "command.cpp"
	if ( (*p) == 47 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr23;
	goto st0;
tr23:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 275 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr24;
	goto st0;
tr24:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 285 "command.cpp"
	if ( (*p) == 47 )
		goto st12;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr26;
	goto st0;
tr26:
#line 148 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 302 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr27;
	goto st0;
tr27:
#line 148 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 312 "command.cpp"
	if ( (*p) == 35 )
		goto tr28;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 35: goto tr29;
		case 82: goto st16;
	}
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 35 )
		goto tr29;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 35 )
		goto tr31;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 43: goto st19;
		case 45: goto st21;
		case 65: goto st23;
		case 76: goto st30;
		case 80: goto st39;
		case 81: goto st46;
		case 112: goto st47;
	}
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 35: goto tr39;
		case 43: goto st20;
	}
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( (*p) == 35 )
		goto tr41;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 35: goto tr42;
		case 45: goto st22;
	}
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( (*p) == 35 )
		goto tr44;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 43: goto st24;
		case 45: goto tr46;
	}
	goto st0;
tr46:
#line 151 "command.rl"
	{ neg=-1;}
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 400 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr47;
	goto st0;
tr47:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 410 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr48;
	goto st0;
tr48:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 420 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr49;
	goto st0;
tr49:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 430 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr50;
	goto st0;
tr50:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 440 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr51;
	goto st0;
tr51:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 450 "command.cpp"
	if ( (*p) == 35 )
		goto tr52;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 83 )
		goto st31;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( (*p) == 49 )
		goto st32;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 43: goto st33;
		case 45: goto tr56;
	}
	goto st0;
tr56:
#line 151 "command.rl"
	{ neg=-1;}
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 483 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr57;
	goto st0;
tr57:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 493 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr58;
	goto st0;
tr58:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 503 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr59;
	goto st0;
tr59:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 513 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr60;
	goto st0;
tr60:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 523 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr61;
	goto st0;
tr61:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 533 "command.cpp"
	if ( (*p) == 35 )
		goto tr62;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	switch( (*p) ) {
		case 43: goto st40;
		case 45: goto tr64;
	}
	goto st0;
tr64:
#line 151 "command.rl"
	{ neg=-1;}
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
#line 552 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr65;
	goto st0;
tr65:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 562 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr66;
	goto st0;
tr66:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
#line 572 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr67;
	goto st0;
tr67:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 582 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr68;
	goto st0;
tr68:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 592 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr69;
	goto st0;
tr69:
#line 150 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 602 "command.cpp"
	if ( (*p) == 35 )
		goto tr70;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 35 )
		goto tr71;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 35 )
		goto tr72;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 32: goto st49;
		case 43: goto st50;
		case 45: goto tr75;
		case 65: goto st55;
		case 67: goto st56;
		case 68: goto st57;
		case 71: goto st58;
		case 76: goto st59;
		case 77: goto st60;
		case 82: goto st61;
		case 83: goto st59;
		case 84: goto st62;
		case 86: goto st63;
		case 90: goto st69;
		case 99: goto st70;
		case 100: goto st71;
		case 103: goto st72;
		case 114: goto st73;
		case 116: goto st74;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st49;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	switch( (*p) ) {
		case 43: goto st50;
		case 45: goto tr75;
	}
	goto st0;
tr75:
#line 151 "command.rl"
	{ neg=-1;}
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
#line 663 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr91;
	goto st0;
tr91:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 673 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr92;
		case 46: goto st52;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr94;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st53;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) == 35 )
		goto tr92;
	goto st0;
tr94:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st54;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
#line 701 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr92;
		case 46: goto st52;
	}
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 35 )
		goto tr96;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 35 )
		goto tr97;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 35 )
		goto tr98;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 35 )
		goto tr99;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 35 )
		goto tr100;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 35 )
		goto tr101;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 35 )
		goto tr102;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 35 )
		goto tr103;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	switch( (*p) ) {
		case 68: goto st64;
		case 70: goto st65;
		case 78: goto st66;
		case 80: goto st67;
		case 84: goto st68;
	}
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 35 )
		goto tr109;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 35 )
		goto tr110;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 35 )
		goto tr111;
	goto st0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( (*p) == 35 )
		goto tr112;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( (*p) == 35 )
		goto tr113;
	goto st0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( (*p) == 35 )
		goto tr114;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) == 35 )
		goto tr115;
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 35 )
		goto tr116;
	goto st0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 35 )
		goto tr117;
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 35 )
		goto tr118;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( (*p) == 35 )
		goto tr119;
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( (*p) == 32 )
		goto st76;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr121;
	} else if ( (*p) >= 9 )
		goto st76;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr121;
	goto st0;
tr121:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
#line 877 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr122;
	goto st0;
tr122:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
#line 887 "command.cpp"
	if ( (*p) == 58 )
		goto st79;
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr124;
	goto st0;
tr124:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 904 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr125;
	goto st0;
tr125:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
#line 914 "command.cpp"
	if ( (*p) == 58 )
		goto st82;
	goto st0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr127;
	goto st0;
tr127:
#line 148 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st83;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
#line 931 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr128;
	goto st0;
tr128:
#line 148 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 941 "command.cpp"
	if ( (*p) == 35 )
		goto tr129;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 83: goto st86;
		case 101: goto tr131;
		case 103: goto st88;
		case 110: goto tr131;
		case 115: goto tr131;
		case 119: goto tr131;
	}
	goto st0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	if ( (*p) == 35 )
		goto tr133;
	goto st0;
tr131:
#line 188 "command.rl"
	{stcmd=(*p);}
	goto st87;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
#line 971 "command.cpp"
	if ( (*p) == 35 )
		goto tr134;
	goto st0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	switch( (*p) ) {
		case 101: goto tr135;
		case 110: goto tr135;
		case 115: goto tr135;
		case 119: goto tr135;
	}
	goto st0;
tr135:
#line 188 "command.rl"
	{stcmd=(*p);}
	goto st89;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
#line 992 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr136;
	goto st0;
tr136:
#line 149 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st90;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
#line 1002 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr137;
	goto st0;
tr137:
#line 149 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st91;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
#line 1012 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr138;
	goto st0;
tr138:
#line 149 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st92;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
#line 1022 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr139;
	goto st0;
tr139:
#line 149 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st93;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
#line 1032 "command.cpp"
	if ( (*p) == 35 )
		goto tr140;
	goto st0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( (*p) == 35 )
		goto tr141;
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	switch( (*p) ) {
		case 35: goto tr142;
		case 101: goto tr143;
		case 110: goto tr143;
		case 115: goto tr143;
		case 119: goto tr143;
	}
	goto st0;
tr143:
#line 188 "command.rl"
	{stcmd=(*p);}
	goto st96;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
#line 1061 "command.cpp"
	if ( (*p) == 35 )
		goto tr142;
	goto st0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	switch( (*p) ) {
		case 67: goto tr144;
		case 71: goto tr144;
		case 77: goto tr144;
		case 83: goto tr144;
	}
	goto st0;
tr144:
#line 188 "command.rl"
	{stcmd=(*p);}
	goto st98;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
#line 1082 "command.cpp"
	if ( (*p) == 35 )
		goto tr145;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st99;
	goto st0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	if ( (*p) == 35 )
		goto tr145;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	switch( (*p) ) {
		case 83: goto tr147;
		case 97: goto tr148;
		case 100: goto tr148;
		case 103: goto tr148;
		case 114: goto tr147;
		case 116: goto tr148;
		case 122: goto tr148;
	}
	goto st0;
tr147:
#line 188 "command.rl"
	{stcmd=(*p);}
	goto st101;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
#line 1115 "command.cpp"
	if ( (*p) == 32 )
		goto st102;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr150;
	} else if ( (*p) >= 9 )
		goto st102;
	goto st0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr150;
	goto st0;
tr150:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st103;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
#line 1137 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr151;
	goto st0;
tr151:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st104;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
#line 1147 "command.cpp"
	switch( (*p) ) {
		case 47: goto st105;
		case 58: goto st105;
	}
	goto st0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr153;
	goto st0;
tr153:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st106;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
#line 1166 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr154;
	goto st0;
tr154:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 186 "command.rl"
	{deg=deg*3600+min*60;}
	goto st107;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
#line 1177 "command.cpp"
	switch( (*p) ) {
		case 46: goto st108;
		case 47: goto st110;
		case 58: goto st110;
	}
	goto st0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr157;
	goto st0;
tr157:
#line 181 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 1197 "command.cpp"
	if ( (*p) == 35 )
		goto tr158;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr159;
	goto st0;
tr159:
#line 148 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st111;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
#line 1214 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr160;
	goto st0;
tr160:
#line 148 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 187 "command.rl"
	{deg+=sec;}
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1225 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr158;
		case 46: goto st113;
	}
	goto st0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st114;
	goto st0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( (*p) == 35 )
		goto tr158;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st109;
	goto st0;
tr148:
#line 188 "command.rl"
	{stcmd=(*p);}
	goto st115;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
#line 1253 "command.cpp"
	switch( (*p) ) {
		case 32: goto st116;
		case 43: goto st117;
		case 45: goto tr166;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr167;
	} else if ( (*p) >= 9 )
		goto st116;
	goto st0;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	switch( (*p) ) {
		case 32: goto st117;
		case 43: goto st117;
		case 45: goto tr166;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr167;
	} else if ( (*p) >= 9 )
		goto st117;
	goto st0;
tr166:
#line 151 "command.rl"
	{ neg=-1;}
	goto st117;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
#line 1286 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr167;
	goto st0;
tr167:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
#line 1296 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr169;
		} else if ( (*p) >= 33 )
			goto st119;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st119;
		} else if ( (*p) >= 91 )
			goto st119;
	} else
		goto st119;
	goto st0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr170;
	goto st0;
tr170:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st120;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
#line 1325 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr171;
	goto st0;
tr171:
#line 147 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 186 "command.rl"
	{deg=deg*3600+min*60;}
	goto st121;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
#line 1336 "command.cpp"
	if ( (*p) == 35 )
		goto tr172;
	goto st110;
tr172:
#line 183 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st129;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
#line 1348 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr178;
		case 35: goto st1;
		case 58: goto st2;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr159;
	goto st0;
tr169:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st122;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
#line 1363 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr173;
		} else if ( (*p) >= 33 )
			goto st119;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st119;
		} else if ( (*p) >= 91 )
			goto st119;
	} else
		goto st119;
	goto st0;
tr173:
#line 146 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st123;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
#line 1385 "command.cpp"
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st119;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st119;
		} else if ( (*p) >= 91 )
			goto st119;
	} else
		goto st119;
	goto st0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	if ( (*p) == 80 )
		goto st125;
	goto st0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( (*p) == 35 )
		goto tr175;
	goto st0;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	if ( (*p) == 72 )
		goto st127;
	goto st0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	if ( (*p) == 35 )
		goto tr177;
	goto st0;
	}
	_test_eof128: cs = 128; goto _test_eof; 
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
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 283 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
};



