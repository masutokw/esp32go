#include "time.h"
#include "taki.h"
#include "webserver.h"
#include "nunchuck.h"
#include "tb6612.h"
#ifdef IR_CONTROL
extern uint32_t truecode, lasti;
extern byte cmd_map [];
const String codes[31] = {"EAST", "WEST", "NORTH", "SOUTH", "OK", "FOCUS_F", "FOCUS_B", "GUIDE", "CENTER", "FIND", "SLEW", "N_EAST", "N_WEST", "S_EAST",
                          "S_WEST", "TRACK", "UNTRACK", "B_1", "B_2", "B_3", "B_4", "B_5", "B_6", "B_7", "B_8", "B_9", "B_0", "GO_TO", "CLEAR", "FLIP_W", "FLIP_E"
                         };
#endif
extern int clients_connected;
extern uint64_t  period_alt;
extern char sync_target;
extern int  focuspeed;
extern int  focuspeed_low;
extern int focusmax;
extern  int align_star_index;
extern c_star st_1, st_2;
extern  time_t init_time;
extern stepper focus_motor;
extern  int stepcounter1, stepcounter2;
extern hw_timer_t * timer_az;
extern hw_timer_t * timer_alt;
extern int8_t focusinv;
extern int focusvolt;
extern int azbackcounter, altbackcounter;
char  buffer[30];
extern c_star st_current;
//extern bool n_disable;
extern WiFiClient serverClients[MAX_SRV_CLIENTS];

String getContentType(String filename)
{
  if (serverweb.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
void handleConfig(void)
{
  char temp [4500];
  char msg[43];
  // msg.reserve(500);
  time_t now;
  now = time(nullptr);
  if (serverweb.hasArg("MAXCOUNTER") && serverweb.hasArg("MAXCOUNTER_ALT"))
  {
    snprintf(temp, 300, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
             serverweb.arg("MAXCOUNTER"), serverweb.arg("MAXCOUNTER_ALT"),
             serverweb.arg("GUIDE"), serverweb.arg("CENTER"), serverweb.arg("FIND"), serverweb.arg("SLEW"),
             serverweb.arg("GUIDEA"), serverweb.arg("CENTERA"), serverweb.arg("FINDA"), serverweb.arg("SLEWA"),
             serverweb.arg("PRESCALER"),
             serverweb.arg("LONGITUDE"), serverweb.arg("LATITUDE"), serverweb.arg("TIMEZONE"),
             serverweb.arg("FOCUSMAX"), serverweb.arg("FOCUSPEEDLOW"), serverweb.arg("FOCUSPEED"),
             serverweb.arg("RAMP"), serverweb.arg("RAMPA"), serverweb.arg("BACK_AZ"), serverweb.arg("BACK_ALT"),
             serverweb.arg("MOUNT"), serverweb.arg("TRACK"), serverweb.arg("AUTOFLIP"), String(serverweb.hasArg("INVAZ")),
             String(serverweb.hasArg("INVALT")), serverweb.arg("PWR_DIR"), String(serverweb.hasArg("ACTAZ")), String(serverweb.hasArg("ACTALT")));

    /*  String temp = serverweb.arg("SLEW");
      telescope->rate[3][0] = temp.toFloat();
      temp = serverweb.arg("SLEWA");
      telescope->rate[3][1] = temp.toFloat();*/
    File f = SPIFFS.open("/mount.config", "w");
    if (!f)
    {

      snprintf(msg, 42, "File open failed");
    }
    else
    {
      f.println(temp);
      f.close ();
      readconfig(telescope);
      now = time(nullptr);
      snprintf(msg, 42, "Config Saved at %s ", ctime(&now));

    }

  }
  snprintf(temp, 4500,

           "<html><style> %s </style>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
<body  bgcolor=\"#000000\" text=\"%s\"><form action='/config' method='POST'><h2>ESP32go NUNCHUK</h2>\
<fieldset style=\"width:15%; border-radius:15px\"> <legend>Mount parameters:</legend><table style='width:250px'>\
<tr><th><button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Main</button></th><th>Azimuth</th><th>Altitude</th></tr>\
<tr><td>Counter</td><td> <input type='number' name='MAXCOUNTER' class=\"text_red\" value='%d'></td><td> <input type='number' name='MAXCOUNTER_ALT'  class=\"text_red\" value='%d'></td></tr>\
</table><br>\
\
<table style='width:250px'><tr><th>Rate X</th><th>RA/AZ</th><th>Dec/Alt</th></tr>\
<tr><td>Guide</td><td><input type='number' step='0.01' name='GUIDE' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='GUIDEA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Center</td><td><input type='number' step='0.01' name='CENTER' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='CENTERA'  class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Find</td><td><input type='number' step='0.01' name='FIND' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='FINDA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Slew</td><td><input type='number' step='0.01' name='SLEW' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='SLEWA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Ramp</td><td><input type='number' step='0.01' name='RAMP' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='RAMPA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>BackSlash</td><td><input type='number' step='1' name='BACK_AZ' class=\"text_red\" value='%d'></td><td><input type='number' step='1' name='BACK_ALT' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Prescaler</td><td><input type='number' step='0.0001' name='PRESCALER' class=\"text_red\" value='%.5f' uSec</td></tr>\
<tr><td>EQ Track</td><td><input type='number' min='0' max='4' title='0.No track 1-Sideral 2-Solar 3-Lunar 4-King.' step='1' name='TRACK'  class=\"text_red\" value ='%d' </td></tr>\
<tr><td>EQ<input type='radio' name='MOUNT' value='0'   %s ></td><td>ALT-AZ<input type='radio' name='MOUNT' value='1' %s ></td><td>EQ2-stars<input type='radio' name='MOUNT' value='2' %s ></td></tr>\
<tr><td>FLIP<input type='checkbox' name='AUTOFLIP' value='1'  %s ></td><td>Invert Az<input type='checkbox' name='INVAZ' value='1' %s ></td><td>Invert Alt<input type='checkbox' name='INVALT' value='1' %s ></td></tr>\
<tr><td>AZ<input type='checkbox' name='ACTAZ' value='1' %s></td><td>ALT<input type='checkbox' name='ACTALT' value='1' %s  ></td></tr>\
\
</table><input type='submit' name='SUBMIT' class=\"button_red\" value='Save'></fieldset>\
<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Focuser</legend>\
<table style='width:250px'><tr><td>Focus Max:</td><td><input type='number'step='1' name='FOCUSMAX' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Low Speed:</td><td><input type='number'step='1' name='FOCUSPEEDLOW' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Speed</td><td><input type='number'step='1' name='FOCUSPEED' class=\"text_red\" value='%d'></td></tr><tr>\
<td>Volt</td><td><input type='number'step='1' name='PWR_DIR' class=\"text_red\" value='%d'></td></tr>\
<tr><td><button onclick=\"location.href='/focus'\" class=\"button_red\" type=\"button\">Focuser set</button></td></tr></table>\
</fieldset>\
<fieldset style=\"width:15% ; border-radius:15px\"> <legend>Geodata</legend>\
<table style='width:250px'><tr><td>Longitude:</td><td><input type='number' step='any' name='LONGITUDE' class=\"text_red\" value='%.4f'></td></tr>\
<tr><td>Latitude:</td><td><input type='number'step='any'  name='LATITUDE' class=\"text_red\"  value='%.4f'></td></tr>\
<tr><td>GMT offset:</td><td><input type='number'step='1' name='TIMEZONE' class=\"text_red\" value='%d'></td></tr></table>\
</fieldset></form><br>\
<br>Load Time : %s \
<br>%s \
</body></html>",
           BUTTTEXTT, TEXT_COLOR,
           telescope->azmotor->maxcounter, telescope->altmotor->maxcounter,
           telescope->rate[0][0], telescope->rate[0][1], telescope->rate[1][0], telescope->rate[1][1],
           telescope->rate[2][0], telescope->rate[2][1], telescope->rate[3][0], telescope->rate[3][1],
           telescope->azmotor->acceleration / SEC_TO_RAD, telescope->altmotor->acceleration / SEC_TO_RAD,
           telescope->azmotor->backlash, telescope->altmotor->backlash, telescope->prescaler, telescope->track,
           telescope->mount_mode == EQ ?  "checked" : "", telescope->mount_mode == ALTAZ ?  "checked" : "", telescope->mount_mode == ALIGN ?  "checked" : "",
           telescope->autoflip ? "checked" : "", telescope->azmotor->cw ?  "checked" : "", telescope->altmotor->cw ?  "checked" : "", telescope->azmotor->active ?  "checked" : "", telescope->altmotor->active ?  "checked" : "",
           focusmax, focuspeed_low, focuspeed, focusvolt * focusinv,
           telescope->longitude, telescope->lat, telescope->time_zone, ctime(&now), &msg);
  serverweb.send(200, "text/html", temp);
}


void handlePark(void)
{
  time_t now;
  now = time(nullptr);
  mount_park(telescope);
  String content =  "<html>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"#FFFFFF\"><h2>ESP32go PARKED</h2><br>";
  content += "Mount parked  ,position saved on EEPROM.<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "Alt res:" + String(RAD_TO_ARCS * telescope->altmotor->resolution) + "<br>";
  content += "Az res:" + String(RAD_TO_ARCS *  telescope->azmotor->resolution) + "<br>";
  content += "Sideral:" + String(sidereal_timeGMT (telescope->longitude, telescope->time_zone)) + "<br>";
  content += "Time :" + String(ctime(&now)) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}
void handleHome(void)
{
  time_t now;
  now = time(nullptr);
  if (serverweb.hasArg("HOME"))
  {
    String net = serverweb.arg("HOME");
    char cmd = net.toInt();
    switch (cmd)
    {
      case 0:
        mount_home_set(telescope);
        break;
      case 1:
        mount_goto_home(telescope);
        break;
      case 2: 
      if (serverClients[0]) serverClients[0].stop();
      break;
    }
  }
  String content =  "<html>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"#FFFFFF\"><h2>ESP32go++ PARKED</h2><br>";
  content += "Mount parked  ,position saved on EEPROM.<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "Alt res:" + String(RAD_TO_ARCS * telescope->altmotor->resolution) + "<br>";
  content += "Az res:" + String(RAD_TO_ARCS *  telescope->azmotor->resolution) + "<br>";
  content += "Sideral:" + String(sidereal_timeGMT (telescope->longitude, telescope->time_zone)) + "<br>";
  content += "Time :" + String(ctime(&now)) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}
void handleSync(void)
{
  String msg, msg1;
  time_t rtc;
  if (serverweb.hasArg("GMT"))
  {
    msg1 = serverweb.arg("OFFSET");
    msg = serverweb.arg("GMT");
    rtc = msg.toInt();
    //msg = serverweb.arg("OFFSET");
    timeval tv = { rtc, 0 };
    //timezone tz = {msg.toInt()  ,0 };
    settimeofday(&tv, nullptr);
    rtc = time(nullptr);
    if (telescope->mount_mode == EQ)
    {
      sdt_init(telescope->longitude, telescope->time_zone);
    }
    else
    {
      telescope->is_tracking = FALSE;
      sync_target = TRUE;
      tak_init(telescope);
      //telescope->is_tracking = TRUE;
      telescope->azmotor->targetspeed = 0.0;
      telescope->altmotor->targetspeed = 0.0;
    }

  }
  String content =  "<!DOCTYPE html><html>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"#FFFFFF\"><h2>ESP32Go Sync </h2><br>";
  content += "<p id=\"fecha\">" + msg + " " + String(ctime(&rtc)) + "</p>";
  content += "<p id=\"fecha\">" + String(rtc) + "</p>";
  
  content += "<button onclick=\"location.href='/starinstructions'\"class=\"button_red\" type=\"button\">Continue 2-star alignment</button><br><br>";
  
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);

}
void handleTime(void)
{
  time_t now;
  now = time(nullptr);
  String content =  "<!DOCTYPE html><html>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"#FFFFFF\"><h2>ESP32Go++ Time </h2><br>";
  content += "<form id=\"frm1\" action=\"/sync\">";
  content += "<button onclick=\"myFunction()\">Synchronize now!</button>";
  content += "<input type=\"number\" name=\"GMT\" id=\"gmt\"  type=\"hidden\"><br><br>";
  content += "<input type=\"number\" name=\"OFFSET\" type=\"hidden\" id=\"offset\"><br><br>";
  content += "</form><p id=\"fecha\"></p>";
  content += "</form><p id=\"now\">" + String(now) + "</p>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "<script> var d = new Date();var n = ~~(Date.now()/1000);document.getElementById(\"gmt\").value = n; var n = d.toTimeString()+\" \"+d.toDateString();";
  content += "document.getElementById(\"fecha\").innerHTML = n; var o = d.getTimezoneOffset();";
  content += "document.getElementById(\"offset\").value =-o;";
  content += "function myFunction() {var d = new Date();var n = ~~(Date.now()/1000);document.getElementById(\"gmt\").value =n;";
  content += " document.getElementById(\"frm1\").submit();}</script>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);

}
void handleRestart(void)
{
  mount_park(telescope);
  String content =   "<html>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"#FFFFFF\"><h2>ESP-PGT++ restarted</h2><br>";
  content += "Mount parked  ,position saved on EEPROM.<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Home</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
  delay(1000);
  ESP.restart();
}
#ifdef NUNCHUCK_CONTROL
void handleNunchuk(void)
{
  String action = "disabled";
  if (serverweb.hasArg("ENABLE"))
  {
    nunchuck_init(SDA_PIN, SCL_PIN);
    nunchuck_disable(FALSE);
    action = "restarted";
  }
  else
    nunchuck_disable(TRUE);

  String content =   "<html>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"#FFFFFF\"><h2>ESP32go Nunchuck " + action + "</h2><br>";

  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Home</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);

}
#endif

void handleStar( void)
{
  String msg, txt;
  if (serverweb.hasArg("Mode"))
  {
    msg = serverweb.arg("Mode");
    telescope->smode = msg.toInt();
  }
  switch (telescope->smode)
  {
    case 0:
      txt = "Normal Sync";
      break;
    case 1:
      txt = "Set Star1";
      break;
    case 2:
      txt = "Set Star2";
      break;
  }

  String content =   "<html><style>" + String(BUTT) + String(TEXTT) + "</style>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"" + String(TEXT_COLOR) + "\"><h2>Sync mode</h2><br>";
  content += "Last selected star " + String(align_star_index) + "<br>";
  content += "Sync mode set to:" + txt + "<br><br>";

  content += "<a href=\"/instructions\" target=\"_instructions\"><button class=\"button_red\" type=\"button\">Instructions</button></a><br><br>";

  content += "<fieldset style=\"width:50% ; border-radius:15px\"> <legend>Data</legend>";
  content += "<button onclick=\"location.href='/Align?Mode=1'\"   class=\"button_red\" type=\"button\">Star1 Select</button>";
  content += "<br>RA: " + String(st_1.ra * RAD_TO_DEG / 15.0) + "  Dec: " + String(st_1.dec * RAD_TO_DEG) + "<br>";
  content += "Az:  " + String(st_1.az * RAD_TO_DEG) + " Alt: " + String(st_1.alt * RAD_TO_DEG) + "<br>";
  content += "Time " + String(st_1.timer_count) + "<br><br>";
  content += "<button onclick=\"location.href='/Align?Mode=2'\"  class=\"button_red\" type=\"button\">Star2 Select</button>";
  content += "<br>RA:" + String(st_2.ra * RAD_TO_DEG / 15.0) + "  Dec:" + String(st_2.dec * RAD_TO_DEG) + "<br>";
  content += "Alt " + String(st_2.az * RAD_TO_DEG) + " Alt:" + String(st_2.alt * RAD_TO_DEG) + "<br>";
  content += "Time: " + String(st_2.timer_count) + "</fieldset><br>";
  content += "<button onclick=\"location.href='/Align?Mode=0'\"  class=\"button_red\" type=\"button\">Sync mode</button><br>";
  content += "<button onclick=\"top.location.href='/time'\"  class=\"button_red\" type=\"button\">Update Date/Time</button><br>";
  content += "<button onclick=\"top.location.href='/'\"  class=\"button_red\" type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);

}
void handleNetwork( void)
{ if (serverweb.hasArg("SSID") && serverweb.hasArg("PASSWORD"))
  {
    String ssid = serverweb.arg("SSID") + "\n" + serverweb.arg("PASSWORD") + "\n";
    ssi = serverweb.arg("SSID");
    pwd = serverweb.arg("PASSWORD");
    File f = SPIFFS.open("/wifi.config", "w");
    if (!f)
    {
      ssid = ("file open failed");
    }
    else
      f.println(ssid);
    f.close ();
  }
  String msg, ip, mask, gate, dns;
  if (serverweb.hasArg("IP") && serverweb.hasArg("MASK") && serverweb.hasArg("GATEWAY") && serverweb.hasArg("DNS") && serverweb.hasArg("OTAB"))
  {
    String net = serverweb.arg("IP") + "\n" + serverweb.arg("MASK") + "\n" + serverweb.arg("GATEWAY") + "\n" + serverweb.arg("DNS") + "\n" + serverweb.arg("OTAB") + "\n";
    File f = SPIFFS.open("/network.config", "w");
    if (!f)
    {
      net = ("file open failed");
    }
    else
      f.println(net);
    f.close ();
    msg = serverweb.arg("IP");
    msg += "\n" + serverweb.arg("MASK");
    msg += "\n" + serverweb.arg("GATEWAY");
    msg += "\n" + serverweb.arg("DNS") ;
    msg += "\n" + serverweb.arg("OTAB") + "\n";
  }
  String content = "<html><style>" + String(BUTT) + String(TEXTT) + "</style>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"" + String(TEXT_COLOR) + "\"><form action='/network' method='POST'><h2>Network Config</h2>";
  content += "<fieldset style=\"width:15% ; border-radius:15px\"> <legend>Login  information:</legend>";
  content += "<table style='width:250px'>";
  content += "<tr><td>SSID</td><td><input type='text' name='SSID' class=\"text_red\" value='" + ssi + "'></td></tr> ";
  content += "<tr><td>Password:</td><td><input type='password' name='PASSWORD' class=\"text_red\" value='" + pwd + "'></td></tr></table></fieldset>";

  content += "<fieldset style=\"width:15%;border-radius:15px\"><legend>Network</legend><table style='width:200px'>";
  content += "<tr><td>IP</td><td><input type='text' name='IP' class=\"text_red\" value='" + WiFi.localIP().toString() + "'></td></td>";
  content += "<td><td>MASK</td><td><input type='test' name='MASK'class=\"text_red\"  value='" + WiFi.subnetMask().toString() + "'></td></tr>";
  content += "<tr><td>Gateway</td><td><input type='text' name='GATEWAY' class=\"text_red\" value='" + WiFi.gatewayIP().toString() + "'></td></td>";
  content += "<td><td>DNS</td><td><input type='test' name='DNS' class=\"text_red\"  value='" + WiFi.dnsIP().toString() + "'></td></tr>";
  content += "<tr><td>OTA on boot</td><td><input type='number' name='OTAB' class=\"text_red\" value='" + String(otab) + "'></td></td></tr></table>";

  content += "<input type='submit' name='SUBMIT'  class=\"button_red\" value='Save'></fieldset></form>" + msg + "<br>";
  content += "<button onclick=\"location.href='/'\"class=\"button_red\" type=\"button\">Back</button> <button onclick=\"location.href='/restart'\"class=\"button_red\"  type=\"button\">Restart ESP32go</button><br>";
  content += "You must restart ESP32go for network changes to take effect ";

  content += "</body></html>";

  serverweb.send(200, "text/html", content);

}


bool handleFileRead(String path)
{

  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = serverweb.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
#ifdef IR_CONTROL
void handleRemote(void)
{
  char n;
  String code, msg;
  if (serverweb.args() == 31)
  {
    File f = SPIFFS.open("/remote.config", "w");

    for (uint8_t i = 0; i < serverweb.args(); i++)
    {
      //content += " " + serverweb.argName(i) + ": " + serverweb.arg(i) + "<br>";
      code = serverweb.arg(i);
      msg += code + "\n";
      cmd_map[i] = code.toInt();
    }
    f.println(msg);
    f.close ();
  }

  //  String content =  "<html><head> <meta http-equiv='refresh' content='3'><style>" + String(BUTT) + String(TEXTT) + "</style>" + String(AUTO_SIZE) + " </head><body  bgcolor=\"#000000\" text=\"#FF6000\"><h2>info</h2><br>";
  String content =  "<html><head> <style>" + String(BUTT) + String(TEXTT1) + "</style>" + String(AUTO_SIZE) + " </head><body  bgcolor=\"#000000\" text=\"#5599ff\"><h2>IR REMOTE</h2><br>";
  // content += "Ir Code : " + String(truecode) + "  "+String(lasti)+"  " +codes[lasti] + "  "+String(obj)+"<br>";
  content += "<form id=\"IR_Form\" action=\"/remote\">";
  content += "IrCodes: <br><table style='width:200px'>";

  for (n = 0; n < 31; n++)
  {
    if (!(n % 2))
      content += "<tr><td>" + codes[n] + "</td><td><input type='text' name='" + codes[n] + "' class=\"text_red\"  value='" + String(cmd_map[n]) + "'></td>";
    else
      content += "<td>" + codes[n] + "</td><td><input type='text' name='" + codes[n] + "' class=\"text_red\"  value='" + String(cmd_map[n]) + "'></td></tr>";

  }
  content += "</td></table> <input type=\"button\"  class=\"button_red\" onclick=\"myFunction()\" value=\"Save Codes\">";
  // content+="<br><button onclick=\"location.href='/open'\" class=\"button_red\" type=\"button\">Open</button><br>";
  content += "<button onclick=\"location.href='/IR'\" class=\"button_red\" type=\"button\">IR</button><br>";
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Home</button><br>";

  content += "<script>function myFunction() {  document.getElementById(\"IR_Form\").submit();}</script>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}

void handleIr(void)
{
  char n;

  String content =  "<html><head> <meta http-equiv='refresh' content='3'><style>" + String(BUTT) + String(TEXTT) + "</style>" + String(AUTO_SIZE) + " </head><body  bgcolor=\"#000000\" text=\"#5599ff\"><h2>info</h2><br>";
  content += "Ir Code : " + String(truecode) + "<br>";
  if (lasti < 31)  content += "Action : " + codes[lasti] + "<br>";
  content += "<button onclick=\"location.href='/remote'\" class=\"button_red\" type=\"button\">Remote</button><br>";
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button><br>";
  content += "</body></html>";

  serverweb.send(200, "text/html", content);
}
#endif
void handleFocus(void)
{
  if (serverweb.hasArg("FOCUS"))
  {
    String net = serverweb.arg("FOCUS");
    focus_motor.position = focus_motor.target = focus_motor.target = net.toInt();
  }
  if (serverweb.hasArg("MOVE"))
  {
    String net = serverweb.arg("MOVE");
    focus_motor.target = net.toInt();
    move_to(&focus_motor, focus_motor.target, focuspeed);

  }
  String content =  "<html><head><style>" + String(BUTT) + String(TEXTT) + "</style>" + String(AUTO_SIZE) + "</head><body  bgcolor=\"#000000\" text=\"#5599ff\"><h2>Focus</h2><br>";
  content += "Estado : " + String( focus_motor.position) + "<br>" + "<form action='/focus' method='POST'>";

  content += "<td><input type='number' step='1' name='FOCUS' class=\"text_red\" value='" + String(focus_motor.target) + "'></td></tr>";
  content += "<input type='submit' name='SUBMIT'  class=\"button_red\" value='Set'></form>"  "<br>";
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Home</button><br>";
  // content += "Timer1 " + String(stepcounter1) + "<br>";
  //content += "Timer2 " + String(stepcounter2) + "<br>";


  content += "</body></html>";
  serverweb.send(200, "text/html", content);
  //  timerAlarmDisable(timer_alt);
  //  timerAlarmEnable(timer_alt);
}

void handleMeridian(void)
{
  if (serverweb.hasArg("SIDE"))
  {
    String net = serverweb.arg("SIDE");
    int   side = net.toInt();
    //if (telescope->mount_mode == EQ)
    meridianflip(telescope, side);
  }

  String content =  "<html><head> <meta http-equiv='refresh' content=\"0;url='./\"><style>" + String(BUTT) + String(TEXTT) + "</style>" + String(AUTO_SIZE) + " </head><body  bgcolor=\"#000000\" text=\"#FF6000\"><h2>ESP-PGT++ Meridian flip</h2><br>";
  content += "Pier side: " + String(get_pierside(telescope)  ? "WEST" : "EAST") + "<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}

void handleMonitor(void)
{ char page[900];
  char buffra[12];
  char buffdec[12];
  if (telescope->mount_mode) lxprintra(buffra, st_current.ra);
  else mount_lxra_str(buffra, telescope);
  if (telescope->mount_mode) lxprintde(buffdec, st_current.dec);
  else mount_lxde_str(buffdec, telescope);
  buffdec[3] = ':';
  snprintf(page, 900,
           "<html>\
<head> <meta http-equiv='refresh' content='3'><style>%s</style> %s </head>\
<body  bgcolor=\"#000000\" text=\"#5599ff\"><h2>Monitor</h2> \
<br>AZ Counter: %ld <br>Alt Counter: %ld \
<br>AZ Back Counter: %d<br>Alt Back Counter: %d \
<br>Clients: %d<br>Focus Counter: %d \
<br>Is slewing: %d <br>Is tracking: %d \
<br>RA: %s<br>De: %s  \
<br>%d<br><button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button><br>\
</body></html>",
           BUTTTEXTT, AUTO_SIZE, telescope->azmotor->counter, telescope->altmotor->counter, azbackcounter,
           altbackcounter, clients_connected, focus_motor.position,
           (telescope->azmotor->slewing || telescope->altmotor->slewing) ? 1 : 0,
           telescope->is_tracking , &buffra, &buffdec,serverClients[0].remoteIP()[3]);//



  serverweb.send(200, "text/html", page);
}


void handleMain(void)
{ time_t now;
  now = time(nullptr);
  String checked = (get_pierside(telescope) ? "West" : "East");
  String content = "<html><style>" + String(BUTTTEXTT) + "</style>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"" + String(TEXT_COLOR) + "\"><h2>ESP32go";
#ifdef IR_CONTROL
  content += " IR Control</h2>";
#else
  content += " NUNCHUK</h2>";
#endif
  String mount_mode;
  switch (telescope->mount_mode) {
    case EQ : mount_mode = "EQUATORIAL"; break;
    case ALTAZ: mount_mode = "ALT-A.Z"; break;
    case ALIGN: mount_mode = "EQ. 2-Stars Aligned"; break;
  }
  content +=  "Mount mode:" + mount_mode + "<br>";
  content += "<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Config</legend>";
  content += "<table style='width:250px'>";
  content += "<button onclick=\"location.href='/config'\" class=\"button_red\"   type=\"button\">Mount</button>&ensp; ";
  content += "<button onclick=\"location.href='/network'\" class=\"button_red\"   type=\"button\">WLAN&Network</button><br>";
  content += "<button onclick=\"location.href='/update'\" class=\"button_red\" type=\"button\">Firmware</button>&ensp;";
  content += "<button onclick=\"location.href='/restart'\"class=\"button_red\"  type=\"button\">ReStart ESP32go</button></table></fieldset>";
  content += "<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Commands</legend>" ;
  content += "<table style='width:250px'>";
  content += "<button onclick=\"location.href='/meridian?SIDE=0'\" class=\"button_red\"  type=\"button\">Set EAST</button>&ensp;";
  content += "<button onclick=\"location.href='/meridian?SIDE=1'\" class=\"button_red\"  type=\"button\">Set WEST</button>&ensp;" + checked + "<br>";
  content += "<button onclick=\"location.href='/park'\" class=\"button_red\" type=\"button\">Park</button>&ensp;";
  content += "<button onclick=\"location.href='/home?HOME=0'\" class=\"button_red\" type=\"button\">Reset  home</button>&ensp;";
  content += "<button onclick=\"location.href='/home?HOME=1'\" class=\"button_red\" type=\"button\">GO&Park home</button><br>";
  content += "<button onclick=\"location.href='/starinstructions'\"class=\"button_red\" type=\"button\">2 stars align</button></table></fieldset>";
  content += "<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Control set</legend>";
  content += "<table style='width:250px'>";
#ifdef IR_CONTROL
  content += "<button onclick=\"location.href='/remote'\" class=\"button_red\" type=\"button\">IR Remote </button><br>";
#endif
#ifdef NUNCHUCK_CONTROL
  content += "<button onclick=\"location.href='/nunchuk?ENABLE=1'\" class=\"button_red\" type=\"button\">Init Nunchuk </button>";
  content += "<button onclick=\"location.href='/nunchuk'\" class=\"button_red\" type=\"button\">Disable Nunchuk </button>";
#endif
  content += "</table></fieldset> <fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Info</legend>";
  content += "<table style='width:250px'>";
  content += "<button onclick=\"location.href='/time'\" class=\"button_red\" type=\"button\">Sync Date/Time</button><button onclick=\"location.href='/monitor'\" class=\"button_red\" type=\"button\">Monitor Counters</button></table></fieldset>";

  content += "<br>Loaded at Time :" + String(ctime(&now)) + "<br></body></html>";
  serverweb.send(200, "text/html", content);
}

void handleInstructions( void)
{
  String content =   "<html><style>" + String(BUTT) + String(TEXTT) + "</style>" + String(AUTO_SIZE) + "<body  bgcolor=\"#000000\" text=\"" + String(TEXT_COLOR) + "\"><br>";
  content += "<h2>Instructions:</h2>""";
  content += "1-Sync. Date/time and go back to 2 stars aling""<br>";
  content += "2-Click `Star1 Select`""<br>";
  content += "3-Toggle to Skysafari, choose  a star, center and align it""<br>";
  content += "4-Toggle to webserver and Click `Star2 Select`""<br>";
  content += "5-Toggle to Skysafary again, choose different star, center and align it""<br>";
  content += "6-Click `Sync. mode` ""<br>";
  content += "->>Try not to choose very high or very low stars, with some azimuth and altitude gap between them.""<br>";
  content += "->>If you do mistake, go back to Sync date/time and start from new ""<br><br>";
  content += "->>WATCHOUT: Performing date/time Sync will delete align data!.""<br><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}

void handleStarInstructions( void)
{
  String content = "<html><frameset cols=\"50%,50%\" frameborder=\"0\" border=\"0\">";
  content += "<frame name=\"main\" src=\"/Align\">";
  content += "<frame name=\"_instructions\" src=\"\" style=\"background-color:#000000;\">";
  content += "</frameset>";
  content += "<noframes>";
  content += "<script>location=\"/Align\";</script>";
  content += "</noframes></html>";
  serverweb.send(200, "text/html", content);
}

void initwebserver(void)
{


  serverweb.on("/park", handlePark);
  serverweb.on("/time", handleTime);
  serverweb.on("/sync", handleSync);
  serverweb.on("/restart", handleRestart);
  serverweb.on("/Align", handleStar);
  serverweb.on("/home", handleHome);
  serverweb.on("/gohome", handleHome);
  serverweb.on("/network", handleNetwork);
  serverweb.on("/meridian", handleMeridian);
  serverweb.on("/focus", handleFocus);
  serverweb.on("/config", handleConfig);
  serverweb.on("/", handleMain);
  serverweb.on("/main", handleMain );
#ifdef IR_CONTROL
  serverweb.on("/remote", handleRemote);
  serverweb.on("/IR", handleIr);
#endif
#ifdef NUNCHUCK_CONTROL
  serverweb.on("/nunchuk", handleNunchuk);
#endif
  serverweb.on("/monitor", handleMonitor);
  serverweb.on("/starinstructions", handleStarInstructions);
  serverweb.on("/instructions", handleInstructions);
  serverweb.onNotFound([]()
  {
    if (!handleFileRead(serverweb.uri()))
      serverweb.send(404, "text/plain", "FileNotFound");
  });

  serverweb.begin();
}
