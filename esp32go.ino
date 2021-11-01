#include <Arduino.h>
#include "conf.h"
#include <time.h>
#include <Ticker.h>
#include <Wire.h>
#include "misc.h"
#include "mount.h"
#include "webserver.h"
#include "taki.h"

//Comment out undesired Feature
//---------------------------
//#define NUNCHUCK_CONTROL
//#define FIXED_IP 21
//#define OLED_DISPLAY
//#define PAD
//--------------------------------
#ifdef  NUNCHUCK_CONTROL
#include "nunchuck.h"
#endif
#define  CLOCK_OUT_AZ 4
#define DIR_OUT_AZ 19
#define  CLOCK_OUT_ALT 5
#define DIR_OUT_ALT 2
uint64_t  volatile period_az, period_alt;
int volatile azcounter, altcounter;
int  volatile azdir, altdir;
String a;
hw_timer_t * timer_az = NULL;
hw_timer_t * timer_alt = NULL;
#ifdef OTA
#include <ArduinoOTA.h>
#include "OTA_helper.hpp"
#endif
#ifdef   IR_CONTROL
#include "ir_control.h"
#endif
#include <FS.h>
const char *TZstr = "GMT-1";
extern long sdt_millis;
//#include "wifipass.h" //comment wifipass.h and uncomment for your  wifi parameters
const char* ssid = "MySSID";
const char* password = "MyPass";
extern volatile int state;

WiFiServer server(SERVER_PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];

BluetoothSerial SerialBT;
WebServer serverweb(WEB_PORT);
HTTPUpdateServer httpUpdater;

char buff[50] = "Waiting for connection..";
char *pin = "0000";
extern char  response[200];
byte napt = 0;
mount_t *telescope;
c_star volatile st_now, st_target, st_current, st_1, st_2;
String ssi;
String pwd;
Ticker speed_control_tckr, counters_poll_tkr;
extern long command( char *str );
time_t now;
time_t init_time;
void IRAM_ATTR onTimer_az() {

  if (azdir) {
    digitalWrite(CLOCK_OUT_AZ, 0);
    azcounter += azdir;

    if (azcounter < 0)  azcounter = telescope->azmotor->maxcounter;
    if (azcounter > telescope->azmotor->maxcounter)  azcounter = 0;
    digitalWrite(CLOCK_OUT_AZ, 1);
    // digitalWrite(CLOCK_OUT_AZ, !digitalRead(CLOCK_OUT_AZ));
  }
}
void IRAM_ATTR onTimer_alt() {
  if (altdir) {
    digitalWrite(CLOCK_OUT_ALT, 0);
    altcounter += altdir;
    if (altcounter < 0) altcounter = telescope->altmotor->maxcounter;
    if (altcounter > telescope->altmotor->maxcounter)  altcounter = 0;
    digitalWrite(CLOCK_OUT_ALT, 1);
  }
}
#ifdef PAD
#include "pad.h"
#endif

#ifdef OLED_DISPLAY
#include "SSD1306.h"
#include "oled.h"
//#include "SH1106.h"
//SSD1306 display(0x3c, D5, D6);
#endif

void bttask(void) {
  if (SerialBT.available()) {
    char n = 0;
    while (SerialBT.available())  buff[n++] = (char) SerialBT.read() ;
    command(buff);
    buff[n] = 0;
    SerialBT.write((const uint8_t* )response, strlen(response));

  }
}


int net_task(void)
{
  int lag = millis();
  size_t n;
  uint8_t i;
  //Sky Safari does not make a persistent connection, so each commnad query is managed as a single independent client.
  if (server.hasClient())
  {
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
    {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected())
      {
        if (serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        continue;
      }
    }
    //Only one client at time, so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for (i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (serverClients[i] && serverClients[i].connected())
    {
      if (serverClients[i].available())
      {
        //get data from the  client and push it to LX200 FSM

        while (serverClients[i].available())
        {
          delay(1);
          size_t n = serverClients[i].available();
          serverClients[i].readBytes(buff, n);
          command( buff);
          buff[n] = 0;
          serverClients[i].write((char*)response, strlen(response));

          //checkfsm();
        }

      }
    }
  }
  return millis() - lag;
}
void serialtask(void) {
  if (Serial.available()) {
    char n = 0;
    while (Serial.available())  buff[n++] = (char) Serial.read() ;
    command(buff);
    buff[n] = 0;
    Serial.write((const uint8_t* )response, strlen(response));

  }
}

void setup()
{

#ifdef OLED_DISPLAY
  oled_initscr();
#endif

  SerialBT.setPin(pin);
  SerialBT.begin("PGTA_ESP32");
  SerialBT.setPin(pin);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP-PGT", "boquerones");
  // WiFi.softAP("TUBOSAURIO", "acebuche");
  SPIFFS.begin();
  File f = SPIFFS.open("/wifi.config", "r");
  if (f)
  {
    ssi = f.readStringUntil('\n');
    pwd = f.readStringUntil('\n');
    f.close();
    ssi.trim(); pwd.trim();
    WiFi.begin(ssi.c_str(), pwd.c_str());
  }
  else {
    SPIFFS.format();
    WiFi.begin(ssid, password);
  }
  f = SPIFFS.open("/network.config", "r");
  if (f)
  { IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;
    if (ip.fromString(f.readStringUntil('\n')) && subnet.fromString(f.readStringUntil('\n')) && gateway.fromString(f.readStringUntil('\n')) + dns.fromString(f.readStringUntil('\n'))) {
      WiFi.config(ip, gateway, subnet, dns);
      napt = f.readStringUntil('\n').toInt();
    }

    f.close();
  }
#ifdef FIXED_IP
  IPAddress ip(192, 168, 1, FIXED_IP);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 0, 0);
  //  IPAddress DNS(192, 168, 1, 1);
  WiFi.config(ip, gateway, subnet, gateway);
#endif

  delay(500);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if  (WiFi.status() != WL_CONNECTED) WiFi.disconnect(true);
#ifdef NAPT
  else
  { if (napt) {
      dhcps_set_dns(1, WiFi.gatewayIP());
      dhcps_set_dns(0, WiFi.dnsIP(0));
      err_t ret = ip_napt_init(NAPT, NAPT_PORT);
      if (ret == ERR_OK) {
        ret = ip_napt_enable_no(SOFTAP_IF, napt);
      }
    }
  }
#endif
#ifdef OLED_DISPLAY
  oled_waitscr();
#endif

  //start UART and the server
  Serial.begin(BAUDRATE);
#ifdef OLED_DISPLAY
  // Serial.swap();
#endif
  //
  server.begin();
  server.setNoDelay(true);
  telescope = create_mount();
  readconfig(telescope);
  httpUpdater.setup(&serverweb);
  config_NTP(telescope->time_zone, 0);
  if  (WiFi.status() == WL_CONNECTED)
  { int cn = 0;  now = time(nullptr);

    while ((now < EPOCH_1_1_2019) && (cn++) < 5) {
      delay(500);
      now = time(nullptr);
    }
    init_time = time(nullptr);

  }

  initwebserver();
  if (telescope->mount_mode == EQ) {
    sdt_init(telescope->longitude, telescope->time_zone);
    speed_control_tckr.attach_ms(SPEED_CONTROL_TICKER, thread_motor, telescope);
   // counters_poll_tkr.attach_ms(COUNTERS_POLL_TICKER, eq_track, telescope);
  }
  else
  { tak_init(telescope);
    speed_control_tckr.attach_ms(SPEED_CONTROL_TICKER, thread_motor2, telescope);
  }





#ifdef PAD
  pad_Init();
#endif //PAD
#ifdef NUNCHUCK_CONTROL
  // nunchuck_init(D6, D5);
  nunchuck_init(2, 0);
#endif
#ifdef OTA
  InitOTA();
#endif
#ifdef IR_CONTROL
  ir_init();
#endif

  // Set BTN_STOP_ALARM to input mode
  // pinMode(BTN_STOP_ALARM, INPUT);
  pinMode(CLOCK_OUT_AZ, OUTPUT);
  pinMode(CLOCK_OUT_ALT, OUTPUT);
  pinMode(DIR_OUT_AZ, OUTPUT);
  pinMode(DIR_OUT_ALT, OUTPUT);
  // Create semaphore to inform us when the timer has fired
  // timerSemaphore = xSemaphoreCreateBinary();

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer_alt = timerBegin(2, 80, true);
  timer_az = timerBegin(3, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer_az, &onTimer_az, true);
  timerAttachInterrupt(timer_alt, &onTimer_alt, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer_az, 100000, true);
  timerAlarmWrite(timer_alt, 100000, true);

  // Start an alarm
  timerAlarmEnable(timer_az);
  timerAlarmEnable(timer_alt);
}

void loop()
{
  delay(10);
  net_task();
  bttask();
  //serialtask();

  now = time(nullptr);
  serverweb.handleClient();
#ifdef IR_CONTROL
  ir_read();
#endif
#ifdef  NUNCHUCK_CONTROL
  nunchuck_read();
#endif

#ifdef OLED_DISPLAY
  oledDisplay();
#endif
#ifdef PAD
  doEvent();
#endif

#ifdef OTA
  ArduinoOTA.handle();
#endif

}
