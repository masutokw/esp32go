
#line 1 "command.rl"
/*
 * Parses LX200 protocol you must process this file with RAGEL compiler to get command.cpp file
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
	sprintf(tmessage,"%cUpdating Planetary Data#     #",'1');APPEND;
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
static const int command_start = 116;
static const int command_first_final = 116;
static const int command_error = 0;

static const int command_en_main = 116;


#line 136 "command.rl"





    
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
tr11:
#line 172 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	goto st116;
tr20:
#line 194 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
	goto st116;
tr22:
#line 192 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
	goto st116;
tr23:
#line 193 "command.rl"
	{gotofocuser(0,focuspeed_low);}
	goto st116;
tr25:
#line 191 "command.rl"
	{gotofocuser(0,focuspeed);}
	goto st116;
tr33:
#line 196 "command.rl"
	{gotofocuser(focus_counter);}
	goto st116;
tr43:
#line 198 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
	goto st116;
tr51:
#line 195 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
	goto st116;
tr52:
#line 197 "command.rl"
	{stopfocuser();}
	goto st116;
tr53:
#line 199 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st116;
tr66:
#line 162 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	goto st116;
tr67:
#line 165 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	goto st116;
tr68:
#line 159 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	goto st116;
tr69:
#line 189 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	goto st116;
tr70:
#line 179 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	goto st116;
tr71:
#line 166 "command.rl"
	{ lxprintsite();}
	goto st116;
tr72:
#line 157 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	goto st116;
tr73:
#line 161 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	goto st116;
tr74:
#line 164 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	goto st116;
tr75:
#line 168 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	goto st116;
tr76:
#line 163 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	goto st116;
tr77:
#line 169 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	goto st116;
tr81:
#line 151 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
	goto st116;
tr82:
#line 149 "command.rl"
	{mount_move(telescope,stcmd);}
	goto st116;
tr88:
#line 150 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	goto st116;
tr89:
#line 155 "command.rl"
	{mount_stop(telescope,stcmd);}
	goto st116;
tr92:
#line 156 "command.rl"
	{select_rate(telescope,stcmd); }
	goto st116;
tr108:
#line 186 "command.rl"
	{set_date(min,deg,sec);}
	goto st116;
tr113:
#line 188 "command.rl"
	{ telescope->time_zone=deg;}
#line 167 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	goto st116;
tr126:
#line 190 "command.rl"
	{set_time(deg,min,sec);}
	goto st116;
tr136:
#line 180 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st116;
tr153:
#line 200 "command.rl"
	{mount_home_set(telescope);}
	goto st116;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
#line 129 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr154;
		case 35: goto st1;
		case 58: goto st2;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr154:
#line 187 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else sprintf(tmessage,"P") ; APPEND; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 145 "command.cpp"
	if ( (*p) == 58 )
		goto st2;
	goto st0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 67: goto st3;
		case 70: goto st6;
		case 71: goto st36;
		case 77: goto st49;
		case 81: goto st58;
		case 82: goto st60;
		case 83: goto st63;
		case 112: goto st114;
	}
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 77 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 35: goto tr11;
		case 82: goto st5;
	}
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 35 )
		goto tr11;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 43: goto st7;
		case 45: goto st9;
		case 65: goto st11;
		case 76: goto st18;
		case 80: goto st27;
		case 81: goto st34;
		case 112: goto st35;
	}
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 35: goto tr20;
		case 43: goto st8;
	}
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 35 )
		goto tr22;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 35: goto tr23;
		case 45: goto st10;
	}
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
		case 45: goto tr27;
	}
	goto st0;
tr27:
#line 148 "command.rl"
	{ neg=-1;}
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 248 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr28;
	goto st0;
tr28:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 258 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr29;
	goto st0;
tr29:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 268 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr30;
	goto st0;
tr30:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 278 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr31;
	goto st0;
tr31:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 288 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr32;
	goto st0;
tr32:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 298 "command.cpp"
	if ( (*p) == 35 )
		goto tr33;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 83 )
		goto st19;
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( (*p) == 49 )
		goto st20;
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 43: goto st21;
		case 45: goto tr37;
	}
	goto st0;
tr37:
#line 148 "command.rl"
	{ neg=-1;}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 331 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr38;
	goto st0;
tr38:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 341 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr39;
	goto st0;
tr39:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 351 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr40;
	goto st0;
tr40:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 361 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr41;
	goto st0;
tr41:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 371 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr42;
	goto st0;
tr42:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 381 "command.cpp"
	if ( (*p) == 35 )
		goto tr43;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	switch( (*p) ) {
		case 43: goto st28;
		case 45: goto tr45;
	}
	goto st0;
tr45:
#line 148 "command.rl"
	{ neg=-1;}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 400 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr46;
	goto st0;
tr46:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 410 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr47;
	goto st0;
tr47:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 420 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr48;
	goto st0;
tr48:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 430 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr49;
	goto st0;
tr49:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 440 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr50;
	goto st0;
tr50:
#line 147 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 450 "command.cpp"
	if ( (*p) == 35 )
		goto tr51;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 35 )
		goto tr52;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 35 )
		goto tr53;
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	switch( (*p) ) {
		case 65: goto st37;
		case 67: goto st38;
		case 68: goto st39;
		case 71: goto st40;
		case 76: goto st41;
		case 77: goto st42;
		case 82: goto st43;
		case 83: goto st41;
		case 90: goto st44;
		case 100: goto st45;
		case 103: goto st46;
		case 114: goto st47;
		case 116: goto st48;
	}
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) == 35 )
		goto tr66;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( (*p) == 35 )
		goto tr67;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( (*p) == 35 )
		goto tr68;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) == 35 )
		goto tr69;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 35 )
		goto tr70;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) == 35 )
		goto tr71;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 35 )
		goto tr72;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 35 )
		goto tr73;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 35 )
		goto tr74;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 35 )
		goto tr75;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 35 )
		goto tr76;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 35 )
		goto tr77;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	switch( (*p) ) {
		case 83: goto st50;
		case 101: goto tr79;
		case 103: goto st52;
		case 110: goto tr79;
		case 115: goto tr79;
		case 119: goto tr79;
	}
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 35 )
		goto tr81;
	goto st0;
tr79:
#line 185 "command.rl"
	{stcmd=(*p);}
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 598 "command.cpp"
	if ( (*p) == 35 )
		goto tr82;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 101: goto tr83;
		case 110: goto tr83;
		case 115: goto tr83;
		case 119: goto tr83;
	}
	goto st0;
tr83:
#line 185 "command.rl"
	{stcmd=(*p);}
	goto st53;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
#line 619 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr84;
	goto st0;
tr84:
#line 146 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st54;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
#line 629 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr85;
	goto st0;
tr85:
#line 146 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st55;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
#line 639 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr86;
	goto st0;
tr86:
#line 146 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st56;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
#line 649 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr87;
	goto st0;
tr87:
#line 146 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st57;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
#line 659 "command.cpp"
	if ( (*p) == 35 )
		goto tr88;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	switch( (*p) ) {
		case 35: goto tr89;
		case 101: goto tr90;
		case 110: goto tr90;
		case 115: goto tr90;
		case 119: goto tr90;
	}
	goto st0;
tr90:
#line 185 "command.rl"
	{stcmd=(*p);}
	goto st59;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
#line 681 "command.cpp"
	if ( (*p) == 35 )
		goto tr89;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	switch( (*p) ) {
		case 67: goto tr91;
		case 71: goto tr91;
		case 77: goto tr91;
		case 83: goto tr91;
	}
	goto st0;
tr91:
#line 185 "command.rl"
	{stcmd=(*p);}
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
#line 702 "command.cpp"
	if ( (*p) == 35 )
		goto tr92;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st62;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 35 )
		goto tr92;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	switch( (*p) ) {
		case 67: goto st64;
		case 71: goto st74;
		case 76: goto st81;
		case 83: goto tr97;
		case 97: goto tr98;
		case 100: goto tr98;
		case 103: goto tr98;
		case 114: goto tr97;
		case 116: goto tr98;
		case 122: goto tr98;
	}
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 32 )
		goto st65;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr100;
	} else if ( (*p) >= 9 )
		goto st65;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr100;
	goto st0;
tr100:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st66;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
#line 757 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr101;
	goto st0;
tr101:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 767 "command.cpp"
	if ( (*p) == 47 )
		goto st68;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr103;
	goto st0;
tr103:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 784 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr104;
	goto st0;
tr104:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 794 "command.cpp"
	if ( (*p) == 47 )
		goto st71;
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr106;
	goto st0;
tr106:
#line 145 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 811 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr107;
	goto st0;
tr107:
#line 145 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 821 "command.cpp"
	if ( (*p) == 35 )
		goto tr108;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	switch( (*p) ) {
		case 32: goto st75;
		case 43: goto st76;
		case 45: goto tr111;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st75;
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	switch( (*p) ) {
		case 43: goto st76;
		case 45: goto tr111;
	}
	goto st0;
tr111:
#line 148 "command.rl"
	{ neg=-1;}
	goto st76;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
#line 852 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr112;
	goto st0;
tr112:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
#line 862 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr113;
		case 46: goto st78;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr115;
	goto st0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st79;
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( (*p) == 35 )
		goto tr113;
	goto st0;
tr115:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 890 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr113;
		case 46: goto st78;
	}
	goto st0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	if ( (*p) == 32 )
		goto st82;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr118;
	} else if ( (*p) >= 9 )
		goto st82;
	goto st0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr118;
	goto st0;
tr118:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st83;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
#line 921 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr119;
	goto st0;
tr119:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 931 "command.cpp"
	if ( (*p) == 58 )
		goto st85;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr121;
	goto st0;
tr121:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
#line 948 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr122;
	goto st0;
tr122:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st87;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
#line 958 "command.cpp"
	if ( (*p) == 58 )
		goto st88;
	goto st0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr124;
	goto st0;
tr124:
#line 145 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st89;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
#line 975 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr125;
	goto st0;
tr125:
#line 145 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st90;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
#line 985 "command.cpp"
	if ( (*p) == 35 )
		goto tr126;
	goto st0;
tr97:
#line 185 "command.rl"
	{stcmd=(*p);}
	goto st91;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
#line 995 "command.cpp"
	if ( (*p) == 32 )
		goto st92;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr128;
	} else if ( (*p) >= 9 )
		goto st92;
	goto st0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr128;
	goto st0;
tr128:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st93;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
#line 1017 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr129;
	goto st0;
tr129:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st94;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
#line 1027 "command.cpp"
	switch( (*p) ) {
		case 47: goto st95;
		case 58: goto st95;
	}
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr131;
	goto st0;
tr131:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st96;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
#line 1046 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr132;
	goto st0;
tr132:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 183 "command.rl"
	{deg=deg*3600+min*60;}
	goto st97;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
#line 1057 "command.cpp"
	switch( (*p) ) {
		case 46: goto st98;
		case 47: goto st100;
		case 58: goto st100;
	}
	goto st0;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr135;
	goto st0;
tr135:
#line 178 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st99;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
#line 1077 "command.cpp"
	if ( (*p) == 35 )
		goto tr136;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr137;
	goto st0;
tr137:
#line 145 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st101;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
#line 1094 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr138;
	goto st0;
tr138:
#line 145 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 184 "command.rl"
	{deg+=sec;}
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
#line 1105 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr136;
		case 46: goto st103;
	}
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
		goto tr136;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st99;
	goto st0;
tr98:
#line 185 "command.rl"
	{stcmd=(*p);}
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
#line 1133 "command.cpp"
	switch( (*p) ) {
		case 32: goto st106;
		case 43: goto st107;
		case 45: goto tr144;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr145;
	} else if ( (*p) >= 9 )
		goto st106;
	goto st0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	switch( (*p) ) {
		case 32: goto st107;
		case 43: goto st107;
		case 45: goto tr144;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr145;
	} else if ( (*p) >= 9 )
		goto st107;
	goto st0;
tr144:
#line 148 "command.rl"
	{ neg=-1;}
	goto st107;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
#line 1166 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr145;
	goto st0;
tr145:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st108;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
#line 1176 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr146;
	goto st0;
tr146:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 1186 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr148;
		} else if ( (*p) >= 33 )
			goto st110;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st110;
		} else if ( (*p) >= 91 )
			goto st110;
	} else
		goto st110;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr149;
	goto st0;
tr149:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st111;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
#line 1215 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr150;
	goto st0;
tr150:
#line 144 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 183 "command.rl"
	{deg=deg*3600+min*60;}
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1226 "command.cpp"
	if ( (*p) == 35 )
		goto tr151;
	goto st100;
tr151:
#line 180 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st117;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
#line 1238 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr154;
		case 35: goto st1;
		case 58: goto st2;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr137;
	goto st0;
tr148:
#line 143 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st113;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
#line 1253 "command.cpp"
	if ( (*p) < 58 ) {
		if ( 33 <= (*p) && (*p) <= 47 )
			goto st110;
	} else if ( (*p) > 64 ) {
		if ( (*p) > 96 ) {
			if ( 123 <= (*p) && (*p) <= 126 )
				goto st110;
		} else if ( (*p) >= 91 )
			goto st110;
	} else
		goto st110;
	goto st0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( (*p) == 72 )
		goto st115;
	goto st0;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	if ( (*p) == 35 )
		goto tr153;
	goto st0;
	}
	_test_eof116: cs = 116; goto _test_eof; 
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
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 249 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
};
