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
#define  CLOCK_OUT_AZ 4
#define DIR_OUT_AZ 2
#define  CLOCK_OUT_ALT 5
#define  DIR_OUT_ALT 18
#define ENABLE_AZ 16
#define ENABLE_ALT 23
//optional

#define NUNCHUCK_CONTROL
//#define IR_CONTROL
//#define PAD
//#define OLED_DISPLAY
//#define OTA
//#define FIXED_IP 13


//mandatory
#define EPOCH_1_1_2019 1546300800
#define SERVER_PORT 10001
#define WEB_PORT 80
#define BAUDRATE 19200
#define MAX_SRV_CLIENTS 3
#define SPEED_CONTROL_TICKER 20
#define COUNTERS_POLL_TICKER 100
#endif
