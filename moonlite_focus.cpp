#include "conf.h"

#ifdef MOONLITE_FOCUS

#include "moonlite_focus.h"
#include "focus.h"
#include "webserver.h"
#define MOONLITE_PORT 10009
String moon_response;
char moon_buff[100]; //buffer to hold incoming packet

extern stepper focus_motor;
stepper *fmotor=&focus_motor;

WiFiServer moonserver(MOONLITE_PORT);
WiFiClient moonclient;

void moonlite_init(void)
{
  moonserver.begin();
  moonserver.setNoDelay(true);
}

long hexstr2long(String line)
{
  char buf[line.length() + 1];
  line.toCharArray(buf, line.length() + 1);
  return strtol(buf, NULL, 16);
}

void moonlite_cmd(String line)
{
  line=line.substring(1,line.length());// quitamos : y #
  moon_response="";
  String cmd, param;
  int len = line.length();
  if (len >= 2)
  {
    cmd = line.substring(0, 2);
  }
  else
  {
    cmd = line.substring(0, 1);
  }
  if (len > 2)
  {
    if(line.charAt(2)=='#')
      param = line.substring(3);
    else
      param = line.substring(2);
  }
//Serial.println("cmd["+cmd+"]");
//Serial.println("param["+param+"]");

  bool done=false;


  if(cmd.equalsIgnoreCase("GB") // led value - ignore
   || cmd.equalsIgnoreCase("GH") // half mode - ignore
  ) 
  {
    done = true;
    moon_response = "00#";
  }
  else if(cmd.equalsIgnoreCase("GV")) // firmware 
  {
    done = true;
    moon_response = "10#";
  }
  else if(cmd.equalsIgnoreCase("PH")) // home (move to zero)
  {
    done = true;
    gotofocuser(0);
  }
  else if(cmd.equalsIgnoreCase("GP")) // get the current motor position
  {
    done = true;
    char tempString[6];
    sprintf(tempString, "%04lX", focus_motor.position);
    moon_response = String(tempString)+"#";
  }
  else if(cmd.equalsIgnoreCase("GN")) // get the target motor position
  {
    done = true;
    char tempString[6];
    sprintf(tempString, "%04lX", focus_motor.target);
    moon_response = String(tempString)+"#";
  }
  else if(cmd.equalsIgnoreCase("GD")) // get the current motor speed, only values of 02, 04, 08, 10, 20
  {
    done = true;
    char tempString[6];
    sprintf(tempString, "%02X", focus_motor.speed);
    moon_response = String(tempString)+"#";
  }  
  else if(cmd.equalsIgnoreCase("GI")) // motor is moving - 01 if moving, 00 otherwise
  {
    done = true;
    // de momento, siempre que no
    moon_response = "00#";
  }
  else if(cmd.equalsIgnoreCase("SP")) // set current motor position
  {
    done = true;
    focus_motor.position = hexstr2long(param);
  }    
  else if(cmd.equalsIgnoreCase("SN")) // set new motor position
  {
    done = true;
    focus_motor.target = hexstr2long(param);
  }
  else if(cmd.equalsIgnoreCase("FG")) // initiate a move
  {
    done = true;
    gotofocuser(focus_motor.target);
  }
  else if(cmd.equalsIgnoreCase("FQ")) // STOP!
  {
    done = true;
    stopfocuser(fmotor);
  }
  else if(cmd.equalsIgnoreCase("GT")) // temperature 
  {
    done = true;
    int temperature = 20;
    int16_t t_int = (int16_t)(temperature*2);
    char tempString[5];
    sprintf(tempString, "%04X", (int16_t)t_int);
    moon_response = String(tempString)+"#";
  }
  else if(cmd.equalsIgnoreCase("GD")) // step mode 
  {
    done = true;
    moon_response = "08#";
  }
  else if(cmd.equalsIgnoreCase("GC")) // temp coefficient 
  {
    done = true;
    moon_response = "02#";
  }

  if(!done)
  {
//Serial.println("-->cmd ignored"); 
    //moon_response="#";
  }

}

void moonlite_handle(void)
{
  if (moonserver.hasClient()) 
    moonclient = moonserver.available();
  
    if(moonclient && moonclient.connected())
    {
      while (moonclient.available()) 
      {
        delay(1);
        size_t n = moonclient.available();
        moonclient.readBytes(moon_buff, n);
        moon_buff[n] = 0;
//Serial.println("CMD["+String(moon_buff)+"]");   
        moonlite_cmd(moon_buff);
//Serial.println("RESPONSE["+moon_response+"]");   
        moonclient.write(moon_response.c_str());
             }
    }
    else if(moonclient)
    {
      moonclient.stop();
    }
}

#endif // MOONLITE_FOCUS