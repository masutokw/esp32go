#include "conf.h"
#ifdef OLED_DISPLAY
#include "oled.h"
extern mount_t *telescope;
SSD1306 display(0x3c, SDA_PIN, SCL_PIN);
extern time_t now;
extern int clients_connected;
void oledDisplay()
{
  char ra[20] = "";
  char de[20] = "";
  //write some information for debuging purpose to OLED display.
  display.clear();
  // display.drawString (0, 0, "ESP-8266 PicGoto++ 0.1");
  // display.drawString(0, 13, String(buff) + "  " + String(response));
  lxprintra(ra, sidereal_timeGMT_alt(telescope->longitude) * 15.0 * DEG_TO_RAD);
  display.drawString(0, 9, "LST " + String(ra));
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
  display.drawString(0, 36, "IP Clients: "+String(clients_connected));
//---------  
  display.display();
}
void oled_initscr(void)

{
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
  display.clear();
//  display.drawString(0, 0, "Connecting to " + String(ssid));
  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  display.drawString(0, 13, "Got IP! :" + ipStr);
  display.drawString(0, 26, "Waiting for Client");
  display.display();
}
#endif