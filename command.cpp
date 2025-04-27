
#line 1 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
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
extern stepper focus_motor,aux_motor,*pmotor;
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
  case 'F':conf_tmc(AUX_FILE);
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
    
#line 223 "/dev/stdout"
static const char _command_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 1, 13, 1, 14, 1, 
	15, 1, 16, 1, 17, 1, 18, 1, 
	19, 1, 20, 1, 22, 1, 23, 1, 
	24, 1, 25, 1, 26, 1, 27, 1, 
	28, 1, 31, 1, 32, 1, 33, 1, 
	34, 1, 36, 1, 37, 1, 38, 1, 
	39, 1, 40, 1, 41, 1, 42, 1, 
	43, 1, 44, 1, 45, 1, 46, 1, 
	47, 1, 48, 1, 49, 1, 50, 1, 
	51, 1, 52, 1, 53, 1, 54, 1, 
	55, 1, 56, 1, 57, 1, 58, 1, 
	59, 1, 60, 1, 61, 1, 62, 1, 
	63, 1, 64, 1, 65, 1, 66, 1, 
	67, 1, 68, 1, 69, 1, 70, 1, 
	71, 1, 72, 1, 73, 2, 1, 29, 
	2, 2, 30, 2, 8, 71, 2, 9, 
	71, 2, 10, 71, 2, 11, 71, 2, 
	12, 71, 2, 13, 71, 2, 14, 71, 
	2, 15, 71, 2, 16, 71, 2, 17, 
	71, 2, 18, 71, 2, 19, 71, 2, 
	20, 71, 2, 22, 71, 2, 23, 71, 
	2, 24, 71, 2, 26, 71, 2, 27, 
	71, 2, 28, 71, 2, 33, 71, 2, 
	35, 21, 2, 36, 71, 2, 37, 71, 
	2, 38, 71, 2, 39, 71, 2, 40, 
	71, 2, 41, 71, 2, 42, 71, 2, 
	43, 71, 2, 44, 71, 2, 45, 71, 
	2, 46, 71, 2, 48, 71, 2, 49, 
	71, 2, 50, 71, 2, 51, 71, 2, 
	52, 71, 2, 53, 71, 2, 54, 71, 
	2, 55, 71, 2, 57, 71, 2, 58, 
	71, 2, 59, 71, 2, 60, 71, 2, 
	61, 71, 2, 62, 71, 2, 63, 71, 
	2, 64, 71, 2, 65, 71, 2, 66, 
	71, 2, 67, 71, 2, 68, 71, 2, 
	69, 71, 3, 35, 21, 71, 6, 13, 
	14, 15, 16, 62, 49, 7, 13, 14, 
	15, 16, 62, 49, 71
};

static const short _command_key_offsets[] = {
	0, 0, 1, 16, 19, 20, 21, 22, 
	23, 25, 26, 27, 36, 38, 39, 41, 
	42, 44, 46, 48, 50, 52, 54, 55, 
	56, 57, 58, 60, 62, 64, 66, 68, 
	70, 71, 73, 75, 77, 79, 81, 83, 
	84, 85, 86, 89, 90, 109, 110, 111, 
	112, 113, 114, 115, 116, 117, 118, 119, 
	124, 125, 126, 127, 128, 129, 130, 131, 
	132, 133, 134, 135, 136, 137, 138, 140, 
	143, 145, 148, 151, 152, 155, 156, 163, 
	164, 165, 169, 171, 173, 175, 177, 178, 
	179, 180, 185, 186, 190, 193, 194, 204, 
	209, 211, 213, 214, 216, 218, 219, 221, 
	223, 224, 229, 231, 233, 238, 240, 241, 
	244, 249, 251, 253, 254, 256, 258, 259, 
	261, 263, 264, 269, 271, 273, 275, 277, 
	279, 282, 284, 285, 287, 289, 293, 295, 
	299, 301, 308, 315, 317, 327, 329, 331, 
	332, 342, 350, 352, 364, 365, 366, 368, 
	384, 388, 389, 390, 391, 393, 395, 396, 
	397, 407, 409, 410, 412, 413, 416, 419, 
	422, 425, 428, 431, 432, 433, 435, 437, 
	440, 443, 446, 449, 452, 455, 456, 459, 
	462, 465, 468, 471, 474, 475, 476, 477, 
	481, 501, 502, 503, 504, 505, 506, 507, 
	508, 509, 510, 511, 517, 518, 519, 520, 
	521, 522, 523, 524, 525, 526, 527, 528, 
	529, 530, 532, 535, 539, 542, 545, 548, 
	549, 553, 555, 563, 564, 565, 570, 573, 
	576, 579, 582, 583, 585, 586, 591, 592, 
	597, 600, 601, 612, 618, 621, 624, 626, 
	629, 632, 634, 637, 640, 641, 647, 650, 
	653, 658, 661, 662, 665, 671, 674, 677, 
	679, 682, 685, 687, 690, 693, 694, 700, 
	703, 706, 709, 712, 715, 719, 722, 723, 
	726, 729, 733, 736, 740, 742, 750, 758, 
	761, 772, 775, 778, 779, 790, 799, 802, 
	815, 816, 818, 819, 826, 827, 828, 829, 
	832, 834, 837, 840, 841, 842, 848, 849, 
	850, 851, 853, 854, 856, 858, 861, 866, 
	869, 874
};

static const char _command_trans_keys[] = {
	58, 65, 67, 68, 70, 71, 73, 77, 
	80, 81, 82, 83, 97, 99, 104, 112, 
	65, 76, 80, 35, 35, 35, 77, 35, 
	82, 35, 35, 43, 45, 65, 66, 76, 
	80, 81, 112, 115, 35, 43, 35, 35, 
	45, 35, 43, 45, 48, 57, 48, 57, 
	48, 57, 48, 57, 48, 57, 35, 35, 
	83, 49, 43, 45, 48, 57, 48, 57, 
	48, 57, 48, 57, 48, 57, 35, 43, 
	45, 48, 57, 48, 57, 48, 57, 48, 
	57, 48, 57, 35, 35, 35, 44, 48, 
	49, 35, 65, 67, 68, 71, 75, 76, 
	77, 82, 83, 84, 86, 90, 99, 100, 
	103, 107, 114, 116, 120, 35, 35, 35, 
	35, 35, 35, 35, 35, 35, 35, 68, 
	70, 78, 80, 84, 35, 35, 35, 35, 
	35, 35, 35, 35, 35, 35, 35, 35, 
	35, 80, 48, 57, 46, 48, 57, 48, 
	57, 35, 48, 57, 35, 48, 57, 35, 
	46, 48, 57, 46, 83, 101, 103, 104, 
	110, 115, 119, 35, 35, 101, 110, 115, 
	119, 48, 57, 48, 57, 48, 57, 48, 
	57, 35, 80, 35, 35, 101, 110, 115, 
	119, 35, 67, 71, 77, 83, 35, 48, 
	52, 35, 67, 71, 76, 83, 97, 100, 
	103, 114, 116, 122, 32, 9, 13, 48, 
	57, 48, 57, 48, 57, 47, 48, 57, 
	48, 57, 47, 48, 57, 48, 57, 35, 
	32, 43, 45, 9, 13, 43, 45, 48, 
	57, 35, 44, 46, 48, 57, 48, 57, 
	35, 35, 44, 46, 32, 9, 13, 48, 
	57, 48, 57, 48, 57, 58, 48, 57, 
	48, 57, 58, 48, 57, 48, 57, 35, 
	32, 9, 13, 48, 50, 48, 50, 48, 
	57, 47, 58, 48, 53, 48, 57, 46, 
	47, 58, 48, 57, 35, 48, 53, 48, 
	57, 35, 44, 46, 58, 48, 57, 35, 
	58, 48, 57, 35, 58, 32, 43, 45, 
	9, 13, 48, 57, 32, 43, 45, 9, 
	13, 48, 57, 48, 57, 33, 47, 48, 
	57, 58, 64, 91, 96, 123, 126, 48, 
	53, 48, 57, 35, 33, 47, 48, 57, 
	58, 64, 91, 96, 123, 126, 33, 47, 
	58, 64, 91, 96, 123, 126, 48, 57, 
	65, 70, 84, 97, 102, 103, 106, 110, 
	119, 122, 115, 116, 35, 35, 35, 58, 
	35, 65, 67, 68, 70, 71, 73, 77, 
	80, 81, 82, 83, 97, 99, 104, 112, 
	35, 65, 76, 80, 35, 35, 35, 35, 
	77, 35, 82, 35, 35, 35, 43, 45, 
	65, 66, 76, 80, 81, 112, 115, 35, 
	43, 35, 35, 45, 35, 35, 43, 45, 
	35, 48, 57, 35, 48, 57, 35, 48, 
	57, 35, 48, 57, 35, 48, 57, 35, 
	35, 35, 83, 35, 49, 35, 43, 45, 
	35, 48, 57, 35, 48, 57, 35, 48, 
	57, 35, 48, 57, 35, 48, 57, 35, 
	35, 43, 45, 35, 48, 57, 35, 48, 
	57, 35, 48, 57, 35, 48, 57, 35, 
	48, 57, 35, 35, 35, 35, 44, 48, 
	49, 35, 65, 67, 68, 71, 75, 76, 
	77, 82, 83, 84, 86, 90, 99, 100, 
	103, 107, 114, 116, 120, 35, 35, 35, 
	35, 35, 35, 35, 35, 35, 35, 35, 
	68, 70, 78, 80, 84, 35, 35, 35, 
	35, 35, 35, 35, 35, 35, 35, 35, 
	35, 35, 35, 80, 35, 48, 57, 35, 
	46, 48, 57, 35, 48, 57, 35, 48, 
	57, 35, 48, 57, 35, 35, 46, 48, 
	57, 35, 46, 35, 83, 101, 103, 104, 
	110, 115, 119, 35, 35, 35, 101, 110, 
	115, 119, 35, 48, 57, 35, 48, 57, 
	35, 48, 57, 35, 48, 57, 35, 35, 
	80, 35, 35, 101, 110, 115, 119, 35, 
	35, 67, 71, 77, 83, 35, 48, 52, 
	35, 35, 67, 71, 76, 83, 97, 100, 
	103, 114, 116, 122, 32, 35, 9, 13, 
	48, 57, 35, 48, 57, 35, 48, 57, 
	35, 47, 35, 48, 57, 35, 48, 57, 
	35, 47, 35, 48, 57, 35, 48, 57, 
	35, 32, 35, 43, 45, 9, 13, 35, 
	43, 45, 35, 48, 57, 35, 44, 46, 
	48, 57, 35, 48, 57, 35, 35, 44, 
	46, 32, 35, 9, 13, 48, 57, 35, 
	48, 57, 35, 48, 57, 35, 58, 35, 
	48, 57, 35, 48, 57, 35, 58, 35, 
	48, 57, 35, 48, 57, 35, 32, 35, 
	9, 13, 48, 50, 35, 48, 50, 35, 
	48, 57, 35, 47, 58, 35, 48, 53, 
	35, 48, 57, 35, 46, 47, 58, 35, 
	48, 57, 35, 35, 48, 53, 35, 48, 
	57, 35, 44, 46, 58, 35, 48, 57, 
	35, 58, 48, 57, 35, 58, 32, 35, 
	43, 45, 9, 13, 48, 57, 32, 35, 
	43, 45, 9, 13, 48, 57, 35, 48, 
	57, 35, 33, 47, 48, 57, 58, 64, 
	91, 96, 123, 126, 35, 48, 53, 35, 
	48, 57, 35, 35, 33, 47, 48, 57, 
	58, 64, 91, 96, 123, 126, 35, 33, 
	47, 58, 64, 91, 96, 123, 126, 35, 
	48, 57, 35, 65, 70, 84, 97, 102, 
	103, 106, 110, 119, 122, 115, 116, 35, 
	35, 80, 35, 35, 70, 72, 83, 97, 
	110, 115, 35, 35, 35, 35, 48, 49, 
	35, 107, 35, 48, 49, 35, 101, 119, 
	80, 35, 70, 72, 83, 97, 110, 115, 
	35, 35, 35, 48, 49, 107, 48, 49, 
	101, 119, 6, 35, 58, 6, 35, 58, 
	48, 53, 6, 35, 58, 6, 35, 58, 
	48, 53, 6, 35, 58, 48, 53, 0
};

static const char _command_single_lengths[] = {
	0, 1, 15, 3, 1, 1, 1, 1, 
	2, 1, 1, 9, 2, 1, 2, 1, 
	2, 0, 0, 0, 0, 0, 1, 1, 
	1, 1, 2, 0, 0, 0, 0, 0, 
	1, 2, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 1, 19, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 5, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 0, 1, 
	0, 1, 1, 1, 1, 1, 7, 1, 
	1, 4, 0, 0, 0, 0, 1, 1, 
	1, 5, 1, 4, 1, 1, 10, 1, 
	0, 0, 1, 0, 0, 1, 0, 0, 
	1, 3, 2, 0, 3, 0, 1, 3, 
	1, 0, 0, 1, 0, 0, 1, 0, 
	0, 1, 1, 0, 0, 2, 0, 0, 
	3, 0, 1, 0, 0, 4, 0, 2, 
	2, 3, 3, 0, 0, 0, 0, 1, 
	0, 0, 0, 10, 1, 1, 2, 16, 
	4, 1, 1, 1, 2, 2, 1, 1, 
	10, 2, 1, 2, 1, 3, 1, 1, 
	1, 1, 1, 1, 1, 2, 2, 3, 
	1, 1, 1, 1, 1, 1, 3, 1, 
	1, 1, 1, 1, 1, 1, 1, 2, 
	20, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 6, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 2, 1, 2, 1, 1, 1, 1, 
	2, 2, 8, 1, 1, 5, 1, 1, 
	1, 1, 1, 2, 1, 5, 1, 5, 
	1, 1, 11, 2, 1, 1, 2, 1, 
	1, 2, 1, 1, 1, 4, 3, 1, 
	3, 1, 1, 3, 2, 1, 1, 2, 
	1, 1, 2, 1, 1, 1, 2, 1, 
	1, 3, 1, 1, 4, 1, 1, 1, 
	1, 4, 1, 2, 2, 4, 4, 1, 
	1, 1, 1, 1, 1, 1, 1, 11, 
	1, 2, 1, 7, 1, 1, 1, 1, 
	2, 1, 3, 1, 1, 6, 1, 1, 
	1, 0, 1, 0, 2, 3, 3, 3, 
	3, 3
};

static const char _command_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 1, 1, 1, 0, 0, 
	0, 0, 0, 1, 1, 1, 1, 1, 
	0, 0, 1, 1, 1, 1, 1, 0, 
	0, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 1, 1, 
	1, 1, 1, 0, 1, 0, 0, 0, 
	0, 0, 1, 1, 1, 1, 0, 0, 
	0, 0, 0, 0, 1, 0, 0, 2, 
	1, 1, 0, 1, 1, 0, 1, 1, 
	0, 1, 0, 1, 1, 1, 0, 0, 
	2, 1, 1, 0, 1, 1, 0, 1, 
	1, 0, 2, 1, 1, 0, 1, 1, 
	0, 1, 0, 1, 1, 0, 1, 1, 
	0, 2, 2, 1, 5, 1, 1, 0, 
	5, 4, 1, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 1, 1, 
	1, 1, 1, 0, 0, 0, 0, 0, 
	1, 1, 1, 1, 1, 0, 0, 1, 
	1, 1, 1, 1, 0, 0, 0, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1, 1, 1, 1, 1, 0, 
	1, 0, 0, 0, 0, 0, 1, 1, 
	1, 1, 0, 0, 0, 0, 0, 0, 
	1, 0, 0, 2, 1, 1, 0, 1, 
	1, 0, 1, 1, 0, 1, 0, 1, 
	1, 1, 0, 0, 2, 1, 1, 0, 
	1, 1, 0, 1, 1, 0, 2, 1, 
	1, 0, 1, 1, 0, 1, 0, 1, 
	1, 0, 1, 1, 0, 2, 2, 1, 
	5, 1, 1, 0, 5, 4, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 1, 
	0, 1, 0, 0, 0, 0, 0, 0, 
	0, 1, 0, 1, 0, 0, 1, 0, 
	1, 1
};

static const short _command_index_offsets[] = {
	0, 0, 2, 18, 22, 24, 26, 28, 
	30, 33, 35, 37, 47, 50, 52, 55, 
	57, 60, 62, 64, 66, 68, 70, 72, 
	74, 76, 78, 81, 83, 85, 87, 89, 
	91, 93, 96, 98, 100, 102, 104, 106, 
	108, 110, 112, 115, 117, 137, 139, 141, 
	143, 145, 147, 149, 151, 153, 155, 157, 
	163, 165, 167, 169, 171, 173, 175, 177, 
	179, 181, 183, 185, 187, 189, 191, 193, 
	196, 198, 201, 204, 206, 209, 211, 219, 
	221, 223, 228, 230, 232, 234, 236, 238, 
	240, 242, 248, 250, 255, 258, 260, 271, 
	275, 277, 279, 281, 283, 285, 287, 289, 
	291, 293, 298, 301, 303, 308, 310, 312, 
	316, 320, 322, 324, 326, 328, 330, 332, 
	334, 336, 338, 342, 344, 346, 349, 351, 
	353, 357, 359, 361, 363, 365, 370, 372, 
	376, 379, 385, 391, 393, 399, 401, 403, 
	405, 411, 416, 418, 430, 432, 434, 437, 
	454, 459, 461, 463, 465, 468, 471, 473, 
	475, 486, 489, 491, 494, 496, 500, 503, 
	506, 509, 512, 515, 517, 519, 522, 525, 
	529, 532, 535, 538, 541, 544, 546, 550, 
	553, 556, 559, 562, 565, 567, 569, 571, 
	575, 596, 598, 600, 602, 604, 606, 608, 
	610, 612, 614, 616, 623, 625, 627, 629, 
	631, 633, 635, 637, 639, 641, 643, 645, 
	647, 649, 652, 655, 659, 662, 665, 668, 
	670, 674, 677, 686, 688, 690, 696, 699, 
	702, 705, 708, 710, 713, 715, 721, 723, 
	729, 732, 734, 746, 751, 754, 757, 760, 
	763, 766, 769, 772, 775, 777, 783, 787, 
	790, 795, 798, 800, 804, 809, 812, 815, 
	818, 821, 824, 827, 830, 833, 835, 840, 
	843, 846, 850, 853, 856, 861, 864, 866, 
	869, 872, 877, 880, 884, 887, 894, 901, 
	904, 911, 914, 917, 919, 926, 932, 935, 
	948, 950, 953, 955, 963, 965, 967, 969, 
	972, 975, 978, 982, 984, 986, 993, 995, 
	997, 999, 1001, 1003, 1005, 1008, 1012, 1017, 
	1021, 1026
};

static const short _command_indicies[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 
	8, 9, 10, 11, 12, 13, 14, 15, 
	16, 1, 17, 18, 19, 1, 20, 1, 
	21, 1, 22, 1, 23, 1, 24, 25, 
	1, 24, 1, 26, 1, 27, 28, 29, 
	30, 31, 32, 33, 34, 35, 1, 36, 
	37, 1, 38, 1, 39, 40, 1, 41, 
	1, 42, 43, 1, 44, 1, 45, 1, 
	46, 1, 47, 1, 48, 1, 49, 1, 
	50, 1, 51, 1, 52, 1, 53, 54, 
	1, 55, 1, 56, 1, 57, 1, 58, 
	1, 59, 1, 60, 1, 61, 62, 1, 
	63, 1, 64, 1, 65, 1, 66, 1, 
	67, 1, 68, 1, 69, 1, 70, 1, 
	71, 71, 1, 72, 1, 73, 74, 75, 
	76, 77, 78, 79, 80, 81, 82, 83, 
	84, 85, 86, 87, 88, 89, 90, 91, 
	1, 92, 1, 93, 1, 94, 1, 95, 
	1, 96, 1, 97, 1, 98, 1, 99, 
	1, 100, 1, 101, 1, 102, 103, 104, 
	105, 106, 1, 107, 1, 108, 1, 109, 
	1, 110, 1, 111, 1, 112, 1, 113, 
	1, 114, 1, 115, 1, 116, 1, 117, 
	1, 118, 1, 119, 1, 120, 1, 121, 
	1, 122, 123, 1, 124, 1, 125, 126, 
	1, 125, 127, 1, 125, 1, 122, 128, 
	1, 122, 1, 129, 130, 131, 130, 130, 
	130, 130, 1, 132, 1, 133, 1, 134, 
	134, 134, 134, 1, 135, 1, 136, 1, 
	137, 1, 138, 1, 139, 1, 140, 1, 
	141, 1, 142, 143, 143, 143, 143, 1, 
	142, 1, 144, 144, 144, 144, 1, 145, 
	146, 1, 145, 1, 147, 148, 149, 150, 
	151, 151, 151, 150, 151, 151, 1, 152, 
	152, 153, 1, 153, 1, 154, 1, 155, 
	1, 156, 1, 157, 1, 158, 1, 159, 
	1, 160, 1, 161, 1, 162, 163, 164, 
	162, 1, 163, 164, 1, 165, 1, 166, 
	167, 167, 168, 1, 169, 1, 166, 1, 
	166, 167, 167, 1, 170, 170, 171, 1, 
	171, 1, 172, 1, 173, 1, 174, 1, 
	175, 1, 176, 1, 177, 1, 178, 1, 
	179, 1, 180, 180, 181, 1, 181, 1, 
	182, 1, 183, 183, 1, 184, 1, 185, 
	1, 186, 187, 187, 1, 188, 1, 189, 
	1, 190, 1, 191, 1, 189, 192, 192, 
	193, 1, 194, 1, 189, 193, 195, 1, 
	189, 193, 1, 196, 197, 198, 196, 199, 
	1, 197, 197, 198, 197, 199, 1, 199, 
	1, 200, 201, 200, 200, 200, 1, 202, 
	1, 203, 1, 204, 187, 200, 205, 200, 
	200, 200, 1, 200, 200, 200, 200, 1, 
	206, 1, 207, 207, 207, 207, 208, 207, 
	207, 208, 208, 207, 208, 1, 209, 1, 
	211, 210, 211, 212, 210, 211, 213, 214, 
	215, 216, 217, 218, 219, 220, 221, 222, 
	223, 224, 225, 226, 227, 210, 211, 228, 
	229, 230, 210, 231, 210, 232, 210, 233, 
	210, 211, 234, 210, 235, 236, 210, 235, 
	210, 237, 210, 211, 238, 239, 240, 241, 
	242, 243, 244, 245, 246, 210, 247, 248, 
	210, 249, 210, 250, 251, 210, 252, 210, 
	211, 253, 254, 210, 211, 255, 210, 211, 
	256, 210, 211, 257, 210, 211, 258, 210, 
	211, 259, 210, 260, 210, 261, 210, 211, 
	262, 210, 211, 263, 210, 211, 264, 265, 
	210, 211, 266, 210, 211, 267, 210, 211, 
	268, 210, 211, 269, 210, 211, 270, 210, 
	271, 210, 211, 272, 273, 210, 211, 274, 
	210, 211, 275, 210, 211, 276, 210, 211, 
	277, 210, 211, 278, 210, 279, 210, 280, 
	210, 281, 210, 211, 282, 282, 210, 211, 
	283, 284, 285, 286, 287, 288, 289, 290, 
	291, 292, 293, 294, 295, 296, 297, 298, 
	299, 300, 301, 210, 302, 210, 303, 210, 
	304, 210, 305, 210, 306, 210, 307, 210, 
	308, 210, 309, 210, 310, 210, 311, 210, 
	211, 312, 313, 314, 315, 316, 210, 317, 
	210, 318, 210, 319, 210, 320, 210, 321, 
	210, 322, 210, 323, 210, 324, 210, 325, 
	210, 326, 210, 327, 210, 328, 210, 329, 
	210, 211, 330, 210, 211, 331, 210, 211, 
	332, 333, 210, 211, 334, 210, 335, 336, 
	210, 335, 337, 210, 335, 210, 211, 332, 
	338, 210, 211, 332, 210, 211, 339, 340, 
	341, 340, 340, 340, 340, 210, 342, 210, 
	343, 210, 211, 344, 344, 344, 344, 210, 
	211, 345, 210, 211, 346, 210, 211, 347, 
	210, 211, 348, 210, 349, 210, 211, 350, 
	210, 351, 210, 352, 353, 353, 353, 353, 
	210, 352, 210, 211, 354, 354, 354, 354, 
	210, 355, 356, 210, 355, 210, 211, 357, 
	358, 359, 360, 361, 361, 361, 360, 361, 
	361, 210, 362, 211, 362, 363, 210, 211, 
	363, 210, 211, 364, 210, 211, 365, 210, 
	211, 366, 210, 211, 367, 210, 211, 368, 
	210, 211, 369, 210, 211, 370, 210, 371, 
	210, 372, 211, 373, 374, 372, 210, 211, 
	373, 374, 210, 211, 375, 210, 376, 377, 
	377, 378, 210, 211, 379, 210, 376, 210, 
	376, 377, 377, 210, 380, 211, 380, 381, 
	210, 211, 381, 210, 211, 382, 210, 211, 
	383, 210, 211, 384, 210, 211, 385, 210, 
	211, 386, 210, 211, 387, 210, 211, 388, 
	210, 389, 210, 390, 211, 390, 391, 210, 
	211, 391, 210, 211, 392, 210, 211, 393, 
	393, 210, 211, 394, 210, 211, 395, 210, 
	211, 396, 397, 397, 210, 211, 398, 210, 
	399, 210, 211, 400, 210, 211, 401, 210, 
	399, 402, 402, 403, 210, 211, 404, 210, 
	399, 403, 405, 210, 399, 403, 210, 406, 
	211, 407, 408, 406, 409, 210, 407, 211, 
	407, 408, 407, 409, 210, 211, 409, 210, 
	411, 410, 412, 410, 410, 410, 210, 211, 
	413, 210, 211, 414, 210, 415, 397, 411, 
	410, 416, 410, 410, 410, 210, 411, 410, 
	410, 410, 410, 210, 211, 417, 210, 211, 
	418, 418, 418, 418, 208, 418, 418, 208, 
	208, 418, 208, 210, 419, 210, 211, 420, 
	210, 421, 210, 211, 422, 423, 424, 425, 
	426, 427, 210, 428, 210, 429, 210, 430, 
	210, 211, 431, 210, 211, 432, 210, 211, 
	433, 210, 211, 434, 434, 210, 435, 1, 
	436, 1, 437, 438, 439, 440, 441, 442, 
	1, 443, 1, 444, 1, 445, 1, 446, 
	1, 447, 1, 448, 1, 449, 449, 1, 
	450, 451, 0, 1, 450, 451, 0, 190, 
	1, 452, 211, 212, 210, 452, 211, 212, 
	400, 210, 452, 211, 212, 413, 210, 0
};

static const short _command_trans_targs[] = {
	2, 0, 3, 7, 10, 11, 44, 69, 
	78, 87, 89, 91, 94, 146, 147, 307, 
	309, 4, 5, 6, 317, 317, 317, 8, 
	317, 9, 317, 12, 14, 16, 23, 24, 
	33, 40, 41, 42, 317, 13, 317, 317, 
	15, 317, 17, 17, 18, 19, 20, 21, 
	22, 317, 317, 25, 26, 27, 27, 28, 
	29, 30, 31, 32, 317, 34, 34, 35, 
	36, 37, 38, 39, 317, 317, 317, 43, 
	317, 45, 46, 47, 48, 49, 50, 51, 
	52, 53, 54, 55, 61, 62, 63, 64, 
	65, 66, 67, 68, 317, 317, 317, 317, 
	317, 317, 317, 317, 317, 317, 56, 57, 
	58, 59, 60, 317, 317, 317, 317, 317, 
	317, 317, 317, 317, 317, 317, 317, 317, 
	70, 71, 72, 76, 73, 317, 74, 75, 
	77, 79, 80, 81, 317, 317, 82, 83, 
	84, 85, 86, 317, 88, 317, 317, 90, 
	92, 317, 93, 95, 105, 112, 122, 137, 
	96, 97, 98, 99, 100, 101, 102, 103, 
	104, 317, 106, 107, 107, 108, 317, 109, 
	111, 110, 113, 114, 115, 116, 117, 118, 
	119, 120, 121, 317, 123, 124, 125, 126, 
	127, 128, 129, 131, 130, 317, 132, 133, 
	134, 130, 135, 136, 138, 139, 139, 140, 
	141, 144, 142, 143, 318, 145, 43, 148, 
	149, 317, 149, 319, 151, 152, 156, 159, 
	160, 192, 217, 226, 235, 237, 239, 242, 
	294, 295, 297, 299, 153, 154, 155, 319, 
	319, 319, 157, 319, 158, 319, 161, 163, 
	165, 172, 173, 182, 189, 190, 191, 319, 
	162, 319, 319, 164, 319, 166, 166, 167, 
	168, 169, 170, 171, 319, 319, 174, 175, 
	176, 176, 177, 178, 179, 180, 181, 319, 
	183, 183, 184, 185, 186, 187, 188, 319, 
	319, 319, 149, 193, 194, 195, 196, 197, 
	198, 199, 200, 201, 202, 203, 209, 210, 
	211, 212, 213, 214, 215, 216, 319, 319, 
	319, 319, 319, 319, 319, 319, 319, 319, 
	204, 205, 206, 207, 208, 319, 319, 319, 
	319, 319, 319, 319, 319, 319, 319, 319, 
	319, 319, 218, 219, 220, 224, 221, 319, 
	222, 223, 225, 227, 228, 229, 319, 319, 
	230, 231, 232, 233, 234, 319, 236, 319, 
	319, 238, 240, 319, 241, 243, 253, 260, 
	270, 285, 244, 245, 246, 247, 248, 249, 
	250, 251, 252, 319, 254, 255, 255, 256, 
	319, 257, 259, 258, 261, 262, 263, 264, 
	265, 266, 267, 268, 269, 319, 271, 272, 
	273, 274, 275, 276, 277, 279, 278, 319, 
	280, 281, 282, 278, 283, 284, 286, 287, 
	287, 288, 289, 321, 292, 290, 291, 320, 
	293, 149, 296, 319, 298, 319, 300, 301, 
	302, 303, 304, 306, 319, 319, 319, 149, 
	305, 149, 149, 308, 317, 310, 311, 312, 
	313, 314, 316, 317, 317, 317, 43, 315, 
	43, 43, 1, 1, 150
};

static const short _command_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 111, 107, 109, 0, 
	47, 0, 113, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 79, 0, 75, 77, 
	0, 73, 0, 15, 13, 13, 13, 13, 
	13, 83, 93, 0, 0, 0, 15, 13, 
	13, 13, 13, 13, 89, 0, 15, 13, 
	13, 13, 13, 13, 81, 85, 91, 87, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 33, 39, 29, 65, 
	117, 51, 41, 27, 53, 71, 0, 0, 
	0, 0, 0, 119, 127, 121, 123, 125, 
	31, 69, 37, 43, 115, 35, 45, 310, 
	0, 9, 0, 9, 11, 129, 11, 11, 
	9, 0, 57, 0, 21, 17, 57, 7, 
	7, 7, 7, 19, 0, 97, 23, 57, 
	57, 25, 0, 0, 0, 0, 57, 57, 
	0, 1, 1, 0, 3, 3, 0, 5, 
	5, 61, 0, 0, 15, 1, 207, 0, 
	1, 0, 0, 1, 1, 0, 3, 3, 
	0, 5, 5, 67, 0, 1, 1, 0, 
	3, 141, 0, 0, 49, 55, 5, 144, 
	0, 0, 0, 0, 0, 0, 15, 1, 
	0, 1, 3, 141, 55, 1, 133, 57, 
	59, 131, 0, 135, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 273, 
	267, 270, 0, 192, 0, 276, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 231, 
	0, 225, 228, 0, 222, 0, 15, 13, 
	13, 13, 13, 13, 237, 249, 0, 0, 
	0, 15, 13, 13, 13, 13, 13, 243, 
	0, 15, 13, 13, 13, 13, 13, 234, 
	240, 246, 87, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 171, 180, 
	165, 210, 282, 195, 183, 162, 198, 219, 
	0, 0, 0, 0, 0, 285, 297, 288, 
	291, 294, 168, 216, 177, 186, 279, 174, 
	189, 317, 0, 9, 0, 9, 11, 300, 
	11, 11, 9, 0, 57, 0, 153, 147, 
	57, 7, 7, 7, 7, 150, 0, 255, 
	156, 57, 57, 159, 0, 0, 0, 0, 
	57, 57, 0, 1, 1, 0, 3, 3, 
	0, 5, 5, 204, 0, 0, 15, 1, 
	306, 0, 1, 0, 0, 1, 1, 0, 
	3, 3, 0, 5, 5, 213, 0, 1, 
	1, 0, 3, 141, 0, 0, 49, 201, 
	5, 144, 0, 0, 0, 0, 0, 0, 
	15, 1, 0, 135, 1, 3, 141, 201, 
	1, 133, 57, 303, 0, 252, 0, 0, 
	0, 0, 0, 0, 264, 258, 261, 139, 
	0, 137, 105, 0, 95, 0, 0, 0, 
	0, 0, 0, 103, 99, 101, 139, 0, 
	137, 105, 63, 0, 63
};

static const int command_start = 317;
static const int command_first_final = 317;
static const int command_error = 0;

static const int command_en_main = 317;


#line 222 "/Users/jordi/Documents/Arduino/esp32go/command.rl"





    
#line 823 "/dev/stdout"
	{
	cs = command_start;
	}

#line 828 "/dev/stdout"
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
				_trans += (unsigned int)(_mid - _keys);
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
				_trans += (unsigned int)((_mid - _keys)>>1);
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
#line 229 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(deg,(*p)); }
	break;
	case 1:
#line 230 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(min,(*p)); }
	break;
	case 2:
#line 231 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(sec,(*p)); }
	break;
	case 3:
#line 232 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(pulse,(*p));}
	break;
	case 4:
#line 233 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(ip3,(*p));}
	break;
	case 5:
#line 234 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(ip2,(*p));}
	break;
	case 6:
#line 235 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	break;
	case 7:
#line 236 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ neg=-1;}
	break;
	case 8:
#line 237 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_move(telescope,stcmd);}
	break;
	case 9:
#line 238 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	break;
	case 10:
#line 239 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 buzzerOn(300);
					 sprintf(tmessage,"0");APPEND;}
	break;
	case 11:
#line 244 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_stop(telescope,stcmd);}
	break;
	case 12:
#line 245 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{select_rate(telescope,stcmd); }
	break;
	case 13:
#line 246 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	break;
	case 14:
#line 248 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	break;
	case 15:
#line 250 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	break;
	case 16:
#line 251 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	break;
	case 17:
#line 252 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	break;
	case 18:
#line 253 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	break;
	case 19:
#line 254 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintdate1(tmessage);APPEND;}
	break;
	case 20:
#line 255 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintsite();}
	break;
	case 21:
#line 256 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	break;
	case 22:
#line 257 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	break;
	case 23:
#line 258 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	break;
	case 24:
#line 261 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	break;
	case 25:
#line 267 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{deg+=((*p)-'0')*6;}
	break;
	case 26:
#line 268 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	break;
	case 27:
#line 269 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
	break;
	case 28:
#line 270 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	break;
	case 29:
#line 273 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{deg=deg*3600+min*60;}
	break;
	case 30:
#line 274 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{deg+=sec;}
	break;
	case 31:
#line 275 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{stcmd=(*p);}
	break;
	case 32:
#line 276 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{stcmd=(*p);mark=p;mark++;}
	break;
	case 33:
#line 277 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_date(min,deg,sec);}
	break;
	case 34:
#line 278 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	break;
	case 35:
#line 279 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ telescope->time_zone=-deg*neg;}
	break;
	case 36:
#line 281 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	break;
	case 37:
#line 282 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_time(deg,min,sec);}
	break;
	case 38:
#line 283 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	break;
	case 39:
#line 284 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	break;
	case 40:
#line 285 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(0,pmotor->speed);}
	break;
	case 41:
#line 286 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(pmotor->max_steps,pmotor->speed);}
	break;
	case 42:
#line 287 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(0,pmotor->speed_low);}
	break;
	case 43:
#line 288 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(pmotor->max_steps,pmotor->speed_low);}
	break;
	case 44:
#line 289 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(pmotor->position+(focus_counter*neg));}
	break;
	case 45:
#line 290 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(focus_counter);}
	break;
	case 46:
#line 291 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{stopfocuser();}
	break;
	case 47:
#line 292 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if ((*p)=='0') pmotor=&focus_motor; else pmotor=&aux_motor; buzzerOn(300);}
	break;
	case 48:
#line 293 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{pmotor->position=pmotor->target=focus_counter;}
	break;
	case 49:
#line 294 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%05d#",pmotor->position);APPEND;}
	break;
	case 50:
#line 295 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%d#",pmotor->state<stop);APPEND;}
	break;
	case 51:
#line 296 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{buzzerOn(300);mount_goto_home(telescope);}
	break;
	case 52:
#line 297 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",(telescope->parked? "1" : "0"));APPEND;}
	break;
	case 53:
#line 298 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_home_set(telescope);}
	break;
	case 54:
#line 299 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
	break;
	case 55:
#line 300 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
	break;
	case 56:
#line 301 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{meridianflip(telescope,(*p)=='w');}
	break;
	case 57:
#line 302 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	break;
	case 58:
#line 303 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{telescope->track=1;}
	break;
	case 59:
#line 304 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{;}
	break;
	case 60:
#line 305 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	break;
	case 61:
#line 307 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
	break;
	case 62:
#line 308 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage, "%d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2));APPEND;}
	break;
	case 63:
#line 309 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"012 24 2000#") ;APPEND;}
	break;
	case 64:
#line 310 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"01.0#") ;APPEND;}
	break;
	case 65:
#line 311 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	break;
	case 66:
#line 312 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"00:00:00#") ;APPEND;}
	break;
	case 67:
#line 313 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	break;
	case 68:
#line 314 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{setwifipad(ip3,ip2);}
	break;
	case 69:
#line 315 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{appcmd(stcmd);APPEND;}
	break;
	case 70:
#line 316 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	break;
	case 71:
#line 325 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						}
						}
	break;
	case 72:
#line 333 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{setnunchuk((*p));}
	break;
	case 73:
#line 334 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{setflipmode((*p));}
	break;
#line 1226 "/dev/stdout"
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

#line 418 "/Users/jordi/Documents/Arduino/esp32go/command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
		return 0;
};



