
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
  case 'g':sprintf(tmessage,"%f#",telescope->rate[0][0]);
  break;
  case 'j':sprintf(tmessage,"%f#",telescope->rate[0][1]);
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
    
#line 243 "/dev/stdout"
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
	52, 1, 53, 1, 54, 1, 55, 1, 
	56, 1, 57, 1, 58, 1, 59, 1, 
	60, 1, 61, 1, 62, 1, 63, 1, 
	64, 1, 65, 1, 66, 1, 67, 1, 
	68, 1, 69, 1, 70, 1, 71, 1, 
	72, 1, 73, 1, 74, 1, 75, 1, 
	76, 1, 77, 1, 78, 1, 79, 1, 
	80, 1, 81, 1, 82, 1, 83, 1, 
	84, 2, 1, 29, 2, 2, 30, 2, 
	8, 77, 2, 9, 77, 2, 10, 77, 
	2, 11, 77, 2, 12, 77, 2, 13, 
	77, 2, 14, 77, 2, 15, 77, 2, 
	16, 77, 2, 17, 77, 2, 18, 77, 
	2, 19, 77, 2, 20, 77, 2, 22, 
	77, 2, 23, 77, 2, 24, 77, 2, 
	26, 77, 2, 27, 77, 2, 28, 77, 
	2, 33, 77, 2, 35, 21, 2, 36, 
	77, 2, 37, 77, 2, 38, 77, 2, 
	39, 77, 2, 40, 77, 2, 41, 77, 
	2, 42, 77, 2, 43, 77, 2, 44, 
	77, 2, 45, 77, 2, 46, 77, 2, 
	49, 77, 2, 50, 77, 2, 52, 77, 
	2, 54, 77, 2, 55, 77, 2, 56, 
	77, 2, 58, 77, 2, 59, 77, 2, 
	61, 77, 2, 62, 77, 2, 63, 77, 
	2, 64, 77, 2, 65, 77, 2, 66, 
	77, 2, 67, 77, 2, 68, 77, 2, 
	69, 77, 2, 70, 77, 2, 71, 77, 
	2, 72, 77, 2, 73, 77, 2, 74, 
	77, 2, 77, 75, 2, 80, 77, 2, 
	81, 77, 2, 82, 77, 2, 83, 77, 
	2, 84, 77, 3, 35, 21, 77, 6, 
	13, 14, 15, 16, 67, 51, 7, 13, 
	14, 15, 16, 67, 51, 77
};

static const short _command_key_offsets[] = {
	0, 0, 1, 18, 21, 22, 23, 24, 
	25, 27, 28, 29, 39, 41, 42, 44, 
	45, 47, 49, 51, 53, 55, 57, 58, 
	59, 61, 62, 63, 64, 66, 68, 70, 
	72, 74, 76, 77, 79, 81, 83, 85, 
	87, 89, 90, 91, 92, 95, 115, 116, 
	117, 118, 119, 120, 121, 122, 123, 124, 
	125, 126, 131, 132, 133, 134, 135, 136, 
	137, 138, 139, 140, 141, 142, 143, 144, 
	145, 147, 150, 152, 155, 158, 159, 162, 
	163, 171, 172, 173, 177, 179, 181, 183, 
	185, 186, 187, 188, 193, 194, 198, 201, 
	202, 212, 217, 219, 221, 222, 224, 226, 
	227, 229, 231, 232, 237, 239, 241, 246, 
	248, 249, 252, 257, 259, 261, 262, 264, 
	266, 267, 269, 271, 272, 277, 279, 281, 
	283, 285, 287, 290, 292, 293, 295, 297, 
	301, 303, 307, 309, 316, 323, 325, 335, 
	337, 339, 340, 350, 358, 362, 363, 364, 
	365, 366, 368, 383, 384, 385, 386, 387, 
	389, 407, 411, 412, 413, 414, 416, 418, 
	419, 420, 431, 433, 434, 436, 437, 440, 
	443, 446, 449, 452, 455, 456, 457, 460, 
	462, 464, 467, 470, 473, 476, 479, 482, 
	483, 486, 489, 492, 495, 498, 501, 502, 
	503, 504, 508, 529, 530, 531, 532, 533, 
	534, 535, 536, 537, 538, 539, 540, 546, 
	547, 548, 549, 550, 551, 552, 553, 554, 
	555, 556, 557, 558, 559, 561, 564, 568, 
	571, 574, 577, 578, 582, 584, 593, 594, 
	595, 600, 603, 606, 609, 612, 613, 615, 
	616, 621, 622, 627, 630, 631, 642, 648, 
	651, 654, 656, 659, 662, 664, 667, 670, 
	671, 677, 680, 683, 688, 691, 692, 695, 
	701, 704, 707, 709, 712, 715, 717, 720, 
	723, 724, 730, 733, 736, 739, 742, 745, 
	749, 752, 753, 756, 759, 763, 766, 770, 
	772, 780, 788, 791, 802, 805, 808, 809, 
	820, 829, 834, 835, 836, 837, 838, 841, 
	857, 858, 860, 861, 865, 868, 869, 870, 
	877, 878, 879, 880, 883, 885, 888, 891, 
	894, 896, 897, 898, 904, 905, 906, 907, 
	909, 910, 912, 914, 917, 922, 925, 930
};

static const char _command_trans_keys[] = {
	58, 65, 67, 68, 70, 71, 73, 77, 
	80, 81, 82, 83, 84, 88, 97, 99, 
	104, 112, 65, 76, 80, 35, 35, 35, 
	77, 35, 82, 35, 35, 43, 45, 65, 
	66, 73, 76, 80, 81, 112, 115, 35, 
	43, 35, 35, 45, 35, 43, 45, 48, 
	57, 48, 57, 48, 57, 48, 57, 48, 
	57, 35, 35, 48, 56, 35, 83, 49, 
	43, 45, 48, 57, 48, 57, 48, 57, 
	48, 57, 48, 57, 35, 43, 45, 48, 
	57, 48, 57, 48, 57, 48, 57, 48, 
	57, 35, 35, 35, 44, 48, 49, 65, 
	67, 68, 71, 75, 76, 77, 82, 83, 
	84, 85, 86, 90, 99, 100, 103, 107, 
	114, 116, 120, 35, 35, 35, 35, 35, 
	35, 35, 35, 35, 35, 35, 68, 70, 
	78, 80, 84, 35, 35, 35, 35, 35, 
	35, 35, 35, 35, 35, 35, 35, 35, 
	80, 48, 57, 46, 48, 57, 48, 57, 
	35, 48, 57, 35, 48, 57, 35, 46, 
	48, 57, 46, 83, 101, 103, 104, 110, 
	119, 115, 116, 35, 35, 101, 110, 115, 
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
	58, 64, 91, 96, 123, 126, 75, 76, 
	81, 83, 35, 35, 35, 35, 48, 57, 
	65, 68, 70, 82, 84, 97, 100, 102, 
	103, 106, 110, 119, 122, 115, 116, 35, 
	82, 35, 35, 35, 58, 35, 65, 67, 
	68, 70, 71, 73, 77, 80, 81, 82, 
	83, 84, 88, 97, 99, 104, 112, 35, 
	65, 76, 80, 35, 35, 35, 35, 77, 
	35, 82, 35, 35, 35, 43, 45, 65, 
	66, 73, 76, 80, 81, 112, 115, 35, 
	43, 35, 35, 45, 35, 35, 43, 45, 
	35, 48, 57, 35, 48, 57, 35, 48, 
	57, 35, 48, 57, 35, 48, 57, 35, 
	35, 35, 48, 56, 35, 83, 35, 49, 
	35, 43, 45, 35, 48, 57, 35, 48, 
	57, 35, 48, 57, 35, 48, 57, 35, 
	48, 57, 35, 35, 43, 45, 35, 48, 
	57, 35, 48, 57, 35, 48, 57, 35, 
	48, 57, 35, 48, 57, 35, 35, 35, 
	35, 44, 48, 49, 35, 65, 67, 68, 
	71, 75, 76, 77, 82, 83, 84, 85, 
	86, 90, 99, 100, 103, 107, 114, 116, 
	120, 35, 35, 35, 35, 35, 35, 35, 
	35, 35, 35, 35, 35, 68, 70, 78, 
	80, 84, 35, 35, 35, 35, 35, 35, 
	35, 35, 35, 35, 35, 35, 35, 35, 
	80, 35, 48, 57, 35, 46, 48, 57, 
	35, 48, 57, 35, 48, 57, 35, 48, 
	57, 35, 35, 46, 48, 57, 35, 46, 
	35, 83, 101, 103, 104, 110, 119, 115, 
	116, 35, 35, 35, 101, 110, 115, 119, 
	35, 48, 57, 35, 48, 57, 35, 48, 
	57, 35, 48, 57, 35, 35, 80, 35, 
	35, 101, 110, 115, 119, 35, 35, 67, 
	71, 77, 83, 35, 48, 52, 35, 35, 
	67, 71, 76, 83, 97, 100, 103, 114, 
	116, 122, 32, 35, 9, 13, 48, 57, 
	35, 48, 57, 35, 48, 57, 35, 47, 
	35, 48, 57, 35, 48, 57, 35, 47, 
	35, 48, 57, 35, 48, 57, 35, 32, 
	35, 43, 45, 9, 13, 35, 43, 45, 
	35, 48, 57, 35, 44, 46, 48, 57, 
	35, 48, 57, 35, 35, 44, 46, 32, 
	35, 9, 13, 48, 57, 35, 48, 57, 
	35, 48, 57, 35, 58, 35, 48, 57, 
	35, 48, 57, 35, 58, 35, 48, 57, 
	35, 48, 57, 35, 32, 35, 9, 13, 
	48, 50, 35, 48, 50, 35, 48, 57, 
	35, 47, 58, 35, 48, 53, 35, 48, 
	57, 35, 46, 47, 58, 35, 48, 57, 
	35, 35, 48, 53, 35, 48, 57, 35, 
	44, 46, 58, 35, 48, 57, 35, 58, 
	48, 57, 35, 58, 32, 35, 43, 45, 
	9, 13, 48, 57, 32, 35, 43, 45, 
	9, 13, 48, 57, 35, 48, 57, 35, 
	33, 47, 48, 57, 58, 64, 91, 96, 
	123, 126, 35, 48, 53, 35, 48, 57, 
	35, 35, 33, 47, 48, 57, 58, 64, 
	91, 96, 123, 126, 35, 33, 47, 58, 
	64, 91, 96, 123, 126, 35, 75, 76, 
	81, 83, 35, 35, 35, 35, 35, 48, 
	57, 35, 65, 68, 70, 82, 84, 97, 
	100, 102, 103, 106, 110, 119, 122, 115, 
	116, 35, 35, 82, 35, 35, 72, 80, 
	83, 35, 48, 57, 35, 35, 35, 70, 
	72, 83, 97, 110, 115, 35, 35, 35, 
	35, 48, 49, 35, 107, 35, 48, 49, 
	35, 101, 119, 72, 80, 83, 48, 57, 
	35, 35, 70, 72, 83, 97, 110, 115, 
	35, 35, 35, 48, 49, 107, 48, 49, 
	101, 119, 6, 35, 58, 6, 35, 58, 
	48, 53, 6, 35, 58, 6, 35, 58, 
	48, 53, 6, 35, 58, 48, 53, 0
};

static const char _command_single_lengths[] = {
	0, 1, 17, 3, 1, 1, 1, 1, 
	2, 1, 1, 10, 2, 1, 2, 1, 
	2, 0, 0, 0, 0, 0, 1, 1, 
	0, 1, 1, 1, 2, 0, 0, 0, 
	0, 0, 1, 2, 0, 0, 0, 0, 
	0, 1, 1, 1, 1, 20, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 5, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	0, 1, 0, 1, 1, 1, 1, 1, 
	6, 1, 1, 4, 0, 0, 0, 0, 
	1, 1, 1, 5, 1, 4, 1, 1, 
	10, 1, 0, 0, 1, 0, 0, 1, 
	0, 0, 1, 3, 2, 0, 3, 0, 
	1, 3, 1, 0, 0, 1, 0, 0, 
	1, 0, 0, 1, 1, 0, 0, 2, 
	0, 0, 3, 0, 1, 0, 0, 4, 
	0, 2, 2, 3, 3, 0, 0, 0, 
	0, 1, 0, 0, 4, 1, 1, 1, 
	1, 0, 13, 1, 1, 1, 1, 2, 
	18, 4, 1, 1, 1, 2, 2, 1, 
	1, 11, 2, 1, 2, 1, 3, 1, 
	1, 1, 1, 1, 1, 1, 1, 2, 
	2, 3, 1, 1, 1, 1, 1, 1, 
	3, 1, 1, 1, 1, 1, 1, 1, 
	1, 2, 21, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 6, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 2, 1, 2, 1, 
	1, 1, 1, 2, 2, 7, 1, 1, 
	5, 1, 1, 1, 1, 1, 2, 1, 
	5, 1, 5, 1, 1, 11, 2, 1, 
	1, 2, 1, 1, 2, 1, 1, 1, 
	4, 3, 1, 3, 1, 1, 3, 2, 
	1, 1, 2, 1, 1, 2, 1, 1, 
	1, 2, 1, 1, 3, 1, 1, 4, 
	1, 1, 1, 1, 4, 1, 2, 2, 
	4, 4, 1, 1, 1, 1, 1, 1, 
	1, 5, 1, 1, 1, 1, 1, 14, 
	1, 2, 1, 4, 1, 1, 1, 7, 
	1, 1, 1, 1, 2, 1, 3, 3, 
	0, 1, 1, 6, 1, 1, 1, 0, 
	1, 0, 2, 3, 3, 3, 3, 3
};

static const char _command_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 1, 1, 1, 0, 0, 
	1, 0, 0, 0, 0, 1, 1, 1, 
	1, 1, 0, 0, 1, 1, 1, 1, 
	1, 0, 0, 0, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	1, 1, 1, 1, 1, 0, 1, 0, 
	1, 0, 0, 0, 1, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 1, 0, 
	0, 2, 1, 1, 0, 1, 1, 0, 
	1, 1, 0, 1, 0, 1, 1, 1, 
	0, 0, 2, 1, 1, 0, 1, 1, 
	0, 1, 1, 0, 2, 1, 1, 0, 
	1, 1, 0, 1, 0, 1, 1, 0, 
	1, 1, 0, 2, 2, 1, 5, 1, 
	1, 0, 5, 4, 0, 0, 0, 0, 
	0, 1, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 1, 0, 0, 1, 0, 
	0, 0, 1, 1, 1, 1, 1, 0, 
	0, 1, 1, 1, 1, 1, 0, 0, 
	0, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 1, 1, 1, 
	1, 1, 0, 1, 0, 1, 0, 0, 
	0, 1, 1, 1, 1, 0, 0, 0, 
	0, 0, 0, 1, 0, 0, 2, 1, 
	1, 0, 1, 1, 0, 1, 1, 0, 
	1, 0, 1, 1, 1, 0, 0, 2, 
	1, 1, 0, 1, 1, 0, 1, 1, 
	0, 2, 1, 1, 0, 1, 1, 0, 
	1, 0, 1, 1, 0, 1, 1, 0, 
	2, 2, 1, 5, 1, 1, 0, 5, 
	4, 0, 0, 0, 0, 0, 1, 1, 
	0, 0, 0, 0, 1, 0, 0, 0, 
	0, 0, 0, 1, 0, 1, 0, 0, 
	1, 0, 0, 0, 0, 0, 0, 1, 
	0, 1, 0, 0, 1, 0, 1, 1
};

static const short _command_index_offsets[] = {
	0, 0, 2, 20, 24, 26, 28, 30, 
	32, 35, 37, 39, 50, 53, 55, 58, 
	60, 63, 65, 67, 69, 71, 73, 75, 
	77, 79, 81, 83, 85, 88, 90, 92, 
	94, 96, 98, 100, 103, 105, 107, 109, 
	111, 113, 115, 117, 119, 122, 143, 145, 
	147, 149, 151, 153, 155, 157, 159, 161, 
	163, 165, 171, 173, 175, 177, 179, 181, 
	183, 185, 187, 189, 191, 193, 195, 197, 
	199, 201, 204, 206, 209, 212, 214, 217, 
	219, 227, 229, 231, 236, 238, 240, 242, 
	244, 246, 248, 250, 256, 258, 263, 266, 
	268, 279, 283, 285, 287, 289, 291, 293, 
	295, 297, 299, 301, 306, 309, 311, 316, 
	318, 320, 324, 328, 330, 332, 334, 336, 
	338, 340, 342, 344, 346, 350, 352, 354, 
	357, 359, 361, 365, 367, 369, 371, 373, 
	378, 380, 384, 387, 393, 399, 401, 407, 
	409, 411, 413, 419, 424, 429, 431, 433, 
	435, 437, 439, 454, 456, 458, 460, 462, 
	465, 484, 489, 491, 493, 495, 498, 501, 
	503, 505, 517, 520, 522, 525, 527, 531, 
	534, 537, 540, 543, 546, 548, 550, 553, 
	556, 559, 563, 566, 569, 572, 575, 578, 
	580, 584, 587, 590, 593, 596, 599, 601, 
	603, 605, 609, 631, 633, 635, 637, 639, 
	641, 643, 645, 647, 649, 651, 653, 660, 
	662, 664, 666, 668, 670, 672, 674, 676, 
	678, 680, 682, 684, 686, 689, 692, 696, 
	699, 702, 705, 707, 711, 714, 723, 725, 
	727, 733, 736, 739, 742, 745, 747, 750, 
	752, 758, 760, 766, 769, 771, 783, 788, 
	791, 794, 797, 800, 803, 806, 809, 812, 
	814, 820, 824, 827, 832, 835, 837, 841, 
	846, 849, 852, 855, 858, 861, 864, 867, 
	870, 872, 877, 880, 883, 887, 890, 893, 
	898, 901, 903, 906, 909, 914, 917, 921, 
	924, 931, 938, 941, 948, 951, 954, 956, 
	963, 969, 975, 977, 979, 981, 983, 986, 
	1002, 1004, 1007, 1009, 1014, 1017, 1019, 1021, 
	1029, 1031, 1033, 1035, 1038, 1041, 1044, 1048, 
	1052, 1054, 1056, 1058, 1065, 1067, 1069, 1071, 
	1073, 1075, 1077, 1080, 1084, 1089, 1093, 1098
};

static const short _command_indicies[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 
	8, 9, 10, 11, 12, 13, 5, 14, 
	15, 16, 17, 1, 18, 19, 20, 1, 
	21, 1, 22, 1, 23, 1, 24, 1, 
	25, 26, 1, 25, 1, 27, 1, 28, 
	29, 30, 31, 32, 33, 34, 35, 36, 
	37, 1, 38, 39, 1, 40, 1, 41, 
	42, 1, 43, 1, 44, 45, 1, 46, 
	1, 47, 1, 48, 1, 49, 1, 50, 
	1, 51, 1, 52, 1, 53, 1, 54, 
	1, 55, 1, 56, 1, 57, 58, 1, 
	59, 1, 60, 1, 61, 1, 62, 1, 
	63, 1, 64, 1, 65, 66, 1, 67, 
	1, 68, 1, 69, 1, 70, 1, 71, 
	1, 72, 1, 73, 1, 74, 1, 75, 
	75, 1, 76, 77, 78, 79, 80, 81, 
	82, 83, 84, 85, 86, 87, 88, 89, 
	90, 91, 92, 93, 94, 95, 1, 96, 
	1, 97, 1, 98, 1, 99, 1, 100, 
	1, 101, 1, 102, 1, 103, 1, 104, 
	1, 105, 1, 106, 1, 107, 108, 109, 
	110, 111, 1, 112, 1, 113, 1, 114, 
	1, 115, 1, 116, 1, 117, 1, 118, 
	1, 119, 1, 120, 1, 121, 1, 122, 
	1, 123, 1, 124, 1, 125, 1, 126, 
	1, 127, 128, 1, 129, 1, 130, 131, 
	1, 130, 132, 1, 130, 1, 127, 133, 
	1, 127, 1, 134, 135, 136, 135, 135, 
	135, 135, 1, 137, 1, 138, 1, 139, 
	139, 139, 139, 1, 140, 1, 141, 1, 
	142, 1, 143, 1, 144, 1, 145, 1, 
	146, 1, 147, 148, 148, 148, 148, 1, 
	147, 1, 149, 149, 149, 149, 1, 150, 
	151, 1, 150, 1, 152, 153, 154, 155, 
	156, 156, 156, 155, 156, 156, 1, 157, 
	157, 158, 1, 158, 1, 159, 1, 160, 
	1, 161, 1, 162, 1, 163, 1, 164, 
	1, 165, 1, 166, 1, 167, 168, 169, 
	167, 1, 168, 169, 1, 170, 1, 171, 
	172, 172, 173, 1, 174, 1, 171, 1, 
	171, 172, 172, 1, 175, 175, 176, 1, 
	176, 1, 177, 1, 178, 1, 179, 1, 
	180, 1, 181, 1, 182, 1, 183, 1, 
	184, 1, 185, 185, 186, 1, 186, 1, 
	187, 1, 188, 188, 1, 189, 1, 190, 
	1, 191, 192, 192, 1, 193, 1, 194, 
	1, 195, 1, 196, 1, 194, 197, 197, 
	198, 1, 199, 1, 194, 198, 200, 1, 
	194, 198, 1, 201, 202, 203, 201, 204, 
	1, 202, 202, 203, 202, 204, 1, 204, 
	1, 205, 206, 205, 205, 205, 1, 207, 
	1, 208, 1, 209, 192, 205, 210, 205, 
	205, 205, 1, 205, 205, 205, 205, 1, 
	211, 212, 213, 214, 1, 215, 1, 216, 
	1, 217, 1, 218, 1, 219, 1, 220, 
	220, 220, 221, 220, 220, 222, 222, 220, 
	220, 222, 222, 220, 222, 1, 223, 1, 
	224, 1, 225, 1, 227, 226, 227, 228, 
	226, 227, 229, 230, 231, 232, 233, 234, 
	235, 236, 237, 238, 239, 240, 232, 241, 
	242, 243, 244, 226, 227, 245, 246, 247, 
	226, 248, 226, 249, 226, 250, 226, 227, 
	251, 226, 252, 253, 226, 252, 226, 254, 
	226, 227, 255, 256, 257, 258, 259, 260, 
	261, 262, 263, 264, 226, 265, 266, 226, 
	267, 226, 268, 269, 226, 270, 226, 227, 
	271, 272, 226, 227, 273, 226, 227, 274, 
	226, 227, 275, 226, 227, 276, 226, 227, 
	277, 226, 278, 226, 279, 226, 227, 280, 
	226, 227, 281, 226, 227, 282, 226, 227, 
	283, 284, 226, 227, 285, 226, 227, 286, 
	226, 227, 287, 226, 227, 288, 226, 227, 
	289, 226, 290, 226, 227, 291, 292, 226, 
	227, 293, 226, 227, 294, 226, 227, 295, 
	226, 227, 296, 226, 227, 297, 226, 298, 
	226, 299, 226, 300, 226, 227, 301, 301, 
	226, 227, 302, 303, 304, 305, 306, 307, 
	308, 309, 310, 311, 312, 313, 314, 315, 
	316, 317, 318, 319, 320, 321, 226, 322, 
	226, 323, 226, 324, 226, 325, 226, 326, 
	226, 327, 226, 328, 226, 329, 226, 330, 
	226, 331, 226, 332, 226, 227, 333, 334, 
	335, 336, 337, 226, 338, 226, 339, 226, 
	340, 226, 341, 226, 342, 226, 343, 226, 
	344, 226, 345, 226, 346, 226, 347, 226, 
	348, 226, 349, 226, 350, 226, 227, 351, 
	226, 227, 352, 226, 227, 353, 354, 226, 
	227, 355, 226, 356, 357, 226, 356, 358, 
	226, 356, 226, 227, 353, 359, 226, 227, 
	353, 226, 227, 360, 361, 362, 361, 361, 
	361, 361, 226, 363, 226, 364, 226, 227, 
	365, 365, 365, 365, 226, 227, 366, 226, 
	227, 367, 226, 227, 368, 226, 227, 369, 
	226, 370, 226, 227, 371, 226, 372, 226, 
	373, 374, 374, 374, 374, 226, 373, 226, 
	227, 375, 375, 375, 375, 226, 376, 377, 
	226, 376, 226, 227, 378, 379, 380, 381, 
	382, 382, 382, 381, 382, 382, 226, 383, 
	227, 383, 384, 226, 227, 384, 226, 227, 
	385, 226, 227, 386, 226, 227, 387, 226, 
	227, 388, 226, 227, 389, 226, 227, 390, 
	226, 227, 391, 226, 392, 226, 393, 227, 
	394, 395, 393, 226, 227, 394, 395, 226, 
	227, 396, 226, 397, 398, 398, 399, 226, 
	227, 400, 226, 397, 226, 397, 398, 398, 
	226, 401, 227, 401, 402, 226, 227, 402, 
	226, 227, 403, 226, 227, 404, 226, 227, 
	405, 226, 227, 406, 226, 227, 407, 226, 
	227, 408, 226, 227, 409, 226, 410, 226, 
	411, 227, 411, 412, 226, 227, 412, 226, 
	227, 413, 226, 227, 414, 414, 226, 227, 
	415, 226, 227, 416, 226, 227, 417, 418, 
	418, 226, 227, 419, 226, 420, 226, 227, 
	421, 226, 227, 422, 226, 420, 423, 423, 
	424, 226, 227, 425, 226, 420, 424, 426, 
	226, 420, 424, 226, 427, 227, 428, 429, 
	427, 430, 226, 428, 227, 428, 429, 428, 
	430, 226, 227, 430, 226, 432, 431, 433, 
	431, 431, 431, 226, 227, 434, 226, 227, 
	435, 226, 436, 418, 432, 431, 437, 431, 
	431, 431, 226, 432, 431, 431, 431, 431, 
	226, 227, 438, 439, 440, 441, 226, 442, 
	226, 443, 226, 444, 226, 445, 226, 227, 
	446, 226, 227, 447, 447, 447, 448, 447, 
	447, 222, 222, 447, 447, 222, 222, 447, 
	222, 226, 449, 226, 227, 450, 226, 451, 
	226, 227, 452, 453, 454, 226, 227, 455, 
	226, 456, 226, 457, 226, 227, 458, 459, 
	460, 461, 462, 463, 226, 464, 226, 465, 
	226, 466, 226, 227, 467, 226, 227, 468, 
	226, 227, 469, 226, 227, 470, 470, 226, 
	471, 472, 473, 1, 474, 1, 475, 1, 
	476, 1, 477, 478, 479, 480, 481, 482, 
	1, 483, 1, 484, 1, 485, 1, 486, 
	1, 487, 1, 488, 1, 489, 489, 1, 
	490, 491, 0, 1, 490, 491, 0, 195, 
	1, 492, 227, 228, 226, 492, 227, 228, 
	421, 226, 492, 227, 228, 434, 226, 0
};

static const short _command_trans_targs[] = {
	2, 0, 3, 7, 10, 11, 45, 71, 
	80, 89, 91, 93, 96, 148, 153, 154, 
	327, 331, 4, 5, 6, 339, 339, 339, 
	8, 339, 9, 339, 12, 14, 16, 23, 
	24, 26, 35, 42, 43, 44, 339, 13, 
	339, 339, 15, 339, 17, 17, 18, 19, 
	20, 21, 22, 339, 339, 25, 339, 27, 
	28, 29, 29, 30, 31, 32, 33, 34, 
	339, 36, 36, 37, 38, 39, 40, 41, 
	339, 339, 339, 25, 46, 47, 48, 49, 
	50, 51, 52, 53, 54, 55, 56, 57, 
	63, 64, 65, 66, 67, 68, 69, 70, 
	339, 339, 339, 339, 339, 339, 339, 339, 
	339, 339, 339, 58, 59, 60, 61, 62, 
	339, 339, 339, 339, 339, 339, 339, 339, 
	339, 339, 339, 339, 339, 72, 73, 74, 
	78, 75, 339, 76, 77, 79, 81, 82, 
	83, 339, 339, 84, 85, 86, 87, 88, 
	339, 90, 339, 339, 92, 94, 339, 95, 
	97, 107, 114, 124, 139, 98, 99, 100, 
	101, 102, 103, 104, 105, 106, 339, 108, 
	109, 109, 110, 339, 111, 113, 112, 115, 
	116, 117, 118, 119, 120, 121, 122, 123, 
	339, 125, 126, 127, 128, 129, 130, 131, 
	133, 132, 339, 134, 135, 136, 132, 137, 
	138, 140, 141, 141, 142, 143, 146, 144, 
	145, 340, 147, 149, 150, 151, 152, 339, 
	339, 339, 339, 25, 155, 156, 158, 339, 
	157, 339, 158, 341, 160, 161, 165, 168, 
	169, 202, 228, 237, 246, 248, 250, 253, 
	305, 310, 311, 315, 319, 162, 163, 164, 
	341, 341, 341, 166, 341, 167, 341, 170, 
	172, 174, 181, 182, 183, 192, 199, 200, 
	201, 341, 171, 341, 341, 173, 341, 175, 
	175, 176, 177, 178, 179, 180, 341, 341, 
	158, 184, 185, 186, 186, 187, 188, 189, 
	190, 191, 341, 193, 193, 194, 195, 196, 
	197, 198, 341, 341, 341, 158, 203, 204, 
	205, 206, 207, 208, 209, 210, 211, 212, 
	213, 214, 220, 221, 222, 223, 224, 225, 
	226, 227, 341, 341, 341, 341, 341, 341, 
	341, 341, 341, 341, 341, 215, 216, 217, 
	218, 219, 341, 341, 341, 341, 341, 341, 
	341, 341, 341, 341, 341, 341, 341, 229, 
	230, 231, 235, 232, 341, 233, 234, 236, 
	238, 239, 240, 341, 341, 241, 242, 243, 
	244, 245, 341, 247, 341, 341, 249, 251, 
	341, 252, 254, 264, 271, 281, 296, 255, 
	256, 257, 258, 259, 260, 261, 262, 263, 
	341, 265, 266, 266, 267, 341, 268, 270, 
	269, 272, 273, 274, 275, 276, 277, 278, 
	279, 280, 341, 282, 283, 284, 285, 286, 
	287, 288, 290, 289, 341, 291, 292, 293, 
	289, 294, 295, 297, 298, 298, 299, 300, 
	343, 303, 301, 302, 342, 304, 306, 307, 
	308, 309, 341, 341, 341, 341, 158, 312, 
	313, 341, 314, 341, 316, 317, 318, 158, 
	341, 341, 320, 321, 322, 323, 324, 326, 
	341, 341, 341, 158, 325, 158, 158, 328, 
	329, 330, 25, 339, 339, 332, 333, 334, 
	335, 336, 338, 339, 339, 339, 25, 337, 
	25, 25, 1, 1, 159
};

static const short _command_trans_actions[] = {
	0, 0, 0, 0, 0, 89, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 117, 113, 115, 
	0, 47, 0, 121, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 79, 0, 
	75, 77, 0, 73, 0, 15, 13, 13, 
	13, 13, 13, 83, 95, 97, 0, 0, 
	0, 0, 15, 13, 13, 13, 13, 13, 
	91, 0, 15, 13, 13, 13, 13, 13, 
	81, 85, 93, 87, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	33, 39, 29, 65, 125, 51, 41, 27, 
	53, 71, 159, 0, 0, 0, 0, 0, 
	127, 135, 129, 131, 133, 31, 69, 37, 
	43, 123, 35, 45, 351, 0, 9, 0, 
	9, 11, 137, 11, 11, 9, 0, 57, 
	0, 21, 17, 57, 7, 7, 7, 7, 
	19, 0, 101, 23, 57, 57, 25, 0, 
	0, 0, 0, 57, 57, 0, 1, 1, 
	0, 3, 3, 0, 5, 5, 61, 0, 
	0, 15, 1, 227, 0, 1, 0, 0, 
	1, 1, 0, 3, 3, 0, 5, 5, 
	67, 0, 1, 1, 0, 3, 161, 0, 
	0, 49, 55, 5, 164, 0, 0, 0, 
	0, 0, 0, 15, 1, 0, 1, 3, 
	161, 55, 1, 0, 0, 0, 0, 157, 
	155, 151, 153, 143, 57, 0, 59, 139, 
	0, 141, 0, 145, 0, 0, 0, 0, 
	89, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	293, 287, 290, 0, 212, 0, 299, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 251, 0, 245, 248, 0, 242, 0, 
	15, 13, 13, 13, 13, 13, 257, 269, 
	97, 0, 0, 0, 15, 13, 13, 13, 
	13, 13, 263, 0, 15, 13, 13, 13, 
	13, 13, 254, 260, 266, 87, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 191, 200, 185, 230, 305, 215, 
	203, 182, 218, 239, 344, 0, 0, 0, 
	0, 0, 308, 320, 311, 314, 317, 188, 
	236, 197, 206, 302, 194, 209, 358, 0, 
	9, 0, 9, 11, 323, 11, 11, 9, 
	0, 57, 0, 173, 167, 57, 7, 7, 
	7, 7, 170, 0, 275, 176, 57, 57, 
	179, 0, 0, 0, 0, 57, 57, 0, 
	1, 1, 0, 3, 3, 0, 5, 5, 
	224, 0, 0, 15, 1, 347, 0, 1, 
	0, 0, 1, 1, 0, 3, 3, 0, 
	5, 5, 233, 0, 1, 1, 0, 3, 
	161, 0, 0, 49, 221, 5, 164, 0, 
	0, 0, 0, 0, 0, 15, 1, 0, 
	145, 1, 3, 161, 221, 1, 0, 0, 
	0, 0, 341, 338, 332, 335, 143, 57, 
	0, 326, 0, 329, 0, 0, 0, 105, 
	272, 296, 0, 0, 0, 0, 0, 0, 
	284, 278, 281, 149, 0, 147, 111, 0, 
	0, 0, 105, 99, 119, 0, 0, 0, 
	0, 0, 0, 109, 103, 107, 149, 0, 
	147, 111, 63, 0, 63
};

static const int command_start = 339;
static const int command_first_final = 339;
static const int command_error = 0;

static const int command_en_main = 339;


#line 242 "/Users/jordi/Documents/Arduino/esp32go/command.rl"





    
#line 882 "/dev/stdout"
	{
	cs = command_start;
	}

#line 887 "/dev/stdout"
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
#line 249 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(deg,(*p)); }
	break;
	case 1:
#line 250 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(min,(*p)); }
	break;
	case 2:
#line 251 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(sec,(*p)); }
	break;
	case 3:
#line 252 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(pulse,(*p));}
	break;
	case 4:
#line 253 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(ip3,(*p));}
	break;
	case 5:
#line 254 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(ip2,(*p));}
	break;
	case 6:
#line 255 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ADD_DIGIT(focus_counter,(*p));}
	break;
	case 7:
#line 256 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ neg=-1;}
	break;
	case 8:
#line 257 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_move(telescope,stcmd);}
	break;
	case 9:
#line 258 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{pulse_guide(telescope,stcmd,pulse);}
	break;
	case 10:
#line 259 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 buzzerOn(300);
					 sprintf(tmessage,"0");APPEND;}
	break;
	case 11:
#line 264 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_stop(telescope,stcmd);}
	break;
	case 12:
#line 265 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{select_rate(telescope,stcmd); }
	break;
	case 13:
#line 266 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
	break;
	case 14:
#line 268 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
	break;
	case 15:
#line 270 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintaz1(tmessage, st_current.az); APPEND;}
	break;
	case 16:
#line 271 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintde1(tmessage, st_current.alt); APPEND;}
	break;
	case 17:
#line 272 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintra1(tmessage, st_target.ra); APPEND;}
	break;
	case 18:
#line 273 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintde1(tmessage, st_target.dec); APPEND;}
	break;
	case 19:
#line 274 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintdate1(tmessage);APPEND;}
	break;
	case 20:
#line 275 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintsite();}
	break;
	case 21:
#line 276 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
	break;
	case 22:
#line 277 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintlong1(tmessage,telescope->longitude);APPEND;}
	break;
	case 23:
#line 278 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintlat1(tmessage,telescope->lat);APPEND;}
	break;
	case 24:
#line 281 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
	break;
	case 25:
#line 287 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{deg+=((*p)-'0')*6;}
	break;
	case 26:
#line 288 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprinttime1(tmessage);APPEND;}
	break;
	case 27:
#line 289 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
	break;
	case 28:
#line 290 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
	break;
	case 29:
#line 293 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{deg=deg*3600+min*60;}
	break;
	case 30:
#line 294 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{deg+=sec;}
	break;
	case 31:
#line 295 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{stcmd=(*p);}
	break;
	case 32:
#line 296 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{stcmd=(*p);mark=p;mark++;}
	break;
	case 33:
#line 297 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_date(min,deg,sec);}
	break;
	case 34:
#line 298 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
	break;
	case 35:
#line 299 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ telescope->time_zone=-deg*neg;}
	break;
	case 36:
#line 301 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
	break;
	case 37:
#line 302 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_time(deg,min,sec);}
	break;
	case 38:
#line 303 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"24#");APPEND;}
	break;
	case 39:
#line 304 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"50.0#");APPEND;}
	break;
	case 40:
#line 305 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(0,lmotor->speed,lmotor);}
	break;
	case 41:
#line 306 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(lmotor->max_steps,lmotor->speed,lmotor);}
	break;
	case 42:
#line 307 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(0,lmotor->speed_low,lmotor);}
	break;
	case 43:
#line 308 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(lmotor->max_steps,lmotor->speed_low,lmotor);}
	break;
	case 44:
#line 309 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(lmotor->position+(focus_counter*neg));}
	break;
	case 45:
#line 310 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotofocuser(focus_counter,lmotor);}
	break;
	case 46:
#line 311 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{stopfocuser(lmotor);}
	break;
	case 47:
#line 312 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if ((*p)=='0') pmotor=&focus_motor; else pmotor=&aux_motor; buzzerOn(300);}
	break;
	case 48:
#line 313 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if ((*p)=='F') lmotor=&focus_motor; else lmotor=&aux_motor;}
	break;
	case 49:
#line 314 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{lmotor->position=lmotor->target=focus_counter;}
	break;
	case 50:
#line 315 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%05d#",lmotor->position);APPEND;}
	break;
	case 51:
#line 316 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
	break;
	case 52:
#line 317 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%d#",lmotor->state<stop);APPEND;}
	break;
	case 53:
#line 318 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{gotoindex((*p)-'0');}
	break;
	case 54:
#line 319 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{buzzerOn(300);mount_goto_home(telescope);}
	break;
	case 55:
#line 320 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",(telescope->parked? "1" : "0"));APPEND;}
	break;
	case 56:
#line 321 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_home_set(telescope);}
	break;
	case 57:
#line 322 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_fix_home((*p),telescope);}
	break;
	case 58:
#line 323 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
	break;
	case 59:
#line 324 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
	break;
	case 60:
#line 325 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{meridianflip(telescope,(*p)=='w');}
	break;
	case 61:
#line 326 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{telescope->track=0;telescope->azmotor->targetspeed=0.0;}
	break;
	case 62:
#line 327 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{telescope->track=1;}
	break;
	case 63:
#line 328 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{;}
	break;
	case 64:
#line 329 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_home(telescope);}
	break;
	case 65:
#line 331 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (telescope->azmotor->slewing || telescope->altmotor->slewing) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
	break;
	case 66:
#line 332 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
	break;
	case 67:
#line 333 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage, "%02d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2)+((telescope->azmotor->slewing || telescope->altmotor->slewing)<<3));APPEND;}
	break;
	case 68:
#line 335 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",__DATE__) ;APPEND;}
	break;
	case 69:
#line 337 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{
			char version[6];
			versionFromCompileDate(version);
			sprintf(tmessage,"%s#",version) ;APPEND;
		}
	break;
	case 70:
#line 342 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{ sprintf(tmessage,"esp32go#") ;APPEND;}
	break;
	case 71:
#line 344 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%s#",__TIME__) ;APPEND;}
	break;
	case 72:
#line 345 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"43Eg#") ;APPEND;}
	break;
	case 73:
#line 346 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{setwifipad(ip3,ip2);}
	break;
	case 74:
#line 347 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{appcmd(stcmd);APPEND;}
	break;
	case 75:
#line 348 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{mount_park(telescope);ESP.restart();}
	break;
	case 76:
#line 349 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{if (((*p)>='0')&&((*p)<'3'))telescope->smode=(*p)-'0';
						else if (((*p)=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	break;
	case 77:
#line 358 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	break;
	case 78:
#line 368 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{setnunchuk((*p));}
	break;
	case 79:
#line 369 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{setflipmode((*p));}
	break;
	case 80:
#line 370 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_track_speed(telescope,1);}
	break;
	case 81:
#line 371 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_track_speed(telescope,2);}
	break;
	case 82:
#line 372 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_track_speed(telescope,3);}
	break;
	case 83:
#line 373 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{set_track_speed(telescope,4);}
	break;
	case 84:
#line 374 "/Users/jordi/Documents/Arduino/esp32go/command.rl"
	{sprintf(tmessage,"%c%c%c%c%d#",(telescope->is_tracking ? 'T':'t'),(telescope->parked ? 'P':'p'),(telescope->azmotor->slewing || telescope->altmotor->slewing ? 'S':'s'),(get_pierside(telescope)? 'W' : 'E'),get_track_speed(telescope)) ;APPEND;}
	break;
#line 1335 "/dev/stdout"
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

#line 467 "/Users/jordi/Documents/Arduino/esp32go/command.rl"


//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
		return 0;
};



