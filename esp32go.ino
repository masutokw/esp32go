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
#include "tmc.h"
#include "esp32-hal-ledc.h"
#include "wheel.h"
#ifdef MOONLITE_FOCUS
#include "moonlite_focus.h"
#endif
//Comment out undesired Feature at conf.h
#define FORMAT_SPIFFS_IF_FAILED true
#ifdef NUNCHUCK_CONTROL
#include "nunchuck.h"
#endif
#ifdef TMC_DRIVERS
#include <TMCStepper.h>
//extern TMC_DEVICE driver_ra, driver_dec, driver_z, driver_e;
#endif
#ifdef MOONLITE_FOCUS
int moonlite_index = -1;
extern String moon_response;
#endif
volatile int stepcounter1, stepcounter2;
uint64_t volatile period_az, period_alt;
int volatile azcounter, altcounter, azbackcounter, altbackcounter;
boolean volatile az_active = true;
boolean volatile alt_active = true;
int volatile azdir, altdir;
int encb;
String a;
hw_timer_t* timer_az = NULL;
hw_timer_t* timer_alt = NULL;
hw_timer_t* timer_focus = NULL;
hw_timer_t* timer_aux = NULL;
#ifdef OTA
#include <ArduinoOTA.h>
#include "OTA_helper.hpp"
#endif
#ifdef IR_CONTROL
#include "ir_control.h"
#endif
#include <FS.h>
#ifdef RTC_IC
#include "RTClib.h"
DateTime RTC_now;
RTC_IC rtc;
#endif
#ifdef ENCODER
#include "encoder.h"
#endif
const char* TZstr = "GMT-1";
extern long sdt_millis;
#if __has_include("wifipass.h")
#include "wifipass.h"  //comment wifipass.h and uncomment for your  wifi parameters
#else
const char* ssid = "MyWIFI";
const char* password = "Mypassword";
#endif
extern volatile int state;
extern stepper focus_motor, aux_motor, *pmotor;
extern int focusvolt;
WiFiServer server(SERVER_PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];
int clients_connected = 0;
BluetoothSerial SerialBT;
WebServer serverweb(WEB_PORT);
HTTPUpdateServer httpUpdater;
bool bnunchuk = 0;
char buff[512] = "Waiting for connection..";
const char* pin = "0000";
extern char response[200];
extern char tzstr[50];
byte otab = 0;
bool bt_on = true;
bool ap_on = true;
mount_t* telescope;
c_star volatile st_now, st_target, st_current, st_1, st_2;
String ssi;
String pwd;
String ap_ssid;
Ticker speed_control_tckr, counters_poll_tkr;
extern long command(char* str);
time_t now;
time_t init_time;
char counter;
int wifi_pad_IP3 = 0;
int wifi_pad_IP2 = 0;
bool NTP_Sync = false;
void timeavailable(struct timeval* t) {
  //Serial.println("Got time adjustment from NTP!");
  NTP_Sync = true;
  telescope->time_zone = getoffset();
#ifdef RTC_IC
  rtc.adjust(DateTime(time(nullptr)));
#endif
}

void IRAM_ATTR nunchuk_reset() {
  // nunchuck_init(SDA_PIN, SCL_PIN);
  bnunchuk = true;
}

void IRAM_ATTR onTimer_az() {
  uint32_t delay;
  stepcounter1++;

  if (azdir) {
    digitalWrite(CLOCK_OUT_AZ, 0);
    int backlash = telescope->azmotor->backlash;
    char active = telescope->azmotor->active;

    if ((azdir == 1) && (azbackcounter == 0) || (azdir == -1) && (azbackcounter == backlash)) {
      azcounter += azdir;
      if ((active) && (timerAlarmRead(timer_az)) != period_az) timerAlarmWrite(timer_az, period_az, true);
    } else {
      azbackcounter += -azdir;
      if ((active) && (period_az > AZBACKSPD)) timerAlarmWrite(timer_az, AZBACKSPD, true);
    }


    if (azcounter < 0) azcounter = telescope->azmotor->maxcounter;
    if (azcounter > telescope->azmotor->maxcounter) azcounter = 0;
  }
#ifdef AZ_P_DELAY
  char pulse_w;
  for (pulse_w = 0; pulse_w < AZ_P_DELAY; pulse_w++) __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;");
#endif
  digitalWrite(CLOCK_OUT_AZ, 1);
}

void IRAM_ATTR onTimer_alt() {
  stepcounter2++;
  if (altdir) {
    digitalWrite(CLOCK_OUT_ALT, 0);
    int backlash = telescope->altmotor->backlash;
    char active = telescope->altmotor->active;


    if ((altdir == 1) && (altbackcounter == 0) || (altdir == -1) && (altbackcounter == backlash)) {
      altcounter += altdir;
      if ((active) && (timerAlarmRead(timer_alt)) != period_alt) timerAlarmWrite(timer_alt, period_alt, true);
    } else {
      altbackcounter += -altdir;
      if ((active) && (period_alt > ALTBACKSPD)) timerAlarmWrite(timer_alt, ALTBACKSPD, true);
    }

    if (altcounter < 0) altcounter = telescope->altmotor->maxcounter;
    if (altcounter > telescope->altmotor->maxcounter) altcounter = 0;
  }

#ifdef ALT_P_DELAY
  char pulse_w;
  for (pulse_w = 0; pulse_w < ALT_P_DELAY; pulse_w++) __asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop;");
#endif
  digitalWrite(CLOCK_OUT_ALT, 1);
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
    while (SerialBT.available()) buff[n++] = (char)SerialBT.read();
    buff[n] = 0;

    //  Serial.write((const uint8_t* )buff, n);
    command(buff);
#ifdef LX200TRACE
    Serial.write((const uint8_t*)buff, strlen(buff));
    Serial.print("   ");
#endif
    buff[n] = 0;
    SerialBT.write((const uint8_t*)response, strlen(response));
#ifdef LX200TRACE
    Serial.write((const uint8_t*)response, strlen(response));
    Serial.println();
#endif
  }
}

int net_task(void) {
  int lag = millis();
  size_t n;
  uint8_t i, j;
  //Sky Safari does not make a persistent connection, so each commnad query is managed as a single independent client.
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        //kill connections with marked selected ip for wifipad
        if ((serverClients[i].remoteIP()[3] == wifi_pad_IP3) && (serverClients[i].remoteIP()[2] == wifi_pad_IP2)) {
          for (j = 0; j < MAX_SRV_CLIENTS; j++) {
            if ((serverClients[j].remoteIP()[3] == wifi_pad_IP3) && (serverClients[j].remoteIP()[2] == wifi_pad_IP2) && (j != i))
              serverClients[j].stop();
          }
        }
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
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      clients_connected++;
      if (serverClients[i].available()) {
        //get data from the  client and push it to LX200 FSM

        while (serverClients[i].available()) {
          delay(1);
          size_t n = serverClients[i].available();
          serverClients[i].readBytes(buff, n);
          buff[n] = 0;
#ifdef MOONLITE_FOCUS
          if (strncmp(buff, ":GV#", 4) == 0) {
            moonlite_index = i;
          }
          if (i == moonlite_index) {
            moonlite_cmd(buff);
            // moonclient.write(moon_response.c_str());
            serverClients[i].write(moon_response.c_str());
          } else
#endif
        { command(buff);
#ifdef LX200TRACE
          Serial.write((const uint8_t*)buff, strlen(buff));
          Serial.print("   ");
#endif
          buff[n] = 0;
          serverClients[i].write((char*)response, strlen(response));
#ifdef LX200TRACE
          Serial.write((const uint8_t*)response, strlen(response));
          Serial.println();
#endif
         } //checkfsm();
        }
      }
    } else if (serverClients[i]) {
      serverClients[i].stop();
    }
  }
  return millis() - lag;
}
void serialtask(void) {
  if (Serial.available()) {
    char n = 0;
    delay(2);
    while (Serial.available()) buff[n++] = (char)Serial.read();
#ifdef BT_TRACE_USB
    SerialBT.write((const uint8_t*)buff, n);
    SerialBT.println(n);
#endif
    buff[n] = 0;
    command(buff);
    buff[n] = 0;
    Serial.write((const uint8_t*)response, strlen(response));
#ifdef BT_TRACE_USB
    SerialBT.write((const uint8_t*)response, strlen(response));
    SerialBT.println();
#endif
  }
}

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);
  generate_wave(127);
  delay(300);
  pinMode(ENABLE_AZ, OUTPUT);
  pinMode(ENABLE_ALT, OUTPUT);
  digitalWrite(ENABLE_AZ, DEN_DRIVER);
  digitalWrite(ENABLE_ALT, DEN_DRIVER);
  Serial.begin(BAUDRATE);
  Serial.setDebugOutput(false);
#ifndef STEP_FOCUS
  ledcAttachPin(PWM_B, 1);  // assign RGB led pins to channels
  ledcAttachPin(PWM_A, 2);
  ledcSetup(1, 10000, 8);  // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 10000, 8);
  ledcWrite(1, 127);
  ledcWrite(2, 127);
#endif
#ifdef OLED_DISPLAY
  oled_initscr();
#endif
#ifdef NUNCHUCK_CONTROL
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  delay(10);
  nunchuck_init(SDA_PIN, SCL_PIN);
#endif
#ifdef RTC_IC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  } else {
    RTC_now = rtc.now();
    // Serial.println(RTC_now.unixtime());
    timeval tv = { RTC_now.unixtime(), 0 };
    settimeofday(&tv, nullptr);
  }

#endif
  // SerialBT.enableSSP();
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_BT);

  if (!SPIFFS.begin()) {
    SPIFFS.begin(true);
    ESP.restart();
  }

  File f;
  bool sta_ok = false;
  uint8_t i = 0;
  if (SPIFFS.exists(WIFI_FILE)) {
    WiFi.mode(WIFI_STA);
    f = SPIFFS.open(WIFI_FILE, FILE_READ);
    ssi = f.readStringUntil('\n');
    pwd = f.readStringUntil('\n');
    f.close();
    ssi.trim();
    pwd.trim();
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssi.c_str(), pwd.c_str());
    while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
    if (WiFi.status() == WL_CONNECTED) {
      sta_ok = true;
    } else {
      WiFi.disconnect(true);
    }
  }
  if (SPIFFS.exists(NETWORK_FILE)) {
    f = SPIFFS.open(NETWORK_FILE, "r");
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;
    ip.fromString(f.readStringUntil('\n'));
    subnet.fromString(f.readStringUntil('\n'));
    gateway.fromString(f.readStringUntil('\n'));
    dns.fromString(f.readStringUntil('\n'));
    if (sta_ok)
      WiFi.config(ip, gateway, subnet, dns);
    otab = f.readStringUntil('\n').toInt();
    String tmp_value = f.readStringUntil('\n');
    if (tmp_value != "")
      bt_on = tmp_value.toInt();
    tmp_value = f.readStringUntil('\n');
    if (tmp_value != "")
      ap_on = tmp_value.toInt();
    ap_ssid = f.readStringUntil('\n');
    ap_ssid.trim();
    if (ap_ssid.length() < 3)
      ap_ssid = String(SSID_AP);
    f.close();
  }
  if (!sta_ok || ap_on) {
    if (!sta_ok) {
      WiFi.mode(WIFI_AP);
      otab = 0;
    } else {
      WiFi.mode(WIFI_AP_STA);
    }
    //WiFi.softAP(SSID_AP, PASS_AP);
    if (!WiFi.softAP(ap_ssid.c_str(), PASS_AP)) {
      ap_ssid = String(SSID_AP);
      WiFi.softAP(SSID_AP, PASS_AP);
    }
  }
  if (bt_on) {
    SerialBT.begin(BT_NAME + String(baseMac[4], HEX) + "_" + String(baseMac[5], HEX));
    SerialBT.setPin(pin);
  } else {
    WiFi.setSleep(false);
  }

#ifdef FIXED_IP
  IPAddress ip(192, 168, 1, FIXED_IP);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 0, 0);
  //  IPAddress DNS(192, 168, 1, 1);
  WiFi.config(ip, gateway, subnet, gateway);
#endif

  delay(500);
  i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if (WiFi.status() != WL_CONNECTED) WiFi.disconnect(true);

#ifdef OLED_DISPLAY
  oled_waitscr();
#endif

  //start UART and the server
#ifdef TMC_DRIVERS

  tmc_boot();
  // tmcinit();

#endif

#ifdef OLED_DISPLAY
  // Serial.swap();
#endif
  //
  server.begin();
  server.setNoDelay(true);
  init_stepper(&focus_motor, DIR_OUT_FOCUS, CLOCK_OUT_FOCUS, ENABLE_FOCUS);
  init_stepper(&aux_motor, DIR_OUT_AUX, CLOCK_OUT_AUX, ENABLE_AUX);
  telescope = create_mount();
  pmotor = &focus_motor;
  readconfig(telescope);
  load_home(telescope);
  readauxconfig();
  read_wheel_config();
  // setaux(false);
  //write_wheel_config();
  httpUpdater.setup(&serverweb);
  sntp_set_time_sync_notification_cb(timeavailable);
  f = SPIFFS.open(IANA_FILE, "r");
  if (!f) {
    config_NTP(telescope->time_zone, 0);
  } else {
    String s = f.readStringUntil('\n');
    s.toCharArray(tzstr, s.length() + 1);
    config_NTP(telescope->time_zone, tzstr);
  }
  f.close();

  if (WiFi.status() == WL_CONNECTED) {
    int cn = 0;
    now = time(nullptr);

    // while ((now < EPOCH_1_1_2023) && (cn++) < 5) {
    while (!NTP_Sync && (cn++) < 5) {
      delay(500);
      now = time(nullptr);
    }
    init_time = time(nullptr);
  }
#ifdef WEB_INTERFACE
  initwebserver();
#else
  serverweb.begin();

#endif
  // focuser_tckr.attach_ms(5, do_step, &focus_motor);
  if (telescope->mount_mode == EQ) {
    sdt_init(telescope->longitude, telescope->time_zone);
    speed_control_tckr.attach_ms(SPEED_CONTROL_TICKER, thread_motor, telescope);
    // counters_poll_tkr.attach_ms(COUNTERS_POLL_TICKER, eq_track, telescope);

  } else {
    tak_init(telescope);
    mount_track_off(telescope);
    speed_control_tckr.attach_ms(SPEED_CONTROL_TICKER, thread_motor2, telescope);
    // counters_poll_tkr.attach_ms(COUNTERS_POLL_TICKER, track, telescope);
  }

  pinMode(CLOCK_OUT_AZ, OUTPUT);
  pinMode(CLOCK_OUT_ALT, OUTPUT);
  pinMode(DIR_OUT_AZ, OUTPUT);
  pinMode(DIR_OUT_ALT, OUTPUT);
#ifdef ESP32_38
#ifndef TMC_DRIVERS
  pinMode(AZ_RES, OUTPUT);
  pinMode(ALT_RES, OUTPUT);
  digitalWrite(AZ_RES, 1);
  digitalWrite(ALT_RES, 1);
#endif
#endif
#ifndef STEP_FOCUS
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
#endif
  digitalWrite(ENABLE_AZ, EN_DRIVER);
  digitalWrite(ENABLE_ALT, EN_DRIVER);

#ifdef STEP_FOCUS
  pinMode(CLOCK_OUT_FOCUS, OUTPUT);
  pinMode(DIR_OUT_FOCUS, OUTPUT);
  pinMode(ENABLE_FOCUS, OUTPUT);
  digitalWrite(ENABLE_FOCUS, DEN_DRIVER);
  pinMode(CLOCK_OUT_AUX, OUTPUT);
  pinMode(DIR_OUT_AUX, OUTPUT);
  pinMode(ENABLE_AUX, OUTPUT);
  digitalWrite(ENABLE_AUX, DEN_DRIVER);
#endif
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer_alt = timerBegin(TIMER_ALT, 80, true);
  timer_az = timerBegin(TIMER_AZ, 80, true);
  timer_focus = timerBegin(TIMER_FOCUS, 80, true);
  timer_aux = timerBegin(TIMER_AUX, 80, true);


  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer_az, &onTimer_az, false);
  timerAttachInterrupt(timer_alt, &onTimer_alt, false);
  timerAttachInterrupt(timer_focus, &do_step, false);
  timerAttachInterrupt(timer_aux, &aux_ISR, false);

  timerAlarmWrite(timer_az, 100000, true);
  timerAlarmWrite(timer_alt, 100000, true);
  timerAlarmWrite(timer_focus, 100000, true);
  timerAlarmWrite(timer_aux, 100000, true);
  // Start an alarm
  timerAlarmEnable(timer_az);
  timerAlarmEnable(timer_alt);
  timerAlarmDisable(timer_focus);
  timerAlarmDisable(timer_aux);
  timerAlarmWrite(timer_aux, 100, true);
  pinMode(0, INPUT_PULLUP);
  attachInterrupt(0, nunchuk_reset, FALLING);

  //move_to(&focus_motor, focus_motor.position, 2000);
  stopfocuser();
  // WA_O;
  // WB_O;
  // focuser_tckr.detach();
  if (telescope->mount_mode == EQ) telescope->azmotor->targetspeed = telescope->track_speed;

#ifdef PAD
  pad_Init();
#endif  //PAD

#ifdef OTA
  if (otab) InitOTA(ap_ssid);
#endif
#ifdef IR_CONTROL
  ir_init();
#endif
#ifdef NUNCHUCK_CONTROL
  nunchuck_disable(nunchuck_read() == 0);
#endif
#ifdef ENCODER
  encb = init_encoder();
#endif

#ifdef BUZZER_PIN
  pinMode(BUZZER_PIN, OUTPUT);
  buzzerOn(300);
  // delay(300);
  // buzzerOff();
#endif
#ifdef MOONLITE_FOCUS
  moonlite_init();
#endif
}

void loop() {

  // buzzerOff();
  int zcount[2];
  delay(10);
  net_task();
#ifndef BT_TRACE_USB
  if (bt_on)
    bttask();
#endif
#ifndef LX200TRACE
  serialtask();
#endif

  now = time(nullptr);
  serverweb.handleClient();
#ifdef IR_CONTROL
  if (counter % 6 == 0)
    ir_read();
#endif
#ifdef NUNCHUCK_CONTROL
  if (bnunchuk) {
    nunchuck_init(SDA_PIN, SCL_PIN);
    nunchuck_disable(nunchuck_read() == 0);
    bnunchuk = 0;
  };
  if (counter % 10 == 3)
    nunchuck_read();
#endif

#ifdef OLED_DISPLAY
  if ((counter % 100) == 75) oledDisplay();
#endif
#ifdef PAD
  doEvent();
#endif

#ifdef OTA
  if ((counter % 10 == 0) && (otab))
    ArduinoOTA.handle();
#endif
#if defined(RTC_IC) && defined(RTC_NVRAM) && RTC_NVRAM > 0
  if (((counter % (RTC_NVRAM * 100)) == 0) && telescope->is_tracking) {
#if RTC_IC == RTC_DS3231
    zcount[0] = azcounter;
    zcount[1] = altcounter;
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_NVADDR);
    Wire.write((uint8_t*)zcount, (uint8_t)7);
    Wire.endTransmission();

#else
    rtc.writenvram(RTC_NVADDR, (uint8_t*)&azcounter, 4);
    rtc.writenvram(RTC_NVADDR + 4, (uint8_t*)&altcounter, 4);
#endif
  }
#endif
#ifdef MOONLITE_FOCUS
  if (counter % 2 == 0)
    moonlite_handle();
#endif
  //step_out(stepcounter++ % 8);
  counter++;
}
