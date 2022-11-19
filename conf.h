#ifndef CONF_H_INCLUDED
#define CONF_H_INCLUDED
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <SPIFFS.h>
#include "BluetoothSerial.h"
#define ENCODER
#define BT_NAME "ESP32go_BT"
#define IR_PIN 15
#define SDA_PIN 21
#define SCL_PIN 22
//#define PROTO
#ifdef PROTO
#define  CLOCK_OUT_AZ 4
#define DIR_OUT_AZ 2
#define  CLOCK_OUT_ALT 5
#define  DIR_OUT_ALT 18
#define ENABLE_AZ 16
#define ENABLE_ALT 23
#define AZ_RES 17
#define ALT_RES 19
#else
#define  CLOCK_OUT_AZ 4
#define DIR_OUT_AZ 2
#define  CLOCK_OUT_ALT 18
#define  DIR_OUT_ALT 5
#define ENABLE_AZ 17
#define ENABLE_ALT 23
#define AZ_RES 16
#define ALT_RES 19
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

//#define NUNCHUCK_CONTROL
#define IR_CONTROL
//#define PAD
//#define OLED_DISPLAY
//#define OLED_13 // use 1.3 SSH1106 (OLED_DISPLAY is needed also)
#define OTA
//#define FIXED_IP 13


//mandatory
#define EPOCH_1_1_2019 1546300800
#define SERVER_PORT 10001
#define WEB_PORT 80
#define BAUDRATE 115200
//#define BAUDRATE 57600
#define MAX_SRV_CLIENTS 8
#define SPEED_CONTROL_TICKER 20
#define COUNTERS_POLL_TICKER 100
#endif


#define PWM_A 13
#define PWM_B 12
#define AIN_1 33
#define AIN_2 32
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
#else
#define TIMER_AZ 3
#define TIMER_ALT 2
#endif
//#define DC_FOCUS
//On slow pulse detect stepper driver as DRV8825 comment out
//#define AZ_P_DELAY 23
//#define ALT_P_DELAY 23
// Use NCP as default position for EQ mount
//#define NCP_HOME
//Trace WIFI lx200 protocol using output serial interface
//#define LX200TRACE
