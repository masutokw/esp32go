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
  case 'g':sprintf(tmessage,"%f",telescope->rate[0][0]);
  break;
  case 'j':sprintf(tmessage,"%f",telescope->rate[0][1]);
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
    %%{
        machine command;
        write data;
    }%%




    %%{
#Actions
        action getgrads {ADD_DIGIT(deg,fc); }
        action getmin {ADD_DIGIT(min,fc); }
        action getsec {ADD_DIGIT(sec,fc); }
		action getpulse{ADD_DIGIT(pulse,fc);}
		action getip3{ADD_DIGIT(ip3,fc);}
		action getip2{ADD_DIGIT(ip2,fc);}
		action getfocuscounter{ADD_DIGIT(focus_counter,fc);}
        action neg { neg=-1;}
        action dir {mount_move(telescope,stcmd);}
		action pulse_dir{pulse_guide(telescope,stcmd,pulse);}
        action Goto {if (telescope->mount_mode)
					{goto_ra_dec(telescope,mount.ra_target*15.0*SEC_TO_RAD,mount.dec_target*SEC_TO_RAD);}
					 else mount_slew(telescope);
					 buzzerOn(300);
					 sprintf(tmessage,"0");APPEND;}
        action stop {mount_stop(telescope,stcmd);}
        action rate {select_rate(telescope,stcmd); }
        action return_ra {if (telescope->mount_mode) lxprintra1(tmessage, st_current.ra);
							else mount_lxra_str(tmessage,telescope); APPEND;}
        action return_dec {if (telescope->mount_mode) lxprintde1(tmessage, st_current.dec);
							else mount_lxde_str(tmessage,telescope); APPEND;}
        action return_az { lxprintaz1(tmessage, st_current.az); APPEND;}
        action return_alt {lxprintde1(tmessage, st_current.alt); APPEND;}
        action return_ra_target { lxprintra1(tmessage, st_target.ra); APPEND;}
        action return_dec_target {lxprintde1(tmessage, st_target.dec); APPEND;}
        action return_date {lxprintdate1(tmessage);APPEND;}
        action return_site { lxprintsite();}
        action ok  {sprintf(tmessage,"1");APPEND;deg=sec=min=0;}
        action return_longitude {lxprintlong1(tmessage,telescope->longitude);APPEND;}
        action return_lat {lxprintlat1(tmessage,telescope->lat);APPEND;}
        action return_sid_time { ;}

        action sync {if (telescope->mount_mode)
						align_sync_all(telescope,mount.ra_target,mount.dec_target);
						else
						{sync_eq(telescope);telescope->altmotor->slewing= telescope->azmotor->slewing=FALSE;}
						sprintf(tmessage,"sync#");APPEND;
						}
        action rafrac {deg+=(fc-'0')*6;}
        action return_local_time { lxprinttime1(tmessage);APPEND;}
		action return_sideral_time { lxprintra(tmessage, sidereal_timeGMT(telescope->longitude,telescope->time_zone) * 15.0 * DEG_TO_RAD);APPEND;strcat(response,"#");}
        action set_cmd_exec {set_cmd_exe(stcmd,(neg*(deg )));
                             sprintf(tmessage,"1");APPEND;deg=sec=min=0;
                            }
        action addmin {deg=deg*3600+min*60;}
        action addsec {deg+=sec;}
        action storecmd {stcmd=fc;}
		action markinput {stcmd=fc;mark=p;mark++;}
        action setdate {set_date(min,deg,sec);}
        action return_align{if (telescope->mount_mode==ALTAZ) sprintf(tmessage,"A");else if (telescope->track) sprintf(tmessage,"P"); else sprintf(tmessage,"L"); APPEND; }
		action set_gmt_offset{ telescope->time_zone=-deg*neg;}
#		action set_gmt_offset{ telescope->time_zone=deg*neg;}	
		action return_GMT_offset {lxprintGMT_offset(tmessage,telescope->time_zone );APPEND}
        action settime{set_time(deg,min,sec);}
		action return_formatTime{sprintf(tmessage,"24#");APPEND;}
		action return_tRate{sprintf(tmessage,"50.0#");APPEND;}
		action fmove_in {gotofocuser(0,lmotor->speed,lmotor);}
		action fmove_out {gotofocuser(lmotor->max_steps,lmotor->speed,lmotor);}
		action fmovel_in {gotofocuser(0,lmotor->speed_low,lmotor);}
		action fmovel_out {gotofocuser(lmotor->max_steps,lmotor->speed_low,lmotor);}
		action fmove_rel {gotofocuser(lmotor->position+(focus_counter*neg));}
		action fmove_to {gotofocuser(focus_counter,lmotor);}
		action fstop {stopfocuser(lmotor);}
		action select_focus {if (fc=='0') pmotor=&focus_motor; else pmotor=&aux_motor; buzzerOn(300);}
		action select_motor {if (fc=='F') lmotor=&focus_motor; else lmotor=&aux_motor;}
		action fsync_to{lmotor->position=lmotor->target=focus_counter;}
		action fquery{sprintf(tmessage,"%05d#",lmotor->position);APPEND;}
		action fquery_foc{sprintf(tmessage,"%05d#",focus_motor.position);APPEND;}
		action f_moving {sprintf(tmessage,"%d#",lmotor->state<stop);APPEND;}
		action fmoveindex{gotoindex(fc-'0');}
		action goto_home{buzzerOn(300);mount_goto_home(telescope);}
		action getparked{sprintf(tmessage,"%s#",(telescope->parked? "1" : "0"));APPEND;}
		action home{mount_home_set(telescope);}
		action getpierside {sprintf(tmessage,"%s#",(get_pierside(telescope)? "WEST" : "EAST"));APPEND;}
		action getflip {sprintf(tmessage,"%s#",(telescope->autoflip? "1" : "0"));APPEND;}
		action setpierside{meridianflip(telescope,fc=='w');}
		action set_land {telescope->track=0;telescope->azmotor->targetspeed=0.0;}
		action set_polar {telescope->track=1;}
		action set_altaz {;}
		action return_dst{if ((telescope->azmotor->slewing ||(telescope->altmotor->slewing))&&!(telescope->parked)) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
		#action return_dst{if (telescope->azmotor->slewing || telescope->altmotor->slewing) sprintf(tmessage,"|#");else sprintf(tmessage,"#") ;APPEND;}
		action return_track{sprintf(tmessage, telescope->is_tracking ? "1":"0");APPEND;}
		action return_tracks{sprintf(tmessage, "%d", telescope->is_tracking +(telescope->parked <<1)+(get_pierside(telescope)<<2));APPEND;}		
		action a_date {sprintf(tmessage,"012 24 2000#") ;APPEND;}
		action a_number {sprintf(tmessage,"01.0#") ;APPEND;}
		action a_product{ sprintf(tmessage,"esp32go#") ;APPEND;}
		action a_time {sprintf(tmessage,"00:00:00#") ;APPEND;}
		action a_firm {sprintf(tmessage,"43Eg#") ;APPEND;}
		action  set_ip {setwifipad(ip3,ip2);}
		action cmd_app {appcmd(stcmd);APPEND;}
		action syncmode {if ((fc>='0')&&(fc<'3'))telescope->smode=fc-'0';
						else if ((fc=='3')&&(telescope->mount_mode>EQ)){
							 telescope->is_tracking = FALSE;
							sync_target = TRUE;
							tak_init(telescope);
        					telescope->azmotor->targetspeed = 0.0;
							telescope->altmotor->targetspeed = 0.0;}
		
		}
	action writeconf {switch (stcmd){ 
						case 's':conf_write(mark,MOUNT_FILE); readconfig(telescope);break;
						case 'w':conf_write(mark,WIFI_FILE);break;
						case 'n':conf_write(mark,NETWORK_FILE);break;
						case 't':conf_write(mark,TMC_FILE);tmcinit();break;
						case 'f':conf_write(mark,AUX_FILE);readauxconfig();break;
						case 'd':conf_write(mark,WHEEL_FILE);read_wheel_config();break;
						
						}
						}
	action nunchuk {setnunchuk(fc);}
	action autoflip {setflipmode(fc);}	
# LX200  auxiliary terms syntax definitions
        sexmin =  ([0-5][0-9])$getmin@addmin ;
        sex= ([0-5][0-9] )$getsec@addsec ((('.'|',')digit{1,2}){,1})':'{0,1};
		grad=([\+] |''|space | [\-]@neg) ((digit @getgrads){1,3});
        deg = grad punct sexmin (any  sex)? ;
        RA = ([0-2] digit) $getgrads   (':'|'/') sexmin ('.'digit@rafrac | (':'|'/') sex) ;

#Lx200 commands syntax definition
        Poll= 'G'( 'R'%return_ra |
                   'D'%return_dec|
                   'r'%return_ra_target |
                   'd'%return_dec_target |
                   'Z'%return_az |
                   'A'%return_alt |
                   'G'%return_GMT_offset |
                   'L'%return_local_time |
                   'S'%return_sideral_time|
                   'C'%return_date|
                   'M'%return_site|
                   'g'%return_longitude|
                   't'%return_lat|
				   'T'%return_tRate|
				   'c'%return_formatTime|
				   'k'%return_track|
				   'K'%return_tracks|
				   'x'%return_ra %return_dec %return_az %return_alt %return_tracks %fquery_foc );
        Move = 'M' (([nsweh]@storecmd %dir)|
                    ('S'%Goto)|
                    ('g'[nsew]@storecmd digit{4}$getpulse %pulse_dir));

        Rate = 'R' [CGMS]@storecmd (''|[0-4]) %rate;
		Timezone='G'(''|space)([\+] | [\-]@neg)((digit @getgrads){1,2}) (('.'|',') digit)? %set_gmt_offset%ok;
        date ='C' (''|space)  (digit@getgrads){2} '/' (digit @getmin){2} '/' (digit @getsec){2}%setdate;
        time ='L' (''|space) (digit @getgrads){2}':'(digit@getmin){2} ':'(digit @getsec){2}%settime;

        Set='S'((([dazgt]@storecmd (''|space) deg ) |([rS]@storecmd (''|space) RA))%set_cmd_exec|
                 Timezone |
                 date |
                 time);

		Align='A'(('L'%set_land)|
                 ('P'%set_polar)|
                 ('A'%set_altaz));

		Distance='D'%return_dst;
        Sync = "CM"(''|'R')%sync;
        Stop ='Q' (''|[nsew])@storecmd %stop;
       	ACK = 0x06 @return_align;

#focuser
		f_in = ('-'%fmovel_in)|('--'%fmove_in);
		f_out = ('+'%fmovel_out)| ('++'%fmove_out);
		f_stop ='Q'%fstop;
		f_rel='P'([\+]|[\-]@neg)digit{5}$getfocuscounter %fmove_rel;
		f_sync='LS1'([\+]|[\-]@neg)digit{5}$getfocuscounter %fsync_to;
		f_query='p'%fquery;
		f_go='A'([\+]|[\-]@neg)digit{5}$getfocuscounter %fmove_to;
		f_moving='B'%f_moving;
		f_speed=digit;
		f_select='s'[0,1]$select_focus;
		f_index='I'[0-8]$fmoveindex;
		Focuser=('F'|'X')$select_motor(f_in|f_out|f_go|f_query|f_stop|f_sync|f_rel|f_moving|f_select|f_index);
# custom
		Park = ('pH'%home)|('hP'%goto_home)|('pS'%getpierside)|('pF'%getflip)  |(('ps')('e'|'w')@setpierside)|('pnk'('0'|'1')@nunchuk)|('pa'('0'|'1')@autoflip)|('PP'%getparked);
		IP_PAD ='IP' ((digit$getip3){1,3} '.' (digit$getip2){1,3})%set_ip;
		Sync_mode='a'digit$syncmode;
#autostar
        Autostar='GV'('D'%a_date | 'N'%a_number | 'P'%a_product | 'T'%a_time | 'F'%a_firm) ;
		
#app commands
		cgfsend=[zagjATFD]@storecmd %cmd_app;
		cgfwrite=[snwtfd]@markinput extend* %writeconf ;
		Appcmd= 'c'(cgfsend | cgfwrite);
		
		

		
#main
		main :=   ((ACK|''|'#')':'(Set | Move | Stop|Rate | Sync | Poll| Focuser | Align | Park | Distance | Autostar | IP_PAD | Sync_mode| Appcmd)  '#')* ;
		#main :=   ACK | (( ACK | ''|'#')':'(Set | Move | Stop|Rate | Sync | Poll| Focuser | Align | Park | Distance | Autostar)'#')* ;
		
# Initialize and execute.
        write init;
        write exec;
    }%%

//---------------------------------------------------------------------------------------------------------------------
    if ( cs < command_first_final )
        //	fprintf( stderr, "LX command:  error\n" );

        return  neg;
		return 0;
};



