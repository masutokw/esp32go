#include "conf.h"
#ifdef WEB_INTERFACE
#include "time.h"
#include "taki.h"
#include "webserver.h"
#include "nunchuck.h"
#include "tb6612.h"
#include "encoder.h"
#include "tmc.h"
#include "focus.h"
#include "wheel.h"
#ifdef IR_CONTROL
extern uint32_t truecode, lasti;

extern byte cmd_map[];
const String codes[31] = { "EAST", "WEST", "NORTH", "SOUTH", "OK", "FOCUS_F", "FOCUS_B", "GUIDE", "CENTER", "FIND", "SLEW", "N_EAST", "N_WEST", "S_EAST",
                           "S_WEST", "TRACK", "UNTRACK", "B_1", "B_2", "B_3", "B_4", "B_5", "B_6", "B_7", "B_8", "B_9", "B_0", "GO_TO", "CLEAR", "FLIP_W", "FLIP_E" };
#endif
#ifdef PAD
#include "pad.h"
extern bool pad_enabled;

#endif
#ifdef RTC_IC
#include "RTClib.h"
extern RTC_IC rtc;
#endif
extern wheel_t wheel[9];
extern byte wheel_index;
extern byte wheel_slots;
extern tmcmotor_t tmcmotors[4];
extern int clients_connected;
extern uint64_t period_alt;
extern char sync_target;
extern int dcfocus;
extern int align_star_index, encb;
extern c_star st_1, st_2;
extern time_t init_time;
extern stepper focus_motor, aux_motor;
extern int stepcounter1, stepcounter2;
extern hw_timer_t* timer_az;
extern hw_timer_t* timer_alt;
//extern int focusvolt;
extern int azbackcounter, altbackcounter;
extern int wifi_pad_IP3;
extern int wifi_pad_IP2;
char buffer[30];
extern c_star st_current;
//extern bool n_disable;
extern WiFiClient serverClients[MAX_SRV_CLIENTS];
extern bool NTP_Sync;
extern char tzstr[50];
//char temp [4800];
String getContentType(String filename) {
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
void handleConfig(void) {
  char temp[4500];  //maximo que se puede enviar
  char msg[43];
  // msg.reserve(500);
  time_t now;
  now = time(nullptr);
  if (serverweb.hasArg("MAXCOUNTER") && serverweb.hasArg("MAXCOUNTER_ALT")) {
    snprintf(temp, 700, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n#\n",
             serverweb.arg("MAXCOUNTER"), serverweb.arg("MAXCOUNTER_ALT"),
             serverweb.arg("GUIDE"), serverweb.arg("CENTER"), serverweb.arg("FIND"), serverweb.arg("SLEW"),
             serverweb.arg("GUIDEA"), serverweb.arg("CENTERA"), serverweb.arg("FINDA"), serverweb.arg("SLEWA"),
             serverweb.arg("PRESCALER"),
             serverweb.arg("LONGITUDE"), serverweb.arg("LATITUDE"), serverweb.arg("TIMEZONE"),
             serverweb.arg("RAMP"), serverweb.arg("RAMPA"), serverweb.arg("BACK_AZ"), serverweb.arg("BACK_ALT"),
             serverweb.arg("MOUNT"), serverweb.arg("TRACK"), serverweb.arg("AUTOFLIP"), String(serverweb.hasArg("INVAZ")),
             String(serverweb.hasArg("INVALT")), String(serverweb.hasArg("ACTAZ")), String(serverweb.hasArg("ACTALT")));
    File f = SPIFFS.open(MOUNT_FILE, "w");
    if (!f) {

      snprintf(msg, 42, "File open failed");
    } else {
      f.println(temp);
      f.close();
      readconfig(telescope);
      now = time(nullptr);
      snprintf(msg, 42, "Config Saved at %s ", ctime(&now));
    }
  }
  //First  page chunk
  snprintf(temp, 4500,

           "<html><style>" BUTTTEXTT TEXTT2 "</style>" AUTO_SIZE
           "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><form action='/config' method='POST'><h2>ESP32go NUNCHUK</h2>\
<fieldset style=\"width:15%; border-radius:15px\"> <legend>Mount parameters:</legend><table style='width:250px'>\
<tr><th><button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Main</button></th><th>Azimuth</th><th>Altitude</th></tr>\
<tr><td>Msteps</td><td> <input type='number' name='MAXCOUNTER' class=\"text_red\" value='%d'></td><td> <input type='number' name='MAXCOUNTER_ALT'  class=\"text_red\" value='%d'></td></tr>\
</table><br>\
<table style='width:250px'><tr><th>Speeds</th><th>RA/AZ</th><th>Dec/Alt</th></tr>\
<tr><td>Guide</td><td><input type='number' step='0.01' name='GUIDE' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='GUIDEA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Center</td><td><input type='number' step='0.01' name='CENTER' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='CENTERA'  class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Find</td><td><input type='number' step='0.01' name='FIND' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='FINDA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Slew</td><td><input type='number' step='0.01' name='SLEW' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='SLEWA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Ramp</td><td><input type='number' step='0.01' name='RAMP' class=\"text_red\" value='%.2f'></td><td><input type='number' step='0.01' name='RAMPA' class=\"text_red\" value='%.2f'></td></tr>\
<tr><td>Backlash</td><td><input type='number' step='1' name='BACK_AZ' class=\"text_red\" value='%d'></td><td><input type='number' step='1' name='BACK_ALT' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Prescaler</td><td><input type='number' step='0.0001' name='PRESCALER' class=\"text_red\" value='%.5f' uSec</td></tr>\
<tr><td>EQ Track</td><td><input type='number' min='0' max='4' title='0.No track 1-Sideral 2-Solar 3-Lunar 4-King.' step='1' name='TRACK'  class=\"text_red\" value ='%d' </td></tr>\
<tr><td>EQ<input type='radio' name='MOUNT' value='0'   %s ></td><td>ALT-AZ<input type='radio' name='MOUNT' value='1' %s ></td><td>EQ2-stars<input type='radio' name='MOUNT' value='2' %s ></td></tr>\
<tr><td>FLIP<input type='checkbox' name='AUTOFLIP' value='1'  %s ></td><td>Invert Az<input type='checkbox' name='INVAZ' value='1' %s ></td><td>Invert Alt<input type='checkbox' name='INVALT' value='1' %s ></td></tr>\
<tr><td colspan=2>Active backlash AZ<input type='checkbox' name='ACTAZ' value='1' %s></td><td>ALT<input type='checkbox' name='ACTALT' value='1' %s  ></td></tr>\
</table><input type='submit' name='SUBMIT' class=\"button_red\" value='Save'></fieldset>\
",
           telescope->azmotor->maxcounter, telescope->altmotor->maxcounter,
           telescope->rate[0][0], telescope->rate[0][1], telescope->rate[1][0], telescope->rate[1][1],
           telescope->rate[2][0], telescope->rate[2][1], telescope->rate[3][0], telescope->rate[3][1],
           telescope->azmotor->acceleration / SEC_TO_RAD, telescope->altmotor->acceleration / SEC_TO_RAD,
           telescope->azmotor->backlash, telescope->altmotor->backlash, telescope->prescaler, telescope->track,
           telescope->mount_mode == EQ ? "checked" : "", telescope->mount_mode == ALTAZ ? "checked" : "", telescope->mount_mode == ALIGN ? "checked" : "",
           telescope->autoflip ? "checked" : "", telescope->azmotor->cw ? "checked" : "", telescope->altmotor->cw ? "checked" : "", telescope->azmotor->active ? "checked" : "", telescope->altmotor->active ? "checked" : "");
  serverweb.setContentLength(CONTENT_LENGTH_UNKNOWN);
  serverweb.send(200, "text/html", temp);
  delay(50);
  //Second  page chunk
  snprintf(temp, 4500,
           "<fieldset style=\"width:15% ; border-radius:15px\"> <legend>Geodata</legend>\
<table style='width:250px'><tr><td>Longitude:</td><td><input type='number' step='any' name='LONGITUDE' class=\"text_red\" value='%.4f'></td></tr>\
<tr><td>Latitude:</td><td><input type='number'step='any'  name='LATITUDE' class=\"text_red\"  value='%.4f'></td></tr>\
<tr><td>GMT offset:</td><td><input type='number'step='1' name='TIMEZONE' class=\"text_red\" value='%d'></td></tr></table>\
</fieldset></form><br>\
<br>Load Time : %s \
<br>%s \
</body></html>",
           telescope->longitude, telescope->lat, telescope->time_zone, ctime(&now), &msg);
  serverweb.sendContent(temp);
  serverweb.sendContent("");
}

void handlePark(void) {
  time_t now;
  now = time(nullptr);
  mount_park(telescope);
  String content = "<html>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"#FFFFFF\"><h2>ESP32go PARKED</h2><br>";
  content += "Mount parked  ,position saved on EEPROM.<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "Alt res:" + String(RAD_TO_ARCS * telescope->altmotor->resolution) + "<br>";
  content += "Az res:" + String(RAD_TO_ARCS * telescope->azmotor->resolution) + "<br>";
  content += "Sideral:" + String(sidereal_timeGMT(telescope->longitude, telescope->time_zone)) + "<br>";
  content += "Time :" + String(ctime(&now)) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}
void handleHome(void) {
  time_t now;
  now = time(nullptr);
  if (serverweb.hasArg("HOME")) {
    String net = serverweb.arg("HOME");
    char cmd = net.toInt();
    switch (cmd) {
      case 0:
        mount_home_set(telescope);
        break;
      case 1:
        buzzerOn(300);
        mount_goto_home(telescope);
        break;
      case 2:
        if (serverClients[0]) serverClients[0].stop();
        break;
#ifdef PAD
      case 3:
        if (pad_enabled) pad_Detach();
        else pad_Init();
        break;
#endif
    }
  }
  String content = "<html>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>ESP32go++ PARKED</h2><br>";
  content += "Mount parked  ,position saved on EEPROM.<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "Alt res:" + String(RAD_TO_ARCS * telescope->altmotor->resolution) + "<br>";
  content += "Az res:" + String(RAD_TO_ARCS * telescope->azmotor->resolution) + "<br>";
  content += "Sideral:" + String(sidereal_timeGMT(telescope->longitude, telescope->time_zone)) + "<br>";
  content += "Time :" + String(ctime(&now)) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}
void handleSync(void) {
  String msg, msg1;
  time_t rtc_l;
  if (serverweb.hasArg("GMT")) {
    msg1 = serverweb.arg("OFFSET");
    msg = serverweb.arg("GMT");
    rtc_l = msg.toInt();
    //msg = serverweb.arg("OFFSET");
    timeval tv = { rtc_l, 0 };
    //timezone tz = {msg.toInt()  ,0 };
    settimeofday(&tv, nullptr);
    rtc_l = time(nullptr);
#ifdef RTC_IC
    rtc.adjust(DateTime(time(nullptr)));
#endif
    if (telescope->mount_mode == EQ) {
      sdt_init(telescope->longitude, telescope->time_zone);
    } else {
      telescope->is_tracking = FALSE;
      sync_target = TRUE;
      tak_init(telescope);
      //telescope->is_tracking = TRUE;
      telescope->azmotor->targetspeed = 0.0;
      telescope->altmotor->targetspeed = 0.0;
    }
  }
  String content = "<!DOCTYPE html><html>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR " \"><h2>ESP32Go Sync </h2><br>";
  content += "<p id=\"fecha\">" + msg + " " + String(ctime(&rtc_l)) + "</p>";
  content += "<p id=\"fecha\">" + String(rtc_l) + "</p>";

  content += "<button onclick=\"location.href='/Align'\"class=\"button_red\" type=\"button\">Continue 2-star alignment</button><br><br>";

  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}
void handleTime(void) {
  time_t now;
  now = time(nullptr);
  String content = "<!DOCTYPE html><html>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>ESP32Go++ Time </h2><br>";
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
void handleRestart(void) {
  mount_park(telescope);
  String content = "<html>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>ESP-PGT++ restarted</h2><br>";
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
void handleNunchuk(void) {
  String action = "disabled";
  if (serverweb.hasArg("ENABLE")) {
    nunchuck_init(SDA_PIN, SCL_PIN);
    nunchuck_disable(FALSE);
    action = "restarted";
  } else
    nunchuck_disable(TRUE);

  String content = "<html>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>ESP32go Nunchuck " + action + "</h2><br>";

  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Home</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}
#endif

void handleStar(void) {
  String msg, txt;
  if (serverweb.hasArg("Mode")) {
    msg = serverweb.arg("Mode");
    telescope->smode = msg.toInt();
  }
  switch (telescope->smode) {
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

  String content = "<html><style>" BUTTTEXTT "</style>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>Sync mode</h2><br>";
  content += "Last selected star " + String(align_star_index) + "<br>";
  content += "Sync mode set to:" + txt + "<br><br>";

  content += "<a href=\"/instructions\" target=\"_instructions\"><button class=\"button_red\" type=\"button\">Instructions</button></a><br><br>";

  content += "<fieldset style=\"width:15% ; border-radius:15px\"> <legend>Data</legend>";
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
void handleNetwork(void) {
  if (serverweb.hasArg("SSID") && serverweb.hasArg("PASSWORD")) {
    String ssid = serverweb.arg("SSID") + "\n" + serverweb.arg("PASSWORD") + "\n";
    ssi = serverweb.arg("SSID");
    pwd = serverweb.arg("PASSWORD");
    File f = SPIFFS.open(WIFI_FILE, "w");
    if (!f) {
      ssid = ("file open failed");
    } else
      f.println(ssid);
    f.close();
  }
  String msg, ip, mask, gate, dns;
  if (serverweb.hasArg("IP") && serverweb.hasArg("MASK") && serverweb.hasArg("GATEWAY") && serverweb.hasArg("DNS") && serverweb.hasArg("OTAB")) {
    String net = serverweb.arg("IP") + "\n" + serverweb.arg("MASK") + "\n" + serverweb.arg("GATEWAY") + "\n" + serverweb.arg("DNS") + "\n" + serverweb.arg("OTAB") + "\n" + (serverweb.arg("BT")!="" ? serverweb.arg("BT"):"0") + "\n" + (serverweb.arg("AP")!="" ? serverweb.arg("AP"):"0") + "\n" + serverweb.arg("AP_SSID") + "\n" ;
    File f = SPIFFS.open(NETWORK_FILE, "w");
    if (!f) {
      net = ("file open failed");
    } else
      f.println(net);
    f.close();
    msg = serverweb.arg("IP");
    msg += "\n" + serverweb.arg("MASK");
    msg += "\n" + serverweb.arg("GATEWAY");
    msg += "\n" + serverweb.arg("DNS");
    msg += "\n" + serverweb.arg("OTAB");
    msg += "\n" + serverweb.arg("BT");
    msg += "\n" + serverweb.arg("AP");
    msg += "\n" + serverweb.arg("AP_SSID") + "\n";
  }
  msg += "\n current local IP:"+WiFi.localIP().toString()+"\n";

  IPAddress localip=WiFi.localIP();
  IPAddress gateway=WiFi.gatewayIP();
  IPAddress subnet=WiFi.subnetMask();
  IPAddress dnsserver=WiFi.dnsIP();

  if (SPIFFS.exists(NETWORK_FILE)) 
  {
      File f;
      f = SPIFFS.open(NETWORK_FILE, "r");
      localip.fromString(f.readStringUntil('\n'));
      subnet.fromString(f.readStringUntil('\n'));
      gateway.fromString(f.readStringUntil('\n'));
      dnsserver.fromString(f.readStringUntil('\n'));
      f.close();
  }

  String content = "<html><style>" BUTTTEXTT "</style>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><form action='/network' method='POST'><h2>Network Config</h2>";
  content += "<fieldset style=\"width:15% ; border-radius:15px\"> <legend>Login  information:</legend>";
  content += "<table style='width:250px'>";
  content += "<tr><td>SSID</td><td><input type='text' name='SSID' class=\"text_red\" value='" + ssi + "'></td></tr> ";
  content += "<tr><td>Password:</td><td><input type='password' name='PASSWORD' class=\"text_red\" value='" + pwd + "'></td></tr></table></fieldset>";

  content += "<fieldset style=\"width:15%;border-radius:15px\"><legend>Network</legend><table style='width:200px'>";
  content += "<tr><td>IP</td><td><input type='text' name='IP' class=\"text_red\" value='" + localip.toString() + "'></td></td>";
  content += "<td><td>MASK</td><td><input type='test' name='MASK'class=\"text_red\"  value='" + subnet.toString() + "'></td></tr>";
  content += "<tr><td>Gateway</td><td><input type='text' name='GATEWAY' class=\"text_red\" value='" + gateway.toString() + "'></td></td>";
  content += "<td><td>DNS</td><td><input type='test' name='DNS' class=\"text_red\"  value='" + dnsserver.toString() + "'></td></tr>";
  content += "<tr><td colspan='2'>OTA on boot</td><td colspan='3'><input type='number' name='OTAB' class=\"text_red\" value='" + String(otab) + "'></td></tr>";
  content += "<tr><td colspan='2'>Bluetooth on boot</td><td colspan='3'><input type='checkbox' name='BT' class=\"text_red\" value='1' " + String(bt_on ? "checked" : "") + "></td></tr>";
  content += "<tr><td colspan='2'>SoftAP on boot</td><td colspan='3'><input type='checkbox' name='AP' class=\"text_red\" value='1' " + String(ap_on ? "checked" : "") + "></td></tr>";
  content += "<tr><td colspan='2'>SoftAP SSID</td><td colspan='3'><input type='text' name='AP_SSID' class=\"text_red\" value='" + ap_ssid + "'></td></tr>"; 
  content += "</table>";

  content += "<input type='submit' name='SUBMIT'  class=\"button_red\" value='Save'></fieldset></form>" + msg + "<br>";
  content += "<button onclick=\"location.href='/'\"class=\"button_red\" type=\"button\">Back</button> <button onclick=\"location.href='/restart'\"class=\"button_red\"  type=\"button\">Restart ESP32go</button><br>";
  content += "You must restart ESP32go for network changes to take effect ";

  content += "</body></html>";

  serverweb.send(200, "text/html", content);
}


bool handleFileRead(String path) {

  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
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
void handleRemote(void) {
  char n;
  String code, msg;
  if (serverweb.args() == 31) {
    File f = SPIFFS.open(IR_FILE, "w");

    for (uint8_t i = 0; i < serverweb.args(); i++) {
      //content += " " + serverweb.argName(i) + ": " + serverweb.arg(i) + "<br>";
      code = serverweb.arg(i);
      msg += code + "\n";
      cmd_map[i] = code.toInt();
    }
    f.println(msg);
    f.close();
  }


  String content = "<html><head> <style>" BUTT TEXTT1 "</style>" AUTO_SIZE " </head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>IR REMOTE</h2><br>";

  content += "<form id=\"IR_Form\" action=\"/remote\">";
  content += "IrCodes: <br><table style='width:200px'>";

  for (n = 0; n < 31; n++) {
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

void handleIr(void) {
  char n;

  String content = "<html><head> <meta http-equiv='refresh' content='3'><style>" BUTTTEXTT "</style>" AUTO_SIZE " </head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>info</h2><br>";
  content += "Ir Code : " + String(truecode) + "<br>";
  if (lasti < 31) content += "Action : " + codes[lasti] + "<br>";
  content += "<button onclick=\"location.href='/remote'\" class=\"button_red\" type=\"button\">Remote</button><br>";
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button><br>";
  content += "</body></html>";

  serverweb.send(200, "text/html", content);
}
#endif

void handleFocusPos(void) {
  String net="";
  String msg;
  if (serverweb.hasArg("MOVE")) {
    net = serverweb.arg("MOVE");
    focus_motor.target = net.toInt();
    move_to(&focus_motor, focus_motor.target, focus_motor.speed);
    msg = "Focus moved to position " + net;
  }
  int memfocus[6];
  if (SPIFFS.exists(FOCUSPOS_FILE)) 
  {
    File f;
    f = SPIFFS.open(FOCUSPOS_FILE, "r");
    for(int i=0;i<=5; i++)
      memfocus[i]=f.readStringUntil('\n').toInt();
    f.close();
  }

  String content = "<html><head><style>" BUTT TEXTT "</style>" AUTO_SIZE "</head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>Focus position</h2><br>";
  content += "<form action='/focuspos' method='POST'>";
  content += "<select name='MOVE' id='MOVE'>";
  for (int g = 0; g <= 5; g++) {
    content += "<option value='" + String(memfocus[g]) + "' " + (net == String(memfocus[g]) ? "selected" : "") + ">M" + String(g+1) + "</option>";
  }
  content += "</select><input type='submit' name='SUBMIT'  class=\"button_red\" value='Move focus'>";
  content += "</form><br>" + msg;
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button><br>";
  content += "</body></html>";

  serverweb.send(200, "text/html", content);

}


void handleFocus(void) {
  int memfocus[6];
  if (serverweb.hasArg("FOCUS")) {
    int action=0;
    String net = serverweb.arg("FOCUS");
    if(serverweb.hasArg("ACTION"))
      action=serverweb.arg("ACTION").toInt();
    if(action > 0)
    {
      if (SPIFFS.exists(FOCUSPOS_FILE)) 
      {
        File f;
        f = SPIFFS.open(FOCUSPOS_FILE, "r");
        for(int i=0;i<=5; i++)
          memfocus[i]=f.readStringUntil('\n').toInt();
        f.close();
      }
      String msg;
      for(int i=0;i<=5; i++)
        msg += ((action-1)==i ? String(focus_motor.position):String(memfocus[i]))+"\n";
      File f = SPIFFS.open(FOCUSPOS_FILE, "w");
      f.println(msg);
      f.close();
    }
    else
    {
      focus_motor.position = focus_motor.target = focus_motor.target = net.toInt();
    }
  }
  if (serverweb.hasArg("MOVE")) {
    String net = serverweb.arg("MOVE");
    focus_motor.target = net.toInt();
    move_to(&focus_motor, focus_motor.target, focus_motor.speed);
  }

  if (SPIFFS.exists(FOCUSPOS_FILE)) 
  {
    File f;
    f = SPIFFS.open(FOCUSPOS_FILE, "r");
    for(int i=0;i <=5; i++)
      memfocus[i]=f.readStringUntil('\n').toInt();
    f.close();
  }

  String content = "<html><head><style>" BUTT TEXTT "</style>" AUTO_SIZE "</head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\">";
  content += "<fieldset style=\"width:15% ; border-radius:15px\"> <legend>Focus</legend>";
  content += "Status: " + String(focus_motor.position) + "<br>" + "<form action='/focus' method='POST'>";

  content += "<td><input type='number' step='1' name='FOCUS' class=\"text_red\" value='" + String(focus_motor.target) + "'></td></tr>";
  content += "<input type='hidden' name='ACTION' value='0'>";
  content += "<input type='submit' name='SUBMIT'  class=\"button_red\" value='Set as current position'><br>";
  content += "</fieldset><br>";
  content += "<fieldset style=\"width:15% ; border-radius:15px\"> <legend>Set current position as</legend>";
  content += "<input type='submit' name='SUBMIT1'  class=\"button_red\" value='M1' onclick='document.forms[0].elements[\"ACTION\"].value=1;'>&nbsp;&nbsp;Current:&nbsp;"+String(memfocus[0])+"<br>";
  content += "<input type='submit' name='SUBMIT2'  class=\"button_red\" value='M2' onclick='document.forms[0].elements[\"ACTION\"].value=2;'>&nbsp;&nbsp;Current:&nbsp;"+String(memfocus[1])+"<br>";
  content += "<input type='submit' name='SUBMIT3'  class=\"button_red\" value='M3' onclick='document.forms[0].elements[\"ACTION\"].value=3;'>&nbsp;&nbsp;Current:&nbsp;"+String(memfocus[2])+"<br>";
  content += "<input type='submit' name='SUBMIT4'  class=\"button_red\" value='M4' onclick='document.forms[0].elements[\"ACTION\"].value=4;'>&nbsp;&nbsp;Current:&nbsp;"+String(memfocus[3])+"<br>";
  content += "<input type='submit' name='SUBMIT5'  class=\"button_red\" value='M5' onclick='document.forms[0].elements[\"ACTION\"].value=5;'>&nbsp;&nbsp;Current:&nbsp;"+String(memfocus[4])+"<br>";
  content += "<input type='submit' name='SUBMIT6'  class=\"button_red\" value='M6' onclick='document.forms[0].elements[\"ACTION\"].value=6;'>&nbsp;&nbsp;Current:&nbsp;"+String(memfocus[5])+"<br>";
  content += "</form></fieldset><br>";
  content += "<button onclick=\"location.href='/aux'\" class=\"button_red\" type=\"button\">Back</button><br><br>";
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Home</button><br>";
  // content += "Timer1 " + String(stepcounter1) + "<br>";
  //content += "Timer2 " + String(stepcounter2) + "<br>";


  content += "</body></html>";
  serverweb.send(200, "text/html", content);
  //  timerAlarmDisable(timer_alt);
  //  timerAlarmEnable(timer_alt);
}

void handleMeridian(void) {
  if (serverweb.hasArg("SIDE")) {
    String net = serverweb.arg("SIDE");
    int side = net.toInt();
    //if (telescope->mount_mode == EQ)
    meridianflip(telescope, side);
  }

  String content = "<html><head> <meta http-equiv='refresh' content=\"0;url='./\"><style>" BUTTTEXTT "</style>" AUTO_SIZE " </head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>ESP-PGT++ Meridian flip</h2><br>";
  content += "Pier side: " + String(get_pierside(telescope) ? "WEST" : "EAST") + "<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}

void handleMonitor(void) {
  char page[1500];
  char buffra[12];
  char buffdec[12];
  char times[300];
  time_t now = time(nullptr);
  time_t t = time(NULL);
  int fg, azcount, altcount;
  int zcount[2] = { 0, 0 };
  //---
  char wifisignal[80];
  wifiStrength(wifisignal);
  //---

  fg = getoffset();
  sprintf(times, "UTC:%s  %d", asctime(gmtime(&t)), fg);


  int enc = 0;
#ifdef ENCODER
  if (encb) enc = read_raw_encoder();
#endif
#if defined(RTC_IC) && defined(RTC_NVRAM) && RTC_NVRAM > 0
#if RTC_IC == RTC_DS3231
  // uint8_t RTC_ADDRESS = 0x68;
  int i = 0;
  uint8_t* bufy = (uint8_t*)&zcount;
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(RTC_NVADDR);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)RTC_ADDRESS, (uint8_t)7);
  while (Wire.available()) bufy[i++] = Wire.read();
#else
  rtc.readnvram((uint8_t*)&zcount[0], 4, RTC_NVADDR);
  rtc.readnvram((uint8_t*)&zcount[1], 4, RTC_NVADDR + 4);
#endif
#endif


  // Serial.println(RTC_now.unixtime());
  //timeval tv = { RTC_now.unixtime(), 0 };
  if (telescope->mount_mode) lxprintra(buffra, st_current.ra);
  else mount_lxra_str(buffra, telescope);
  if (telescope->mount_mode) lxprintde(buffdec, st_current.dec);
  else mount_lxde_str(buffdec, telescope);
  char azangle[15],altangle[15];
  lxprintaz1(azangle,telescope->azmotor->position);
  lxprintaz1(altangle,telescope->altmotor->position);
  buffdec[3] = ':';
  snprintf(page, 1500,
           "<html>\
<head> <meta http-equiv='refresh' content='3'><style>" BUTTTEXTT "</style>" AUTO_SIZE " </head>\
<body  bgcolor=\"#000000\" text=\"#5599ff\"><h2>Monitor</h2> \
<br>AZ Counter: %ld <br>Alt Counter: %ld \
<br>AZ Back Counter: %d<br>Alt Back Counter: %d \
<br>Clients: %d<br>Focus Counter: %d \
<br>AUX Counter: %d \
<br>Is slewing: AZ:%d ALT:%d <br>Is tracking: %d (track %d)\
<br>Target speed: AZ:%s ALT:%s \
<br>Parked: %d \
<br>RA: %s<br>De: %s  \
<br>AZ angle: %s<br>Alt angle: %s \
<br>PEC:%d  %d<br>\
<br>WifiPAD IP : X.X%d.%d<br><button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button><br>\
 Date %s <br> %s \
<br>Wifi Signal Strength: %s <br>\
<br> NVRAM %d %d\
</body></html>",
           telescope->azmotor->counter, telescope->altmotor->counter, azbackcounter,
           altbackcounter, clients_connected, focus_motor.position, aux_motor.position,
           telescope->azmotor->slewing, telescope->altmotor->slewing, telescope->is_tracking, telescope->track,
           String(telescope->azmotor->targetspeed, 15).c_str(), String(telescope->altmotor->targetspeed, 15).c_str(),
           telescope->parked, &buffra, &buffdec, &azangle, &altangle, encb, enc, wifi_pad_IP2, wifi_pad_IP3, ctime(&now), times, wifisignal, zcount[0], zcount[1]);
  serverweb.send(200, "text/html", page);
}

void handleMain(void) {
  time_t now;
  now = time(nullptr);
  String checked = (get_pierside(telescope) ? "West" : "East");
  String mount_mode;
  switch (telescope->mount_mode) {
    case EQ: mount_mode = "EQUATORIAL"; break;
    case ALTAZ: mount_mode = "ALT-A.Z"; break;
    case ALIGN: mount_mode = "EQ. 2-Stars Aligned"; break;
  }
  String content = "<html><style>" BUTTTEXTT "</style>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>ESP32go";
#ifdef IR_CONTROL
  content += " IR Control</h2>";
#else
  content += " NUNCHUK</h2>";
#endif

  content += "Mount mode:" + mount_mode + "<br>";
  content += "<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Config</legend>";
  content += "<table style='width:250px'>";
  content += "<button onclick=\"location.href='/config'\" class=\"button_red\"   type=\"button\">Mount</button>&ensp; ";
  content += "<button onclick=\"location.href='/network'\" class=\"button_red\"   type=\"button\">WLAN&Network</button><br>";
  content += "<button onclick=\"location.href='/aux'\" class=\"button_red\" type=\"button\">Focus&Aux</button>&ensp;";
  content += "<button onclick=\"location.href='/wheelconfig'\" class=\"button_red\" type=\"button\">Filter Wheel Slots</button>&ensp;";
  content += "<button onclick=\"location.href='/update'\" class=\"button_red\" type=\"button\">Firmware</button>&ensp;";
  content += "<button onclick=\"location.href='/homecfg'\" class=\"button_red\" type=\"button\">Home Set</button>&ensp;";
#ifdef TMC_DRIVERS
  content += "<button onclick=\"location.href='/tmc'\" class=\"button_red\" type=\"button\">TMC</button>&ensp;";
#endif
  content += "<button onclick=\"location.href='/restart'\"class=\"button_red\"  type=\"button\">ReStart ESP32go</button></table></fieldset>";
  content += "<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Commands</legend>";
  content += "<table style='width:250px'>";
  content += "<button onclick=\"location.href='/meridian?SIDE=0'\" class=\"button_red\"  type=\"button\">Set EAST</button>&ensp;";
  content += "<button onclick=\"location.href='/meridian?SIDE=1'\" class=\"button_red\"  type=\"button\">Set WEST</button>&ensp;" + checked + "<br>";
  content += "<button onclick=\"location.href='/park'\" class=\"button_red\" type=\"button\">Park</button>&ensp;";
  content += "<button onclick=\"location.href='/home?HOME=0'\" class=\"button_red\" type=\"button\">Reset  home</button>&ensp;";
  content += "<button onclick=\"location.href='/home?HOME=1'\" class=\"button_red\" type=\"button\">GO&Park home</button><br>";
  content += "<button onclick=\"location.href='/track?TRACK=1'\" class=\"button_red\" type=\"button\">Track On</button>&ensp;";
  content += "<button onclick=\"location.href='/track?TRACK=0'\" class=\"button_red\" type=\"button\">Track Off</button><br>";
  content += "<button onclick=\"location.href='/Align'\"class=\"button_red\" type=\"button\">2 stars align</button><br>";
  content += "<button onclick=\"location.href='/focuspos'\"class=\"button_red\" type=\"button\">Focus</button>";
  content += "<button onclick=\"location.href='/wheel'\"class=\"button_red\" type=\"button\">Filter selection (Wheel)</button></table></fieldset>";
  content += "<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Control set</legend>";
  content += "<table style='width:250px'>";
#ifdef IR_CONTROL
  content += "<button onclick=\"location.href='/remote'\" class=\"button_red\" type=\"button\">IR Remote </button><br>";
#endif
#ifdef NUNCHUCK_CONTROL
  content += "<button onclick=\"location.href='/nunchuk?ENABLE=1'\" class=\"button_red\" type=\"button\">Init Nunchuk </button>";
  content += "<button onclick=\"location.href='/nunchuk'\" class=\"button_red\" type=\"button\">Disable Nunchuk </button>";
#endif
#ifdef PAD
  content += "<br><button onclick=\"location.href='/home?HOME=3'\" class=\"button_red\" type=\"button\"> PAD/ST4 </button>";
  content += String(pad_enabled ? " ON" : " OFF");
#endif
  content += "</table></fieldset> <fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Info</legend>";
  content += "<table style='width:250px'>";
  content += "<button onclick=\"location.href='/time'\" class=\"button_red\" type=\"button\">Sync Date/Time</button>";
  content += "<button onclick=\"location.href='/monitor'\" class=\"button_red\" type=\"button\">Monitor Counters</button><br>";
  content += "<button onclick=\"location.href='/iana'\" class=\"button_red\" type=\"button\">IANA Timezone Set </button></table></fieldset>";
  if(WiFi.status()==WL_CONNECTED)
  {
    char wifisignal[80];
    wifiStrength(wifisignal);
    content += "<br>WiFi Signal Strength : "+ String(wifisignal);
  }
  content += "<br>Loaded at Time :" + String(ctime(&now)) + String(NTP_Sync ? "NTP OK" : "RTC") + " Offset:" + String(getoffset()) + "<br></body></html>";
  
  serverweb.send(200, "text/html", content);
}

void handleInstructions(void) {
  serverweb.send(200, "text/html", "<html><style>" BUTTTEXTT "</style>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><br>\
<h2>Instructions:</h2> \
1-Sync. Date/time and go back to 2 stars aling <br> \
2-Click `Star1 Select`<br> \
3-Toggle to Skysafari, choose  a star, center and align it<br>\
4-Toggle to webserver and Click `Star2 Select`<br>\
5-Toggle to Skysafary again, choose different star, center and align it<br>\
6-Click `Sync. mode` <br>\
->>Try not to choose very high or very low stars, with some azimuth and altitude gap between them.<br>\
->>If you do mistake, go back to Sync date/time and start from new <br><br>\
->>WATCHOUT: Performing date/time Sync will delete align data!.<br><br>\
</body ></html>");

  //  serverweb.send(200, "text/html", content);
}

void handleStarInstructions(void) {
  serverweb.send(200, "text/html",
                 "<html><frameset cols=\"50%,50%\" frameborder=\"0\" border=\"0\">\
   <frame name=\"main\" src=\"/Align\"> \
   <frame name=\"_instructions\" src=\"\" style=\"background-color:#000000;\"> \
   </frameset> \
   <noframes> \
   <script>location=\"/Align\";</script>\
   </noframes></html>");
}
void handleTmc(void) {
  char temp[4500];
  char msg[43];
  time_t now;
  now = time(nullptr);
  snprintf(msg, 42, "TMC %s ", ctime(&now));
  if (serverweb.hasArg("ra_msteps") && serverweb.hasArg("dec_msteps")) {
    snprintf(temp, 4500, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n#\n",
             serverweb.arg("ra_msteps"), serverweb.arg("ra_mamps"), serverweb.arg("ra_t"), String(serverweb.hasArg("ra_spread")), String(serverweb.hasArg("ra_pol")),
             serverweb.arg("dec_msteps"), serverweb.arg("dec_mamps"), serverweb.arg("de_t"), String(serverweb.hasArg("de_spread")), String(serverweb.hasArg("de_pol")),
             serverweb.arg("z_msteps"), serverweb.arg("z_mamps"), serverweb.arg("z_t"), String(serverweb.hasArg("z_spread")), String(serverweb.hasArg("z_pol")),
             serverweb.arg("e_msteps"), serverweb.arg("e_mamps"), serverweb.arg("e_t"), String(serverweb.hasArg("e_spread")), String(serverweb.hasArg("e_pol")));

    File f = SPIFFS.open(TMC_FILE, "w");
    f.println(temp);
    f.close();
    tmcinit();
    snprintf(msg, 42, "TMC Saved at %s ", ctime(&now));
  }
  tmc_readcnf(tmcmotors);

  snprintf(temp, 4500,
           "<html><style>" BUTTTEXTT TEXTT3 "</style>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\">\
<form action='/tmc' method='POST'><h2>ESP32go - TMC</h2>\
<fieldset style=\"width:15%;border-radius:15px\"><legend>TMC values</legend><table style='width:200px'>\
<tr><th>TMC</th> <th>RA </th><th>Dec</th><th>F1  </th><th>F2  </th></tr>\
<tr><td>Msteps</td>\
<td><input type='number' name='ra_msteps' class=\"text_red\" value='%d'></td>\
<td><input type='number' name='dec_msteps' class=\"text_red\" value='%d'></td>\
<td><input type='number' name='z_msteps' class=\"text_red\" value='%d'></td>\
<td><input type='number' name='e_msteps' class=\"text_red\" value='%d'></td>\
</tr>\
<tr><td>mAmps</td>\
<td><input type='number' name='ra_mamps'class=\"text_red\"  value='%d'></td>\
<td><input type='number' name='dec_mamps'class=\"text_red\"  value='%d'></td>\
<td><input type='number' name='z_mamps'class=\"text_red\"  value='%d'></td>\
<td><input type='number' name='e_mamps'class=\"text_red\"  value='%d'></td>\
</tr>\
<tr><td>Trigger</td>\
<td><input type='number' name='ra_t'class=\"text_red\"  value='%d'></td>\
<td><input type='number' name='de_t'class=\"text_red\"  value='%d'></td>\
<td><input type='number' name='z_t'class=\"text_red\"  value='%d'></td>\
<td><input type='number' name='e_t'class=\"text_red\"  value='%d'></td></tr>\
</tr>\
<tr><td>Spcycle</td>\
<td><input type='checkbox' name='ra_spread' value='1' %s></ td>\
<td><input type='checkbox' name='de_spread' value='1' %s></ td>\
<td><input type='checkbox' name='z_spread' value='1' %s></ td>\
<td><input type='checkbox' name='e_spread' value='1' %s></ td>\
</tr>\
<tr><td>Interpol</td>\
<td><input type = 'checkbox' name='ra_pol' value='1' %s></ td>\
<td><input type = 'checkbox' name='de_pol' value='1' %s></ td>\
<td><input type = 'checkbox' name='z_pol' value='1' %s></ td>\
<td><input type = 'checkbox' name='e_pol' value='1' %s></ td>\
</tr>\
</table>\
<input type='submit' name='SUBMIT'  class=\"button_red\" value='Save'></fieldset></form>%s<br>\
<button onclick=\"location.href='/'\"class=\"button_red\" type=\"button\">Back</button> <br>\
</body></html>",
           tmcmotors[RA].msteps, tmcmotors[DE].msteps, tmcmotors[F1].msteps, tmcmotors[F2].msteps,
           tmcmotors[RA].mamps, tmcmotors[DE].mamps, tmcmotors[F1].mamps, tmcmotors[F2].mamps,
           tmcmotors[RA].sp_trigger, tmcmotors[DE].sp_trigger, tmcmotors[F1].sp_trigger, tmcmotors[F2].sp_trigger,
           tmcmotors[RA].spread ? "checked" : "", tmcmotors[DE].spread ? "checked" : "",
           tmcmotors[F1].spread ? "checked" : "", tmcmotors[F2].spread ? "checked" : "",
           tmcmotors[RA].pol ? "checked" : "", tmcmotors[DE].pol ? "checked" : "",
           tmcmotors[F1].pol ? "checked" : "", tmcmotors[F2].pol ? "checked" : "",
           msg);

  serverweb.send(200, "text/html", temp);
}
void handleIana(void) {
  if (serverweb.hasArg("IANA")) {
    String net = serverweb.arg("IANA");
    net.toCharArray(tzstr, net.length() + 1);
    File f = SPIFFS.open(IANA_FILE, "w");
    if (!f) {
      net = ("file open failed");
    } else
      f.println(net);
    f.close();
  }

  String content = "<html><head><style>" BUTT TEXTT2 "</style>" AUTO_SIZE "</head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>IANA</h2><br>";
  content += "<form action='/iana' method='POST'>";
  content += "<table style='width:200px'><tr><td><input type='text' name='IANA' class=\"text_red2\" value='" + String(tzstr) + "'></td><td>";
  content += "<input type='submit' name='SUBMIT'  class=\"button_red\" value='Set zone'></tr></form></table>";
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Home</button><br>";



  content += "</body></html>";
  serverweb.send(200, "text/html", content);
  //  timerAlarmDisable(timer_alt);
  //  timerAlarmEnable(timer_alt);
}

void handleAux() {
  char temp[4500] = "";
  if (serverweb.hasArg("FOCUSMAX")) {
    snprintf(temp, 700, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n#\n",
             serverweb.arg("FOCUSMAX"), serverweb.arg("FOCUSPEEDLOW"), serverweb.arg("FOCUSPEED"), serverweb.arg("PWR_DIR"),
             serverweb.arg("AUXMAX"), serverweb.arg("AUXSPEEDLOW"), serverweb.arg("AUXSPEED"), serverweb.arg("AUX_PWR_DIR"),
             serverweb.arg("DC_FOCUS"), serverweb.arg("IDMOTOR"), serverweb.arg("SLOTS"));
    File f = SPIFFS.open(AUX_FILE, "w");
    if (!f) {

      snprintf(temp, 42, "File open failed");
    } else {
      f.println(temp);
      f.close();
      readauxconfig();
      // now = time(nullptr);
      // snprintf(msg, 42, "Config Saved at %s ", ctime(&now));
    }
  }
  String aux0 = "";
  String aux1 = "";
  String aux2 = "";
  String aux3 = "";
  switch (aux_motor.id) {
    case 0: aux0 = "selected"; break;
    case 1: aux1 = "selected"; break;
    case 2: aux2 = "selected"; break;
    case 3: aux3 = "selected"; break;
  }
  snprintf(temp, 4500,
           "<html><style>" BUTTTEXTT TEXTT3 "</style>" AUTO_SIZE
           "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><form action='/aux' method='POST'><h2>Aux Motors</h2>\
<fieldset style=\"width:15% ; border-radius:15px;\">\
<table style='width:250px'>\
<tr><th>Param</th> <th>Focus</th><th>Aux</th></tr>\
<tr><td>Max travel:</td><td><input type='number'step='1' name='FOCUSMAX' class=\"text_red\" value='%d'></td>\
<td><input type='number'step='1' name='AUXMAX' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Low Speed:</td><td><input type='number'step='1' name='FOCUSPEEDLOW' class=\"text_red\" value='%d'></td>\
<td><input type='number'step='1' name='AUXSPEEDLOW' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Speed</td><td><input type='number'step='1' name='FOCUSPEED' class=\"text_red\" value='%d'></td>\
<td><input type='number'step='1' name='AUXSPEED' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Volt</td><td><input type='number'step='1' name='PWR_DIR' class=\"text_red\" value='%d'></td>\
<td><input type='number'step='1' name='AUX_PWR_DIR' class=\"text_red\" value='%d'></td></tr>\
<tr><td>Stepper<input type='radio' name='DC_FOCUS' value='0' %s ></td><td>Dc<input type='radio' name='DC_FOCUS' value='1' %s ></td>\
<td><select name='IDMOTOR' id='lang'> <option value='0' %s>Focuser</option>\
<option value='1' %s>Rotator</option> <option value='2' %s >Filter Wheel</option>\
<option value='3' %s >Disabled</option></select>\
</td></tr><tr><td></td><td>Slots</td>\
<td><input type='number'step='1' name='SLOTS' class=\"text_red\" value='%d'></td></tr>\
</table>\
<input type='submit' name='SUBMIT' class=\"button_red\" value='Save'>\
</fieldset>\
<br><button onclick=\"location.href='/focus'\" class=\"button_red\" type=\"button\">Focuser set</button>\
<br><br><button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button>\
</form>\
</body></html>",
           focus_motor.max_steps, aux_motor.max_steps, rconv(focus_motor.speed_low), rconv(aux_motor.speed_low), rconv(focus_motor.speed), rconv(aux_motor.speed),
           focus_motor.pwm * (focus_motor.inv ? -1 : 1), aux_motor.pwm * (aux_motor.inv ? -1 : 1),
           dcfocus == 0 ? "checked" : "", dcfocus == 1 ? "checked" : "", aux0, aux1, aux2, aux3, wheel_slots);
  serverweb.send(200, "text/html", temp);
}



void handleWheel() {
  String msg = "";

  if (serverweb.hasArg("SYNC")) {

    aux_motor.position = 0;
    wheel_index = 0;
    msg = "Reset Home sync";
  } else

    if (serverweb.hasArg("MOVE")) {
    String net = serverweb.arg("MOVE");
    wheel_index = net.toInt();
    aux_motor.target = wheel[wheel_index].value;
    move_to(&aux_motor, aux_motor.target, aux_motor.speed);
    msg = "Filter Changed to " + String(wheel[wheel_index].name);
  }


  String content = "<html><head><style>" BUTT TEXTT "</style>" AUTO_SIZE "</head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>Filter Wheel</h2><br>";
  content += "<form action='/wheel' method='POST'>";
  content += "<select name='MOVE' id='lang'>";
  for (byte g = 0; g < 9; g++) {
    content += "<option value='" + String(g) + "' " + (wheel_index == g ? "selected" : "") + ">" + String(wheel[g].name) + "</option>";
  }
  content += "</select><input type='submit' name='SUBMIT'  class=\"button_red\" value='Set Filter'>";
  content += "<input type='submit' name='SYNC'  class=\"button_red\" value='Sync home'></form><br>" + msg;
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}

void handlewheelcgf() {
  char temp[100];
  char tbl[800] = "";
  char msg[43] = "";
  File f;
  String num;
  if (serverweb.hasArg("SLOT0")) {
    for (int n = 0; n < 9; n++) {
      num = String(n);
      snprintf(temp, 100, "%s\n%s\n%s\n", serverweb.arg("NAME" + num), serverweb.arg("SLOT" + num), num);
      strcat(tbl, temp);
    }
    strcat(tbl, "#\n");

    f = SPIFFS.open(WHEEL_FILE, FILE_WRITE, true);
    if (!f) {
      f.close();

      //snprintf(table, 42, "File open failed");
    } else {
      f.println(tbl);
      f.close();
      read_wheel_config();
    }
    snprintf(msg, 42, "Config Saved at %s ", tbl);
  }


  //snprintf(temp,800,
  char tmp[4500] = "<html><style>" BUTTTEXTT TEXTT3 "</style>" AUTO_SIZE
                   "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><form action='/wheelconfig' method='POST'><h2>Filter wheel</h2>\
<fieldset style=\"width:15% ; border-radius:15px;\"> <legend>Wheel</legend>\
<table style='width:250px'>\
<tr><th>Slot</th> <th>Filter</th><th>Counter</th></tr>";
  int n = 0;
  for (n = 0; n < 9; n++) {
    snprintf(tbl, 600, "<tr><td>Slot %d:</td><td><input type='text' name='NAME%d' class=\"text_red\" value='%s'></td>\
<td><input type='number'step='1' name='SLOT%d' class=\"text_red\" value='%d'></td></tr>",
             n, n, wheel[n].name, n, wheel[n].value);
    strcat(tmp, tbl);
  }
  //  strcat(tmp,msg);
  strcat(tmp, "</table>\
<input type='submit' name='SUBMIT' class=\"button_red\" value='Save'>\
</fieldset>\
<br><button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button>\
</form>\
</body></html>");
  //strcat(tmp, table);

  serverweb.send(200, "text/html", tmp);
}

void handlehomecfg() {
  String msg = "";
  char home_index;
  if (serverweb.hasArg("SYNC")) {
    set_home(telescope);

    msg = "Home set to current telescope Position";
  } else

    if (serverweb.hasArg("HOME")) {
    String net = serverweb.arg("HOME");
    home_index = net.toInt() + 48;
    mount_fix_home(home_index, telescope);
    switch (home_index) {
      case '0':
        net = "Polar";
        break;
      case '1':
        net = "Zenit";
        break;
      case '2':
        net = "East";
        break;
      case '3':
        net = "West";
        break;
      case '4':
        net = "Polar-East";
        break;
    }
    msg = " Changed to " + net;
  }

  home_index=get_home_index();
  String hi = "NONE";
  switch (home_index) {
      case '0':
        hi = "Polar";
        break;
      case '1':
        hi = "Zenit";
        break;
      case '2':
        hi = "East";
        break;
      case '3':
        hi = "West";
        break;
      case '4':
        hi = "Polar-East";
        break;
      case '9':
        hi = "Custom";
        break;
      default:
        home_index = 'x';
  }

  String content = "<html><head><style>" BUTT TEXTT "</style>" AUTO_SIZE "</head><body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>Mount Home Config</h2>";
  if(hi != "NONE")
  {
    content += "Current value: " + hi;
    if(home_index != 'x')
      content += " ("+String(telescope->az_home)+" / "+String(telescope->alt_home)+")";
  }
  content += "<br><br>";
  content += "<form action='/homecfg' method='POST'>";
  content += "<select name='HOME' id='lang'>";
  content += "<option value='0' >Polar</option>";
  content += "<option value='4' "+String(home_index=='4' ? "selected":"")+">Polar-East</option>";
  content += "<option value='1' "+String(home_index=='1' ? "selected":"")+">Zenith</option>";
  content += "<option value='2' "+String(home_index=='2' ? "selected":"")+">East</option>";
  content += "<option value='3' "+String(home_index=='3' ? "selected":"")+">West</option>";


  content += "</select><input type='submit' name='SUBMIT'  class=\"button_red\" value='Set Home'>";
  content += "<input type='submit' name='SYNC'  class=\"button_red\" value='Sync home'></form><br>" + msg;
  content += "<button onclick=\"location.href='/'\" class=\"button_red\" type=\"button\">Back</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}

void handleTrack(void) {
  int track;
  String msg,net;
  if (serverweb.hasArg("TRACK")) {
     net = serverweb.arg("TRACK");
    track = net.toInt();
    if (track) {
      mount_stop(telescope, 'w');
      mount_move(telescope, 't');
    } else
      mount_move(telescope, 'h');
    ;

    msg = "Track";
  }

  String content = "<html>" AUTO_SIZE "<body  bgcolor=\"#000000\" text=\"" TEXT_COLOR "\"><h2>TrackControl</h2><br>";
  content += net+"<br>";
  content += "AZ Counter:" + String(telescope->azmotor->counter) + "<br>";
  content += "Alt Counter:" + String(telescope->altmotor->counter) + "<br>";
  content += "<button onclick=\"location.href='/'\"  type=\"button\">Home</button><br>";
  content += "</body></html>";
  serverweb.send(200, "text/html", content);
}

void initwebserver(void) {
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
  serverweb.on("/focuspos", handleFocusPos);
  serverweb.on("/config", handleConfig);
  serverweb.on("/", handleMain);
  serverweb.on("/main", handleMain);
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
  serverweb.on("/tmc", handleTmc);
  serverweb.on("/iana", handleIana);
  serverweb.on("/aux", handleAux);
  serverweb.on("/wheel", handleWheel);
  serverweb.on("/wheelconfig", handlewheelcgf);
  serverweb.on("/homecfg", handlehomecfg);
  serverweb.on("/track", handleTrack);
  serverweb.onNotFound([]() {
    if (!handleFileRead(serverweb.uri()))
      serverweb.send(404, "text/plain", "FileNotFound");
  });

  serverweb.begin();
}
#endif
