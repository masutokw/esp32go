
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
	int ip3 =0;
	int ip2 =0;
    tmessage[0]=0;
    response[0]=0;
	int pulse=0;
	int focus_counter=0;
    
#line 2 "command.cpp"
static const int command_start = 146;
static const int command_first_final = 146;
static const int command_error = 0;

static const int command_en_main = 146;


#line 141 "command.rl"





    
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
tr18:
#line 215 "command.rl"
	{;}
	goto st146;
tr19:
#line 213 "command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	goto st146;
tr20:
#line 214 "command.rl"
	{telescope->track=1;}
	goto st146;
tr22:
#line 179 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	goto st146;
tr24:
#line 216 "command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	goto st146;
tr33:
#line 204 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed_low);}
	goto st146;
tr35:
#line 202 "command.rl"
	{gotofocuser(focus_motor.max_steps,focuspeed);}
	goto st146;
tr36:
#line 203 "command.rl"
	{gotofocuser(0,focuspeed_low);}
	goto st146;
tr38:
#line 201 "command.rl"
	{gotofocuser(0,focuspeed);}
	goto st146;
tr46:
#line 206 "command.rl"
	{gotofocuser(focus_counter);}
	goto st146;
tr47:
#line 210 "command.rl"
	{sprintf(tmessage,"%d#",focus_motor.state<stop);APPEND;}
	goto st146;
tr57:
#line 208 "command.rl"
	{focus_motor.position=focus_motor.target=focus_counter;}
	goto st146;
tr65:
#line 205 "command.rl"
	{gotofocuser(focus_motor.position+(focus_counter*neg));}
	goto st146;
tr66:
#line 207 "command.rl"
	{stopfocuser();}
	goto st146;
tr67:
#line 209 "command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	goto st146;
tr85:
#line 169 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	goto st146;
tr86:
#line 172 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	goto st146;
tr87:
#line 166 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	goto st146;
tr88:
#line 197 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	goto st146;
tr89:
#line 186 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	goto st146;
tr90:
#line 173 "command.rl"
	{ lxprintsite();}
	goto st146;
tr91:
#line 164 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	goto st146;
tr92:
#line 187 "command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
	goto st146;
tr93:
#line 200 "command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	goto st146;
tr99:
#line 219 "command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
	goto st146;
tr100:
#line 223 "command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	goto st146;
tr101:
#line 220 "command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
	goto st146;
tr102:
#line 221 "command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	goto st146;
tr103:
#line 222 "command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
	goto st146;
tr104:
#line 168 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	goto st146;
tr105:
#line 199 "command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	goto st146;
tr106:
#line 171 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	goto st146;
tr107:
#line 175 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	goto st146;
tr108:
#line 218 "command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
	goto st146;
tr109:
#line 170 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	goto st146;
tr110:
#line 176 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	goto st146;
tr116:
#line 224 "command.rl"
	{setwifipad(ip3,ip2);}
	goto st146;
tr123:
#line 158 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
	goto st146;
tr124:
#line 156 "command.rl"
	{mount_move(telescope,stcmd);}
	goto st146;
tr130:
#line 157 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	goto st146;
tr131:
#line 162 "command.rl"
	{mount_stop(telescope,stcmd);}
	goto st146;
tr134:
#line 163 "command.rl"
	{select_rate(telescope,stcmd); }
	goto st146;
tr150:
#line 194 "command.rl"
	{set_date(min,deg,sec);}
	goto st146;
tr155:
#line 196 "command.rl"
	{ telescope->time_zone=-deg*neg;}
#line 174 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	goto st146;
tr168:
#line 198 "command.rl"
	{set_time(deg,min,sec);}
	goto st146;
tr178:
#line 188 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st146;
tr198:
#line 211 "command.rl"
	{mount_goto_home(telescope);}
	goto st146;
tr200:
#line 212 "command.rl"
	{mount_home_set(telescope);}
	goto st146;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
#line 177 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr201;
		case 35: goto st1;
		case 58: goto st2;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr201:
#line 195 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 193 "command.cpp"
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
		case 104: goto st142;
		case 112: goto st144;
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
		goto tr18;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 35 )
		goto tr19;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 35 )
		goto tr20;
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
		case 35: goto tr22;
		case 82: goto st9;
	}
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 35 )
		goto tr22;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 35 )
		goto tr24;
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
		case 35: goto tr33;
		case 43: goto st13;
	}
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 35 )
		goto tr35;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 35: goto tr36;
		case 45: goto st15;
	}
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 35 )
		goto tr38;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 43: goto st17;
		case 45: goto tr40;
	}
	goto st0;
tr40:
#line 155 "command.rl"
	{ neg=-1;}
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 340 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr41;
	goto st0;
tr41:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 350 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr42;
	goto st0;
tr42:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 360 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr43;
	goto st0;
tr43:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 370 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr44;
	goto st0;
tr44:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 380 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr45;
	goto st0;
tr45:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 390 "command.cpp"
	if ( (*p) == 35 )
		goto tr46;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 35 )
		goto tr47;
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
		case 45: goto tr51;
	}
	goto st0;
tr51:
#line 155 "command.rl"
	{ neg=-1;}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 430 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr52;
	goto st0;
tr52:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 440 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr53;
	goto st0;
tr53:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 450 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr54;
	goto st0;
tr54:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 460 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr55;
	goto st0;
tr55:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 470 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr56;
	goto st0;
tr56:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 480 "command.cpp"
	if ( (*p) == 35 )
		goto tr57;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 43: goto st34;
		case 45: goto tr59;
	}
	goto st0;
tr59:
#line 155 "command.rl"
	{ neg=-1;}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 499 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr60;
	goto st0;
tr60:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 509 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr61;
	goto st0;
tr61:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 519 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr62;
	goto st0;
tr62:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 529 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr63;
	goto st0;
tr63:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 539 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr64;
	goto st0;
tr64:
#line 154 "command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 549 "command.cpp"
	if ( (*p) == 35 )
		goto tr65;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) == 35 )
		goto tr66;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 35 )
		goto tr67;
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
		goto tr85;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 35 )
		goto tr86;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 35 )
		goto tr87;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 35 )
		goto tr88;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 35 )
		goto tr89;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 35 )
		goto tr90;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 35 )
		goto tr91;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 35 )
		goto tr92;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 35 )
		goto tr93;
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
		goto tr99;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 35 )
		goto tr100;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 35 )
		goto tr101;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 35 )
		goto tr102;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 35 )
		goto tr103;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 35 )
		goto tr104;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 35 )
		goto tr105;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 35 )
		goto tr106;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 35 )
		goto tr107;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 35 )
		goto tr108;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 35 )
		goto tr109;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 35 )
		goto tr110;
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
		goto tr112;
	goto st0;
tr112:
#line 152 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 770 "command.cpp"
	if ( (*p) == 46 )
		goto st68;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr114;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr115;
	goto st0;
tr115:
#line 153 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 789 "command.cpp"
	if ( (*p) == 35 )
		goto tr116;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr117;
	goto st0;
tr117:
#line 153 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 801 "command.cpp"
	if ( (*p) == 35 )
		goto tr116;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr118;
	goto st0;
tr118:
#line 153 "command.rl"
	{ADD_DIGIT(ip2,(*p));}
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 813 "command.cpp"
	if ( (*p) == 35 )
		goto tr116;
	goto st0;
tr114:
#line 152 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 823 "command.cpp"
	if ( (*p) == 46 )
		goto st68;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr119;
	goto st0;
tr119:
#line 152 "command.rl"
	{ADD_DIGIT(ip3,(*p));}
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 835 "command.cpp"
	if ( (*p) == 46 )
		goto st68;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	switch( (*p) ) {
		case 83: goto st75;
		case 101: goto tr121;
		case 103: goto st77;
		case 104: goto tr121;
		case 110: goto tr121;
		case 115: goto tr121;
		case 119: goto tr121;
	}
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( (*p) == 35 )
		goto tr123;
	goto st0;
tr121:
#line 193 "command.rl"
	{stcmd=(*p);}
	goto st76;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
#line 866 "command.cpp"
	if ( (*p) == 35 )
		goto tr124;
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	switch( (*p) ) {
		case 101: goto tr125;
		case 110: goto tr125;
		case 115: goto tr125;
		case 119: goto tr125;
	}
	goto st0;
tr125:
#line 193 "command.rl"
	{stcmd=(*p);}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
#line 887 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr126;
	goto st0;
tr126:
#line 151 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st79;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
#line 897 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr127;
	goto st0;
tr127:
#line 151 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 907 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr128;
	goto st0;
tr128:
#line 151 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
#line 917 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr129;
	goto st0;
tr129:
#line 151 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	goto st82;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
#line 927 "command.cpp"
	if ( (*p) == 35 )
		goto tr130;
	goto st0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	switch( (*p) ) {
		case 35: goto tr131;
		case 101: goto tr132;
		case 110: goto tr132;
		case 115: goto tr132;
		case 119: goto tr132;
	}
	goto st0;
tr132:
#line 193 "command.rl"
	{stcmd=(*p);}
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 949 "command.cpp"
	if ( (*p) == 35 )
		goto tr131;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 67: goto tr133;
		case 71: goto tr133;
		case 77: goto tr133;
		case 83: goto tr133;
	}
	goto st0;
tr133:
#line 193 "command.rl"
	{stcmd=(*p);}
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
#line 970 "command.cpp"
	if ( (*p) == 35 )
		goto tr134;
	if ( 48 <= (*p) && (*p) <= 52 )
		goto st87;
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	if ( (*p) == 35 )
		goto tr134;
	goto st0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	switch( (*p) ) {
		case 67: goto st89;
		case 71: goto st99;
		case 76: goto st106;
		case 83: goto tr139;
		case 97: goto tr140;
		case 100: goto tr140;
		case 103: goto tr140;
		case 114: goto tr139;
		case 116: goto tr140;
		case 122: goto tr140;
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
			goto tr142;
	} else if ( (*p) >= 9 )
		goto st90;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr142;
	goto st0;
tr142:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st91;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
#line 1025 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr143;
	goto st0;
tr143:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st92;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
#line 1035 "command.cpp"
	if ( (*p) == 47 )
		goto st93;
	goto st0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr145;
	goto st0;
tr145:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st94;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
#line 1052 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr146;
	goto st0;
tr146:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st95;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
#line 1062 "command.cpp"
	if ( (*p) == 47 )
		goto st96;
	goto st0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr148;
	goto st0;
tr148:
#line 150 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st97;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
#line 1079 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr149;
	goto st0;
tr149:
#line 150 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st98;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
#line 1089 "command.cpp"
	if ( (*p) == 35 )
		goto tr150;
	goto st0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	switch( (*p) ) {
		case 32: goto st100;
		case 43: goto st101;
		case 45: goto tr153;
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
		case 45: goto tr153;
	}
	goto st0;
tr153:
#line 155 "command.rl"
	{ neg=-1;}
	goto st101;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
#line 1120 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr154;
	goto st0;
tr154:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
#line 1130 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr155;
		case 44: goto st103;
		case 46: goto st103;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr157;
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
		goto tr155;
	goto st0;
tr157:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
#line 1159 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr155;
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
			goto tr160;
	} else if ( (*p) >= 9 )
		goto st107;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr160;
	goto st0;
tr160:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st108;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
#line 1191 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr161;
	goto st0;
tr161:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 1201 "command.cpp"
	if ( (*p) == 58 )
		goto st110;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr163;
	goto st0;
tr163:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st111;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
#line 1218 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr164;
	goto st0;
tr164:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1228 "command.cpp"
	if ( (*p) == 58 )
		goto st113;
	goto st0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr166;
	goto st0;
tr166:
#line 150 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st114;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
#line 1245 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr167;
	goto st0;
tr167:
#line 150 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st115;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
#line 1255 "command.cpp"
	if ( (*p) == 35 )
		goto tr168;
	goto st0;
tr139:
#line 193 "command.rl"
	{stcmd=(*p);}
	goto st116;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
#line 1265 "command.cpp"
	if ( (*p) == 32 )
		goto st117;
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 50 )
			goto tr170;
	} else if ( (*p) >= 9 )
		goto st117;
	goto st0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	if ( 48 <= (*p) && (*p) <= 50 )
		goto tr170;
	goto st0;
tr170:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
#line 1287 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr171;
	goto st0;
tr171:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st119;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
#line 1297 "command.cpp"
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
		goto tr173;
	goto st0;
tr173:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st121;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
#line 1316 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr174;
	goto st0;
tr174:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 191 "command.rl"
	{deg=deg*3600+min*60;}
	goto st122;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
#line 1327 "command.cpp"
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
		goto tr177;
	goto st0;
tr177:
#line 185 "command.rl"
	{deg+=((*p)-'0')*6;}
	goto st124;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
#line 1347 "command.cpp"
	if ( (*p) == 35 )
		goto tr178;
	goto st0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr179;
	goto st0;
tr179:
#line 150 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	goto st126;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
#line 1364 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr180;
	goto st0;
tr180:
#line 150 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
#line 192 "command.rl"
	{deg+=sec;}
	goto st127;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
#line 1375 "command.cpp"
	switch( (*p) ) {
		case 35: goto tr178;
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
		case 35: goto tr178;
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
		case 35: goto tr178;
		case 58: goto st124;
	}
	goto st0;
tr140:
#line 193 "command.rl"
	{stcmd=(*p);}
	goto st131;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
#line 1416 "command.cpp"
	switch( (*p) ) {
		case 32: goto st132;
		case 43: goto st133;
		case 45: goto tr187;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr188;
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
		case 45: goto tr187;
	}
	if ( (*p) > 13 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr188;
	} else if ( (*p) >= 9 )
		goto st133;
	goto st0;
tr187:
#line 155 "command.rl"
	{ neg=-1;}
	goto st133;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
#line 1449 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr188;
	goto st0;
tr188:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st134;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
#line 1459 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr190;
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
		goto tr191;
	goto st0;
tr191:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	goto st136;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
#line 1488 "command.cpp"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr192;
	goto st0;
tr192:
#line 149 "command.rl"
	{ADD_DIGIT(min,(*p)); }
#line 191 "command.rl"
	{deg=deg*3600+min*60;}
	goto st137;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
#line 1499 "command.cpp"
	if ( (*p) == 35 )
		goto tr193;
	goto st125;
tr193:
#line 188 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	goto st147;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
#line 1511 "command.cpp"
	switch( (*p) ) {
		case 6: goto tr201;
		case 35: goto st1;
		case 58: goto st2;
	}
	if ( 48 <= (*p) && (*p) <= 53 )
		goto tr179;
	goto st0;
tr190:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st138;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
#line 1526 "command.cpp"
	if ( (*p) < 58 ) {
		if ( (*p) > 47 ) {
			if ( 48 <= (*p) && (*p) <= 57 )
				goto tr194;
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
tr194:
#line 148 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	goto st139;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
#line 1548 "command.cpp"
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
		goto tr195;
	goto st0;
tr195:
#line 225 "command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		}
	goto st141;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
#line 1581 "command.cpp"
	if ( (*p) == 35 )
		goto st146;
	goto st0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	if ( (*p) == 80 )
		goto st143;
	goto st0;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
	if ( (*p) == 35 )
		goto tr198;
	goto st0;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
	if ( (*p) == 72 )
		goto st145;
	goto st0;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
	if ( (*p) == 35 )
		goto tr200;
	goto st0;
	}
	_test_eof146: cs = 146; goto _test_eof; 
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
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 305 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
};



