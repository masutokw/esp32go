
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

char response [200];
char tmessage[50];
extern c_star st_now, st_target, st_current;
extern char volatile sync_target;
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
    telescope->is_tracking = TRUE;
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

    
#line 2 "command.cpp"
static const char _command_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 1, 13, 1, 14, 1, 
	15, 1, 16, 1, 17, 1, 19, 1, 
	20, 1, 21, 1, 22, 1, 23, 1, 
	24, 1, 27, 1, 28, 1, 29, 1, 
	31, 1, 32, 2, 1, 25, 2, 2, 
	26, 2, 30, 18
};

static const unsigned char _command_key_offsets[] = {
	0, 0, 1, 7, 8, 10, 11, 24, 
	25, 26, 27, 28, 29, 30, 31, 32, 
	33, 34, 35, 36, 42, 43, 44, 48, 
	50, 52, 54, 56, 57, 62, 63, 67, 
	70, 71, 81, 86, 88, 90, 91, 93, 
	95, 96, 98, 100, 101, 106, 108, 110, 
	114, 116, 117, 119, 124, 126, 128, 129, 
	131, 133, 134, 136, 138, 139, 144, 146, 
	148, 150, 152, 154, 157, 159, 160, 162, 
	164, 166, 168, 171, 178, 185, 187, 189, 
	199, 201, 203, 204, 212, 215
};

static const char _command_trans_keys[] = {
	58, 67, 71, 77, 81, 82, 83, 77, 
	35, 82, 35, 65, 67, 68, 71, 76, 
	77, 82, 83, 90, 100, 103, 114, 116, 
	35, 35, 35, 35, 35, 35, 35, 35, 
	35, 35, 35, 35, 83, 101, 103, 110, 
	115, 119, 35, 35, 101, 110, 115, 119, 
	48, 57, 48, 57, 48, 57, 48, 57, 
	35, 35, 101, 110, 115, 119, 35, 67, 
	71, 77, 83, 35, 48, 52, 35, 67, 
	71, 76, 83, 97, 100, 103, 114, 116, 
	122, 32, 9, 13, 48, 57, 48, 57, 
	48, 57, 47, 48, 57, 48, 57, 47, 
	48, 57, 48, 57, 35, 32, 43, 45, 
	9, 13, 43, 45, 48, 57, 35, 46, 
	48, 57, 48, 57, 35, 35, 46, 32, 
	9, 13, 48, 57, 48, 57, 48, 57, 
	58, 48, 57, 48, 57, 58, 48, 57, 
	48, 57, 35, 32, 9, 13, 48, 50, 
	48, 50, 48, 57, 47, 58, 48, 53, 
	48, 57, 46, 47, 58, 48, 57, 35, 
	48, 53, 48, 57, 35, 46, 48, 57, 
	35, 48, 57, 32, 43, 45, 9, 13, 
	48, 57, 32, 43, 45, 9, 13, 48, 
	57, 48, 57, 48, 57, 33, 47, 48, 
	57, 58, 64, 91, 96, 123, 126, 48, 
	53, 48, 57, 35, 33, 47, 58, 64, 
	91, 96, 123, 126, 6, 35, 58, 6, 
	35, 58, 48, 53, 0
};

static const char _command_single_lengths[] = {
	0, 1, 6, 1, 2, 1, 13, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 6, 1, 1, 4, 0, 
	0, 0, 0, 1, 5, 1, 4, 1, 
	1, 10, 1, 0, 0, 1, 0, 0, 
	1, 0, 0, 1, 3, 2, 0, 2, 
	0, 1, 2, 1, 0, 0, 1, 0, 
	0, 1, 0, 0, 1, 1, 0, 0, 
	2, 0, 0, 3, 0, 1, 0, 0, 
	2, 0, 1, 3, 3, 0, 0, 0, 
	0, 0, 1, 0, 3, 3
};

static const char _command_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 0, 0, 0, 0, 1, 
	0, 0, 2, 1, 1, 0, 1, 1, 
	0, 1, 1, 0, 1, 0, 1, 1, 
	1, 0, 0, 2, 1, 1, 0, 1, 
	1, 0, 1, 1, 0, 2, 1, 1, 
	0, 1, 1, 0, 1, 0, 1, 1, 
	0, 1, 1, 2, 2, 1, 1, 5, 
	1, 1, 0, 4, 0, 1
};

static const short _command_index_offsets[] = {
	0, 0, 2, 9, 11, 14, 16, 30, 
	32, 34, 36, 38, 40, 42, 44, 46, 
	48, 50, 52, 54, 61, 63, 65, 70, 
	72, 74, 76, 78, 80, 86, 88, 93, 
	96, 98, 109, 113, 115, 117, 119, 121, 
	123, 125, 127, 129, 131, 136, 139, 141, 
	145, 147, 149, 152, 156, 158, 160, 162, 
	164, 166, 168, 170, 172, 174, 178, 180, 
	182, 185, 187, 189, 193, 195, 197, 199, 
	201, 204, 206, 209, 215, 221, 223, 225, 
	231, 233, 235, 237, 242, 246
};

static const char _command_indicies[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 
	1, 8, 1, 9, 10, 1, 9, 1, 
	11, 12, 13, 14, 15, 16, 17, 15, 
	18, 19, 20, 21, 22, 1, 23, 1, 
	24, 1, 25, 1, 26, 1, 27, 1, 
	28, 1, 29, 1, 30, 1, 31, 1, 
	32, 1, 33, 1, 34, 1, 35, 36, 
	37, 36, 36, 36, 1, 38, 1, 39, 
	1, 40, 40, 40, 40, 1, 41, 1, 
	42, 1, 43, 1, 44, 1, 45, 1, 
	46, 47, 47, 47, 47, 1, 46, 1, 
	48, 48, 48, 48, 1, 49, 50, 1, 
	49, 1, 51, 52, 53, 54, 55, 55, 
	55, 54, 55, 55, 1, 56, 56, 57, 
	1, 57, 1, 58, 1, 59, 1, 60, 
	1, 61, 1, 62, 1, 63, 1, 64, 
	1, 65, 1, 66, 67, 68, 66, 1, 
	67, 68, 1, 69, 1, 70, 71, 72, 
	1, 73, 1, 70, 1, 70, 71, 1, 
	74, 74, 75, 1, 75, 1, 76, 1, 
	77, 1, 78, 1, 79, 1, 80, 1, 
	81, 1, 82, 1, 83, 1, 84, 84, 
	85, 1, 85, 1, 86, 1, 87, 87, 
	1, 88, 1, 89, 1, 90, 91, 91, 
	1, 92, 1, 93, 1, 94, 1, 95, 
	1, 93, 96, 1, 97, 1, 93, 98, 
	1, 99, 100, 101, 99, 102, 1, 100, 
	100, 101, 100, 102, 1, 102, 1, 103, 
	1, 104, 105, 104, 104, 104, 1, 106, 
	1, 107, 1, 108, 91, 104, 104, 104, 
	104, 1, 109, 110, 0, 1, 109, 110, 
	0, 94, 1, 0
};

static const char _command_trans_targs[] = {
	2, 0, 3, 6, 19, 28, 30, 33, 
	4, 84, 5, 7, 8, 9, 10, 11, 
	12, 13, 14, 15, 16, 17, 18, 84, 
	84, 84, 84, 84, 84, 84, 84, 84, 
	84, 84, 84, 20, 21, 22, 84, 84, 
	23, 24, 25, 26, 27, 84, 84, 29, 
	31, 84, 32, 34, 44, 51, 61, 75, 
	35, 36, 37, 38, 39, 40, 41, 42, 
	43, 84, 45, 46, 46, 47, 84, 48, 
	50, 49, 52, 53, 54, 55, 56, 57, 
	58, 59, 60, 84, 62, 63, 64, 65, 
	66, 67, 68, 70, 69, 84, 71, 72, 
	73, 74, 69, 76, 77, 77, 78, 79, 
	80, 83, 81, 82, 85, 1, 1
};

static const char _command_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 41, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 27, 
	33, 23, 55, 45, 35, 21, 25, 31, 
	37, 29, 39, 0, 49, 0, 15, 11, 
	49, 7, 7, 7, 7, 13, 17, 49, 
	49, 19, 0, 0, 0, 0, 49, 49, 
	0, 1, 1, 0, 3, 3, 0, 5, 
	5, 51, 0, 0, 9, 1, 65, 0, 
	1, 0, 0, 1, 1, 0, 3, 3, 
	0, 5, 5, 57, 0, 1, 1, 0, 
	3, 59, 0, 0, 43, 47, 5, 62, 
	0, 0, 0, 0, 0, 9, 1, 1, 
	0, 1, 3, 59, 47, 53, 0
};

static const int command_start = 84;
static const int command_first_final = 84;
static const int command_error = 0;

static const int command_en_main = 84;


#line 132 "command.rl"





    
#line 176 "command.cpp"
	{
	cs = command_start;
	}

#line 179 "command.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _command_trans_keys + _command_key_offsets[cs];
	_trans = _command_index_offsets[cs];

	_klen = _command_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _command_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _command_indicies[_trans];
	cs = _command_trans_targs[_trans];

	if ( _command_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _command_actions + _command_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 139 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	break;
	case 1:
#line 140 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	break;
	case 2:
#line 141 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	break;
	case 3:
#line 142 "command.rl"
	{ADD_DIGIT(pulse,(*p));}
	break;
	case 4:
#line 143 "command.rl"
	{ neg=-1;}
	break;
	case 5:
#line 144 "command.rl"
	{mount_move(telescope,stcmd);}
	break;
	case 6:
#line 145 "command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	break;
	case 7:
#line 146 "command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 sprintf(tmessage,"0");APPEND;}
	break;
	case 8:
#line 150 "command.rl"
	{mount_stop(telescope,stcmd);}
	break;
	case 9:
#line 151 "command.rl"
	{select_rate(telescope,stcmd); }
	break;
	case 10:
#line 152 "command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	break;
	case 11:
#line 154 "command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	break;
	case 12:
#line 156 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	break;
	case 13:
#line 157 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	break;
	case 14:
#line 158 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	break;
	case 15:
#line 159 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	break;
	case 16:
#line 160 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	break;
	case 17:
#line 161 "command.rl"
	{ lxprintsite();}
	break;
	case 18:
#line 162 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	break;
	case 19:
#line 163 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	break;
	case 20:
#line 164 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	break;
	case 21:
#line 167 "command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	break;
	case 22:
#line 173 "command.rl"
	{deg+=((*p)-'0')*6;}
	break;
	case 23:
#line 174 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	break;
	case 24:
#line 175 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	break;
	case 25:
#line 178 "command.rl"
	{deg=deg*3600+min*60;}
	break;
	case 26:
#line 179 "command.rl"
	{deg+=sec;}
	break;
	case 27:
#line 180 "command.rl"
	{stcmd=(*p);}
	break;
	case 28:
#line 181 "command.rl"
	{set_date(min,deg,sec);}
	break;
	case 29:
#line 182 "command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else sprintf(tmessage,"P") ; APPEND; }
	break;
	case 30:
#line 183 "command.rl"
	{ telescope->time_zone=deg;}
	break;
	case 31:
#line 184 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	break;
	case 32:
#line 185 "command.rl"
	{set_time(deg,min,sec);}
	break;
#line 363 "command.cpp"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 222 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
};



