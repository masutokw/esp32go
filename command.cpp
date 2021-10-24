
#line 1 "command.rl"
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

    
#line 2 "command.cpp"
static const char _command_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 1, 13, 1, 14, 1, 
	15, 1, 17, 1, 18, 1, 19, 1, 
	20, 1, 21, 1, 22, 1, 25, 1, 
	26, 1, 27, 1, 29, 1, 30, 2, 
	1, 23, 2, 2, 24, 2, 28, 16
	
};

static const unsigned char _command_key_offsets[] = {
	0, 0, 6, 7, 9, 10, 23, 24, 
	25, 26, 27, 28, 29, 30, 31, 32, 
	33, 34, 35, 40, 41, 42, 47, 48, 
	52, 55, 56, 66, 71, 73, 75, 76, 
	78, 80, 81, 83, 85, 86, 91, 93, 
	95, 99, 101, 102, 104, 109, 111, 113, 
	114, 116, 118, 119, 121, 123, 124, 129, 
	131, 133, 135, 137, 139, 142, 144, 145, 
	147, 149, 156, 163, 165, 167, 177, 179, 
	181, 182, 190, 193, 194
};

static const char _command_trans_keys[] = {
	67, 71, 77, 81, 82, 83, 77, 35, 
	82, 35, 65, 67, 68, 71, 76, 77, 
	82, 83, 90, 100, 103, 114, 116, 35, 
	35, 35, 35, 35, 35, 35, 35, 35, 
	35, 35, 35, 83, 101, 110, 115, 119, 
	35, 35, 35, 101, 110, 115, 119, 35, 
	67, 71, 77, 83, 35, 48, 52, 35, 
	67, 71, 76, 83, 97, 100, 103, 114, 
	116, 122, 32, 9, 13, 48, 57, 48, 
	57, 48, 57, 47, 48, 57, 48, 57, 
	47, 48, 57, 48, 57, 35, 32, 43, 
	45, 9, 13, 43, 45, 48, 57, 35, 
	46, 48, 57, 48, 57, 35, 35, 46, 
	32, 9, 13, 48, 57, 48, 57, 48, 
	57, 58, 48, 57, 48, 57, 58, 48, 
	57, 48, 57, 35, 32, 9, 13, 48, 
	50, 48, 50, 48, 57, 47, 58, 48, 
	53, 48, 57, 46, 47, 58, 48, 57, 
	35, 48, 53, 48, 57, 32, 43, 45, 
	9, 13, 48, 57, 32, 43, 45, 9, 
	13, 48, 57, 48, 57, 48, 57, 33, 
	47, 48, 57, 58, 64, 91, 96, 123, 
	126, 48, 53, 48, 57, 35, 33, 47, 
	58, 64, 91, 96, 123, 126, 6, 35, 
	58, 58, 58, 48, 53, 0
};

static const char _command_single_lengths[] = {
	0, 6, 1, 2, 1, 13, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 5, 1, 1, 5, 1, 4, 
	1, 1, 10, 1, 0, 0, 1, 0, 
	0, 1, 0, 0, 1, 3, 2, 0, 
	2, 0, 1, 2, 1, 0, 0, 1, 
	0, 0, 1, 0, 0, 1, 1, 0, 
	0, 2, 0, 0, 3, 0, 1, 0, 
	0, 3, 3, 0, 0, 0, 0, 0, 
	1, 0, 3, 1, 1
};

static const char _command_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	1, 0, 0, 2, 1, 1, 0, 1, 
	1, 0, 1, 1, 0, 1, 0, 1, 
	1, 1, 0, 0, 2, 1, 1, 0, 
	1, 1, 0, 1, 1, 0, 2, 1, 
	1, 0, 1, 1, 0, 1, 0, 1, 
	1, 2, 2, 1, 1, 5, 1, 1, 
	0, 4, 0, 0, 1
};

static const short _command_index_offsets[] = {
	0, 0, 7, 9, 12, 14, 28, 30, 
	32, 34, 36, 38, 40, 42, 44, 46, 
	48, 50, 52, 58, 60, 62, 68, 70, 
	75, 78, 80, 91, 95, 97, 99, 101, 
	103, 105, 107, 109, 111, 113, 118, 121, 
	123, 127, 129, 131, 134, 138, 140, 142, 
	144, 146, 148, 150, 152, 154, 156, 160, 
	162, 164, 167, 169, 171, 175, 177, 179, 
	181, 183, 189, 195, 197, 199, 205, 207, 
	209, 211, 216, 220, 222
};

static const char _command_indicies[] = {
	0, 2, 3, 4, 5, 6, 1, 7, 
	1, 8, 9, 1, 8, 1, 10, 11, 
	12, 13, 14, 15, 16, 14, 17, 18, 
	19, 20, 21, 1, 22, 1, 23, 1, 
	24, 1, 25, 1, 26, 1, 27, 1, 
	28, 1, 29, 1, 30, 1, 31, 1, 
	32, 1, 33, 1, 34, 35, 35, 35, 
	35, 1, 36, 1, 37, 1, 38, 39, 
	39, 39, 39, 1, 38, 1, 40, 40, 
	40, 40, 1, 41, 42, 1, 41, 1, 
	43, 44, 45, 46, 47, 47, 47, 46, 
	47, 47, 1, 48, 48, 49, 1, 49, 
	1, 50, 1, 51, 1, 52, 1, 53, 
	1, 54, 1, 55, 1, 56, 1, 57, 
	1, 58, 59, 60, 58, 1, 59, 60, 
	1, 61, 1, 62, 63, 64, 1, 65, 
	1, 62, 1, 62, 63, 1, 66, 66, 
	67, 1, 67, 1, 68, 1, 69, 1, 
	70, 1, 71, 1, 72, 1, 73, 1, 
	74, 1, 75, 1, 76, 76, 77, 1, 
	77, 1, 78, 1, 79, 79, 1, 80, 
	1, 81, 1, 82, 83, 83, 1, 84, 
	1, 85, 1, 86, 1, 87, 1, 88, 
	89, 90, 88, 91, 1, 89, 89, 90, 
	89, 91, 1, 91, 1, 92, 1, 93, 
	94, 93, 93, 93, 1, 95, 1, 96, 
	1, 97, 83, 93, 93, 93, 93, 1, 
	98, 99, 100, 1, 100, 1, 100, 86, 
	1, 0
};

static const char _command_trans_targs[] = {
	2, 0, 5, 18, 21, 23, 26, 3, 
	75, 4, 6, 7, 8, 9, 10, 11, 
	12, 13, 14, 15, 16, 17, 75, 75, 
	75, 75, 75, 75, 75, 75, 75, 75, 
	75, 75, 19, 20, 75, 75, 75, 22, 
	24, 75, 25, 27, 37, 44, 54, 65, 
	28, 29, 30, 31, 32, 33, 34, 35, 
	36, 75, 38, 39, 39, 40, 75, 41, 
	43, 42, 45, 46, 47, 48, 49, 50, 
	51, 52, 53, 75, 55, 56, 57, 58, 
	59, 60, 61, 63, 62, 75, 64, 62, 
	66, 67, 67, 68, 69, 70, 73, 71, 
	72, 76, 75, 75, 1
};

static const char _command_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	37, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 23, 29, 
	19, 51, 41, 31, 17, 21, 27, 33, 
	25, 35, 0, 45, 11, 9, 13, 45, 
	45, 15, 0, 0, 0, 0, 45, 45, 
	0, 1, 1, 0, 3, 3, 0, 5, 
	5, 47, 0, 0, 7, 1, 61, 0, 
	1, 0, 0, 1, 1, 0, 3, 3, 
	0, 5, 5, 53, 0, 1, 1, 0, 
	3, 55, 0, 0, 39, 43, 5, 58, 
	0, 0, 7, 1, 1, 0, 1, 3, 
	55, 43, 49, 0, 0
};

static const int command_start = 74;
static const int command_first_final = 74;
static const int command_error = 0;

static const int command_en_main = 74;


#line 130 "command.rl"





    
#line 164 "command.cpp"
	{
	cs = command_start;
	}

#line 167 "command.cpp"
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
#line 137 "command.rl"
	{ADD_DIGIT(deg,(*p)); }
	break;
	case 1:
#line 138 "command.rl"
	{ADD_DIGIT(min,(*p)); }
	break;
	case 2:
#line 139 "command.rl"
	{ADD_DIGIT(sec,(*p)); }
	break;
	case 3:
#line 140 "command.rl"
	{ neg=-1;}
	break;
	case 4:
#line 141 "command.rl"
	{mount_move(telescope,stcmd);}
	break;
	case 5:
#line 142 "command.rl"
	{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD); sprintf(tmessage,"0");APPEND;}
	break;
	case 6:
#line 143 "command.rl"
	{mount_stop(telescope,stcmd);}
	break;
	case 7:
#line 144 "command.rl"
	{select_rate(telescope,stcmd); }
	break;
	case 8:
#line 145 "command.rl"
	{ lxprintra1(tmessage, st_current.ra); APPEND;}
	break;
	case 9:
#line 146 "command.rl"
	{lxprintde1(tmessage, st_current.dec); APPEND;}
	break;
	case 10:
#line 147 "command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	break;
	case 11:
#line 148 "command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	break;
	case 12:
#line 149 "command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	break;
	case 13:
#line 150 "command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	break;
	case 14:
#line 151 "command.rl"
	{lxprintdate1(tmessage);APPEND;}
	break;
	case 15:
#line 152 "command.rl"
	{ lxprintsite();}
	break;
	case 16:
#line 153 "command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	break;
	case 17:
#line 154 "command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	break;
	case 18:
#line 155 "command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	break;
	case 19:
#line 157 "command.rl"
	{ align_sync_all(telescope,mount.ra_target,mount.dec_target); sprintf(tmessage,"sync#");APPEND;}
	break;
	case 20:
#line 158 "command.rl"
	{deg+=((*p)-'0')*6;}
	break;
	case 21:
#line 159 "command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	break;
	case 22:
#line 160 "command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	break;
	case 23:
#line 163 "command.rl"
	{deg=deg*3600+min*60;}
	break;
	case 24:
#line 164 "command.rl"
	{deg+=sec;}
	break;
	case 25:
#line 165 "command.rl"
	{stcmd=(*p);}
	break;
	case 26:
#line 166 "command.rl"
	{set_date(min,deg,sec);}
	break;
	case 27:
#line 167 "command.rl"
	{sprintf(tmessage,"A"); APPEND; }
	break;
	case 28:
#line 168 "command.rl"
	{ telescope->time_zone=deg;}
	break;
	case 29:
#line 169 "command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	break;
	case 30:
#line 170 "command.rl"
	{set_time(deg,min,sec);}
	break;
#line 335 "command.cpp"
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

#line 208 "command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
};



