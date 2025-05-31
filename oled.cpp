#include "conf.h"
#ifdef OLED_DISPLAY
#include "oled.h"
extern mount_t *telescope;
#ifdef OLED_13
  SH1106 display(0x3c, SDA_PIN, SCL_PIN);
#else
  SSD1306 display(0x3c, SDA_PIN, SCL_PIN);
#endif
extern time_t now;
extern int clients_connected;
static const unsigned long REFRESH_INTERVAL = 1000; // (ms) refresh oled every second
static unsigned long lastRefreshTime = 0;
bool oled_disabled=false;
#include "tb6612.h"
extern stepper focus_motor,aux_motor,*pmotor;

void oled_check()
{
  byte error;
  byte address = 0x3c;
  Wire.begin();
  Wire.beginTransmission(address);
  error = Wire.endTransmission();
  if (error == 0)
  {
//Serial.println("OLED found!");
    return;
  }
  oled_disabled=true;
}

void oledDisplay()
{
  if(oled_disabled)
  {
//Serial.println("oled IS disabled");
    return;
  }
  if(millis() - lastRefreshTime < REFRESH_INTERVAL)
    return;
	lastRefreshTime = millis();

  char ra[20] = "";
  char de[20] = "";
  //write some information for debuging purpose to OLED display.
  display.clear();
  // display.drawString (0, 0, "ESP-8266 PicGoto++ 0.1");
  // display.drawString(0, 13, String(buff) + "  " + String(response));
  lxprintra(ra, sidereal_timeGMT_alt(telescope->longitude) * 15.0 * DEG_TO_RAD);
  //display.drawString(0, 9, "LST " + String(ra));
  display.drawString(0, 9, "Lon: "+String(int(telescope->longitude)) + "." + String(getDecimal(telescope->longitude))+"  Lat: "+String(int(telescope->lat)) + "." + String(getDecimal(telescope->lat)) );

   lxprintra(ra, calc_Ra(telescope->azmotor->position, telescope->longitude));
   lxprintde(de, telescope->altmotor->position);

  display.drawString(0, 50, "RA:" + String(ra) + " DE:" + String(de));
  lxprintde(de, telescope->azmotor->delta);
  //display.drawString(0, 36, String(de)); // ctime(&now));
  display.drawString(0, 18, "MA:" + String(telescope->azmotor->counter) + " MD:" + String(telescope->altmotor->counter));
  //display.drawString(0, 27, "Dt:" + String(digitalRead(16)));//(telescope->azmotor->slewing));
  //display.drawString(0, 27, "Dt:" + String(digitalRead(13)) + " Rate:" + String(telescope->srate));
  //unsigned int n= pwd.length();
  //display.drawString(0, 32,String(pw)+ " "+ String(n));
  display.drawString(0, 0, ctime(&now));
//---------
  display.drawString(0, 28, "T["+String(telescope->is_tracking ? "1":"0")+String(telescope->track ? "1":"0")+"]  S["+String((telescope->azmotor->slewing || telescope->altmotor->slewing) ? "1":"0")+"]  PSide["+String(get_pierside(telescope) ? "W":"E")+"]  Flip["+String(telescope->autoflip ? "1":"0")+"]");
  display.drawString(0, 38, "F["+String(pmotor==&focus_motor ? "FOC":"AUX")+"]  IP Clients: "+String(clients_connected));
//---------  
  display.display();
}

void oled_initscr(void)
{
  oled_check();
  if(oled_disabled)
    return;
  display.init();
//    display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
//  display.drawString(0, 0, "Connecting to " + String(ssid));
  display.display();
}

void oled_waitscr(void)
{
  if(oled_disabled)
    return;
  display.clear();
//  display.drawString(0, 0, "Connecting to " + String(ssid));
  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  display.drawString(0, 13, "Got IP! :" + ipStr);
  display.drawString(0, 26, "Waiting for Client");
  display.display();
}
#endif