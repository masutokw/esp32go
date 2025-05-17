#ifndef CONF_H_INCLUDED
#define CONF_H_INCLUDED
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <SPIFFS.h>
#include "BluetoothSerial.h"
#include "sntp.h"
#include "fysetc.h"
//#include "tmc.h"
//#define   RTC_IC RTC_DS3231
//#define   RTC_IC RTC_DS1307
#define   RTC_NVRAM 0   //writing nvram interval in seconds (only when tracking),if 0 disables.
#if RTC_IC==RTC_DS3231
#define RTC_NVADDR 0x07
#define RTC_ADDRESS 0x68
#else
#define RTC_NVADDR 0x00  //0x7 for
#endif
#define WEB_INTERFACE
//#define ENCODER
#define BT_NAME "ESP32go_BT"
#define IR_PIN 15
#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 26
///-------------------------------------------------------------
#define TMC_DRIVERS   //uncomment if using tmc driver
#define STEP_FOCUS   // uncomment when using 3d driver for focuser //comment when tb6612
//#define M_STEP         //uncomment when TB6612 microsteppig 
#define FYSECT_BRD   //uncommnet for FYSECT 3D-printer board
//#define PROTO  //uncomment for not Angel´s board
//-----------------------------------------------
#ifdef FYSECT_BRD
#define CLOCK_OUT_AZ X_STEP
#define DIR_OUT_AZ X_DIR
#define CLOCK_OUT_ALT Y_STEP
#define DIR_OUT_ALT Y_DIR
#define ENABLE_AZ MOTOR_EN
#define ENABLE_ALT MOTOR_EN
#define ENABLE_FOCUS 23 // MOTOR_EN    // ---> TEMPORARY PATCH
#define AZ_RES 17
#define ALT_RES 19
#define CLOCK_OUT_FOCUS E_STEP 
#define DIR_OUT_FOCUS E_DIR
#define STEP_FOCUS
#define CLOCK_OUT_AUX Z_STEP
#define DIR_OUT_AUX Z_DIR
#define ENABLE_AUX 23 // MOTOR_EN    // ---> TEMPORARY PATCH
#define TMC_DRIVERS
#define BUZZER_PIN 13
#else
#define CLOCK_OUT_FOCUS  12 
#define DIR_OUT_FOCUS 13
#define ENABLE_FOCUS 14
#define CLOCK_OUT_AUX 32
#define DIR_OUT_AUX 33
#define ENABLE_AUX 19

#ifdef PROTO
#define CLOCK_OUT_AZ  4
#define DIR_OUT_AZ 2
#define CLOCK_OUT_ALT 5
#define DIR_OUT_ALT 18
#define ENABLE_AZ 16
#define ENABLE_ALT 23
#define AZ_RES 17
#define ALT_RES 19

#else
#define CLOCK_OUT_AZ 4
#define DIR_OUT_AZ 2
#define CLOCK_OUT_ALT 18
#define DIR_OUT_ALT 5
#define ENABLE_AZ 17
#define ENABLE_ALT 23
#define AZ_RES 16
#define ALT_RES 19
#define ESP32_38
#endif
#endif
//optional
//#define ENABLE_HIGH
#ifdef ENABLE_HIGH
#define EN_DRIVER 1
#define DEN_DRIVER 0
#else
#define EN_DRIVER 0
#define DEN_DRIVER 1
#endif

#define NUNCHUCK_CONTROL
//#define IR_CONTROL
//#define PAD
#ifdef PAD
// IMPORTANT: All pad pins (PAD_X and PIN_MODE) need an external 10K pull-up
#define PAD_N 36
#define PAD_S 39
#define PAD_W 34
#define PAD_E 35
#define PIN_MODE 25
#endif
//#define OLED_DISPLAY
//#define OLED_13 // use 1.3 SSH1106 (OLED_DISPLAY is needed also)
#define OTA
//#define FIXED_IP 17


//mandatory
#define EPOCH_1_1_2023 1673275134
#define SERVER_PORT 10001
#define WEB_PORT 80
#define BAUDRATE 115200
//#define BAUDRATE 57600
#define MAX_SRV_CLIENTS 8
#define SPEED_CONTROL_TICKER 20
#define COUNTERS_POLL_TICKER 100
#endif

//#define DRV_8833
#ifdef DRV_8833
#define PWM_A 33
#define PWM_B 32
#define AIN_1 13
#define AIN_2 12
#else
#define PWM_A 13
#define PWM_B 12
#define AIN_1 33
#define AIN_2 32
#endif

#define BIN_1 14
#define BIN_2 27

#define AZ_INV
#ifdef AZ_INV
#define AZ_CW 1
#define AZ_CCW 0
#else
#define AZ_CW 0
#define AZ_CCW 1
#endif

#define ALT_INV
#ifdef ALT_INV
#define ALT_CW 1
#define ALT_CCW 0
#else
#define ALT_CW 0
#define ALT_CCW 1
#endif
// timer 3 serial conflict?
//
#ifdef IR_CONTROL
#define TIMER_AZ 0
#define TIMER_ALT 1
#define TIMER_FOCUS 3
#define TIMER_AUX 2
#else
#define TIMER_AZ 3
#define TIMER_ALT 2
#define TIMER_FOCUS 0
#define TIMER_AUX 1
#endif


//On slow pulse detect stepper driver as DRV8825 comment out
#define AZ_P_DELAY 23
#define ALT_P_DELAY 23
// Use NCP as default position for EQ mount
//#define NCP_HOME
// Retry NTP connection up to 3 trimes to get network time before mount init
//#define RETRY_NTP
//Trace WIFI lx200 protocol using output serial interface
//#define LX200TRACE
//#define  BT_TRACE_USB
#define AZBACKSPD 226
#define ALTBACKSPD 226
#define WIFI_FILE "/wifi.config"
#define MOUNT_FILE "/mount.config"
#define NETWORK_FILE "/network.config"
#define IR_FILE "/remote.config"
#define TMC_FILE "/tmc.config"
#define IANA_FILE "/iana.config"
#define AUX_FILE "/focus.config"
#define WHEEL_FILE "/wheel.config"
#define SSID_AP  "ESP32go"
#define PASS_AP  "boquerones"
#define TZ_SPAIN "CET-1CEST,M3.5.0,M10.5.0/3"
//selected rate at boot 0:guide,1:center,2:find,3:slew.
#define I_SELECT_RATE 3
//#define HIRES_TIMER