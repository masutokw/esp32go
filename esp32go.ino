#include <Arduino.h>
#include "conf.h"
#include <time.h>
#include <Ticker.h>
#include <Wire.h>
#include "misc.h"
#include "mount.h"
#include "webserver.h"
#include "taki.h"
#include "tb6612.h"
#include "focus.h"
#include "esp32-hal-ledc.h"
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
volatile int stepcounter1, stepcounter2;
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
const char* ssid = "MyWIFI";
const char* password = "Mypassword";
extern volatile int state;
extern stepper focus_motor;
extern int8_t focusinv;
extern int focusvolt;
WiFiServer server(SERVER_PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];
int clients_connected = 0;

BluetoothSerial SerialBT;
WebServer serverweb(WEB_PORT);
HTTPUpdateServer httpUpdater;
bool bnunchuk = 0;
char buff[50] = "Waiting for connection..";
const char *pin = "0000";
extern char  response[200];
byte napt = 0;
mount_t  *telescope;
c_star volatile st_now, st_target, st_current, st_1, st_2;
String ssi;
String pwd;
Ticker speed_control_tckr, counters_poll_tkr, focuser_tckr;
extern long command( char *str );
time_t now;
time_t init_time;
char counter;
void IRAM_ATTR nunchuk_reset() {
  // nunchuck_init(SDA_PIN, SCL_PIN);
  bnunchuk = true;
}
void IRAM_ATTR onTimer_az() {
  uint32_t delay;
  stepcounter1++;
  if (azdir) {

    digitalWrite(CLOCK_OUT_AZ, 0);
    char pulse_w;
    azcounter += azdir;
    if (azcounter < 0)  azcounter = telescope->azmotor->maxcounter;
    if (azcounter > telescope->azmotor->maxcounter)  azcounter = 0;
#ifdef AZ_P_DELAY
    for (pulse_w = 0; pulse_w < AZ_P_DELAY; pulse_w++) __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;");
#endif
    digitalWrite(CLOCK_OUT_AZ, 1);
    // digitalWrite(CLOCK_OUT_AZ, !digitalRead(CLOCK_OUT_AZ));
  }
}
void IRAM_ATTR onTimer_alt() {
  stepcounter2++;
  if (altdir) {
    char pulse_w;
    digitalWrite(CLOCK_OUT_ALT, 0);
    altcounter += altdir;
    if (altcounter < 0) altcounter = telescope->altmotor->maxcounter;
    if (altcounter > telescope->altmotor->maxcounter)  altcounter = 0;
#ifdef ALT_P_DELAY
    for (pulse_w = 0; pulse_w < ALT_P_DELAY; pulse_w++) __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;");
#endif
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
    buff[n] = 0;
    //  Serial.write((const uint8_t* )buff, n);
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
    // WiFiClient serverClient = server.available();
    // serverClient.stop();
    if (i >= MAX_SRV_CLIENTS) {
      //no free/disconnected spot so reject
      server.available().stop();
    }
  }
  //check clients for data
  clients_connected = 0;
  for (i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (serverClients[i] && serverClients[i].connected())
    {
      clients_connected++;
      if (serverClients[i].available())
      {
        //get data from the  client and push it to LX200 FSM

        while (serverClients[i].available())
        {
          delay(1);
          size_t n = serverClients[i].available();
          serverClients[i].readBytes(buff, n);
          buff[n ] = 0;
          command(buff);
#ifdef LX200TRACE
          Serial.write((const uint8_t* )buff, strlen(buff)); Serial.print("   ");
#endif
          buff[n] = 0;
          serverClients[i].write((char*)response, strlen(response));
#ifdef LX200TRACE
          Serial.write((const uint8_t* )response, strlen(response));Serial.println();
#endif
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
    delay(2);
    while (Serial.available())  buff[n++] = (char) Serial.read() ;
    // SerialBT.write((const uint8_t* )buff, n);
    //  SerialBT.println(n);
    buff[n] = 0;
    command(buff);
    buff[n] = 0;
    Serial.write((const uint8_t* )response, strlen(response));

  }
}

void setup()
{ delay(300);
  pinMode(ENABLE_AZ, OUTPUT);
  pinMode(ENABLE_ALT, OUTPUT);
  digitalWrite(ENABLE_AZ, DEN_DRIVER);
  digitalWrite(ENABLE_ALT, DEN_DRIVER);
  Serial.begin(BAUDRATE);
  ledcAttachPin(12, 1); // assign RGB led pins to channels
  ledcAttachPin(13, 2);
  ledcSetup(1, 10000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 10000, 8);
  ledcWrite(1, focusvolt);
  ledcWrite(2, focusvolt);

#ifdef OLED_DISPLAY
  oled_initscr();
#endif

  //SerialBT.setPin(pin);
  SerialBT.begin(BT_NAME);
  SerialBT.setPin(pin);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32go", "boquerones");
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

#ifdef OLED_DISPLAY
  // Serial.swap();
#endif
  //
  server.begin();
  server.setNoDelay(true);
  telescope = create_mount();
  init_stepper(&focus_motor);
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
  // focuser_tckr.attach_ms(5, do_step, &focus_motor);
  if (telescope->mount_mode == EQ) {
    sdt_init(telescope->longitude, telescope->time_zone);
    speed_control_tckr.attach_ms(SPEED_CONTROL_TICKER, thread_motor, telescope);
    // counters_poll_tkr.attach_ms(COUNTERS_POLL_TICKER, eq_track, telescope);

  }
  else
  { tak_init(telescope);
    speed_control_tckr.attach_ms(SPEED_CONTROL_TICKER, thread_motor2, telescope);
    // counters_poll_tkr.attach_ms(COUNTERS_POLL_TICKER, track, telescope);
  }

#ifdef PAD
  pad_Init();
#endif //PAD
#ifdef NUNCHUCK_CONTROL
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  nunchuck_init( SDA_PIN, SCL_PIN);
  nunchuck_disable(nunchuck_read() == 0);
#endif
#ifdef OTA
  InitOTA();
#endif
#ifdef IR_CONTROL
  ir_init();
#endif

  pinMode(CLOCK_OUT_AZ, OUTPUT);
  pinMode(CLOCK_OUT_ALT, OUTPUT);
  pinMode(DIR_OUT_AZ, OUTPUT);
  pinMode(DIR_OUT_ALT, OUTPUT);
  pinMode(AZ_RES, OUTPUT);
  pinMode(ALT_RES, OUTPUT);
  //pinMode(PWM_A, OUTPUT);
  //pinMode(PWM_B, OUTPUT);
  pinMode(AIN_1, OUTPUT);
  pinMode(AIN_2, OUTPUT);
  pinMode(BIN_1, OUTPUT);
  pinMode(BIN_2, OUTPUT);
  digitalWrite(PWM_A, 1);
  digitalWrite(PWM_B, 1);
  digitalWrite(AIN_1, 0);
  digitalWrite(AIN_2, 1);
  digitalWrite(BIN_1, 1);
  digitalWrite(BIN_2, 0);
  digitalWrite(ENABLE_AZ, EN_DRIVER);
  digitalWrite(ENABLE_ALT, EN_DRIVER);
  digitalWrite(AZ_RES, 1);
  digitalWrite(ALT_RES, 1);
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).

  timer_alt = timerBegin(TIMER_ALT, 80, true);
  timer_az = timerBegin(TIMER_AZ, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer_az, &onTimer_az, false);
  timerAttachInterrupt(timer_alt, &onTimer_alt, false);

  timerAlarmWrite(timer_az, 100000, true);
  timerAlarmWrite(timer_alt, 100000, true);

  // Start an alarm
  timerAlarmEnable(timer_az);
  timerAlarmEnable(timer_alt);
  pinMode(0, INPUT_PULLUP);
  attachInterrupt(0, nunchuk_reset, FALLING);

  //move_to(&focus_motor,focus_motor.position);
  stopfocuser();
  WA_O; WB_O;
  focuser_tckr.detach();
}

void loop()
{
  delay(10);

  net_task();
  bttask();
#ifndef LX200TRACE
  serialtask();
#endif

  now = time(nullptr);
  serverweb.handleClient();
#ifdef IR_CONTROL
  if (counter % 17 == 0)
    ir_read();
#endif
#ifdef  NUNCHUCK_CONTROL
  if (bnunchuk) {
    nunchuck_init(SDA_PIN, SCL_PIN);
    nunchuck_disable(nunchuck_read() == 0);
    bnunchuk = 0;
  };
  if (counter % 10  == 3)
    nunchuck_read();
#endif

#ifdef OLED_DISPLAY
  oledDisplay();
#endif
#ifdef PAD
  doEvent();
#endif

#ifdef OTA
  if (counter++ % 10 == 0)
    ArduinoOTA.handle();

#endif
  //step_out(stepcounter++ % 8);
}
