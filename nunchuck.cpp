#include "conf.h"
#ifdef NUNCHUCK_CONTROL
#include <Arduino.h>
#include <Wire.h>
#include "mount.h"
#include "focus.h"
#include "tb6612.h"
extern mount_t *telescope;
extern stepper *pmotor,aux_motor,focus_motor;
extern int focusvolt;
#include "nunchuck.h"
#define ADDRESS 0x52
byte chuckbuffer[6];
bool n_disable=0;
int lastx, lasty, lastpress;
void nunchuck_init(int sda, int scl)
{
  Wire.begin(sda, scl);
  Wire.setClock(100000);
  Wire.beginTransmission(ADDRESS);
  Wire.write(0xF0);
  Wire.write(0x55);
  Wire.endTransmission();
  delay(1);
  Wire.beginTransmission(ADDRESS);
  Wire.write(0xFB);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(100);
  lastx = lasty = 1;
}
void nunchuck_disable(byte n){
  n_disable=n;
}

uint8_t nunchuck_read(void)
{ uint8_t cls;
  int count = 0;
  int pressed;
if (n_disable)  return 255;  
if (Wire.available()) {
    while (Wire.available())
      cls = Wire.read();
  }
 cls=Wire.requestFrom(ADDRESS, 6);
  while (Wire.available())
  {
    chuckbuffer[count] = Wire.read();
    count++;
  }
 
  if ((count == 6) && (chuckbuffer[4] != 0) && (chuckbuffer[4] != 255)&&(chuckbuffer[3] != 0) && (chuckbuffer[3] != 255)&&(chuckbuffer[5] != 129))
 //if (count == 6) //&& (chuckbuffer[4] != 0) && (chuckbuffer[4] != 255)&&(chuckbuffer[3] != 0) && (chuckbuffer[3] != 255))
  {
    pressed = ~chuckbuffer[5] & 0x03;
    if (pressed) lastpress = pressed;
    //if (pressed) telescope->srate = pressed;
    if (lastx != (chuckbuffer[0] /= 86))
    {
       switch (chuckbuffer[0])
      {
        case 0 :
          if (pressed==3) pmotor=&aux_motor;
          if (pressed == 2) telescope->srate = 3;
          else if (lastpress == 1) gotofocuser(pmotor->max_steps,pmotor->speed_low);
          else if (pressed == 0)  mount_move(telescope, 'e'); //Serial.println("Left");
          break;
        case 1 :
          if (lastpress == 1)
          {
            stopfocuser();
            mount_stop(telescope, 'w');
            mount_stop(telescope, 's');
          }
          else
          {
            if (pressed == 0)  mount_stop(telescope, 'w');
            stopfocuser();
          }; //Serial.println("CenterX");
          break;
        case 2 :
           if (pressed==3) pmotor=&focus_motor;
          if (pressed == 2) telescope->srate = 2 ;
          else if (lastpress == 1)  gotofocuser(0,pmotor->speed_low);
          else if (pressed == 0) mount_move(telescope, 'w'); //Serial.println("Rigth");
          break;
        default:
          break;

      }
    }

    lastx = chuckbuffer[0];
    if (lasty != (chuckbuffer[1]  /= 86))
    {
      switch (chuckbuffer[1] )
      {
        case 0 :
          if (pressed == 2) telescope->srate = 0;
          else if (lastpress == 1)  gotofocuser(pmotor->max_steps,pmotor->speed);
          else if (pressed == 0) mount_move(telescope, 's'); //Serial.println("Down");
          break;
        case 1 :
          if (lastpress == 1)
          {
            stopfocuser();
            mount_stop(telescope, 'w');
            mount_stop(telescope, 's');
          }
          else
          {
            if (pressed == 0) mount_stop(telescope, 's');
            stopfocuser();
          }; // Serial.println("CenterY");
          break;
        case 2 :
          if (pressed == 2) telescope->srate = 1;
          else if (lastpress == 1 )gotofocuser(0,pmotor->speed);
          else if (pressed == 0) mount_move(telescope, 'n'); //Serial.println("Up");
          break;
        default:
          break;

      }

    }
    lasty = chuckbuffer[1];
  }
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(1);
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
return cls;
}
#endif
