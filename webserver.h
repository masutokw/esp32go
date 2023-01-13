#ifndef WEBSERVER_H_INCLUDED
#define WEBSERVER_H_INCLUDED
#include "conf.h"
#include "mount.h"
#include "misc.h"
#include <FS.h>
//#define NIGTH_VISION
#ifdef NIGTH_VISION 
#define BUTTTEXTT ".button_red {text-align: right; background-color:#100000;color:yellow;border-color:red; border-radius:5px} .text_red {text-align: right;height:20px; width:90px; background-color:black;color: red;border:0}"
#define BUTT ".button_red {text-align: right; background-color:#100000;color:yellow;border-color:red; border-radius:5px}"
#define TEXTT  ".text_red {text-align: right;height:20px; width:90px; background-color:black;color: red;border:0}"
#define TEXTT1  ".text_red {text-align: right;height:20px; width:45px; background-color:black;color: red;border:1}"
#define TEXT_COLOR "#FF6000"

#else
#define BUTTTEXTT ".button_red {text-align: right; background-color:#3333ff;color:yellow;border-color:blue; border-radius:5px}.text_red {text-align: right;height:20px; width:90px; background-color:black;color:white;border:0}" 
#define BUTT ".button_red {text-align: right; background-color:#3333ff;color:yellow;border-color:blue; border-radius:5px}"
#define TEXTT  ".text_red {text-align: right;height:20px; width:90px; background-color:black;color:white;border:0}" 
#define TEXTT1  ".text_red {text-align: right;height:20px; width:45px; background-color:black;color:white;border:1}"
#define TEXT_COLOR "#5599ff"
#endif  
#define AUTO_SIZE "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
  
extern WebServer serverweb;
extern mount_t *telescope;
extern String ssi;
extern String pwd;
extern byte otab;
void initwebserver(void);

#endif // WEBSERVER_H_INCLUDED
