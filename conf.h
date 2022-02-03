#ifndef CONF_H_INCLUDED
#define CONF_H_INCLUDED
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <SPIFFS.h>
#include "BluetoothSerial.h"
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

#define NUNCHUCK_CONTROL
//#define IR_CONTROL
//#define PAD
//#define OLED_DISPLAY
#define OTA
//#define FIXED_IP 13


//mandatory
#define EPOCH_1_1_2019 1546300800
#define SERVER_PORT 10001
#define WEB_PORT 80
#define BAUDRATE 115200
#define MAX_SRV_CLIENTS 3
#define SPEED_CONTROL_TICKER 20
#define COUNTERS_POLL_TICKER 100
#define STEP_PULSE 23
#endif


#define PWM_A 13
#define PWM_B 12
#define AIN_1 33
#define AIN_2 32
#define BIN_1 27
#define BIN_2 14

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
