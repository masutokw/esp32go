#ifndef WEBSERVER_H_INCLUDED
#define WEBSERVER_H_INCLUDED
#include "conf.h"
#include "mount.h"
#include "misc.h"
#include <FS.h>
//#define NIGTH_VISION
//#define ALTERNATE_VISION // jordix colors
#ifdef NIGTH_VISION
#define TEXT_COL "red"
#define BUT_BG "#100000"
#define BUT_COLOR "yellow"
#define BUT_BORDER_COLOR "red"
#define BUT_BORDER "5px"
#define TEXT_COLOR "#FF6000"

#elif defined(ALTERNATE_VISION)
#define TEXT_COL "silver"
#define BUT_BG "#8c0318"
#define BUT_COLOR "silver"
#define BUT_BORDER_COLOR "silver"
#define BUT_BORDER "10px"
#define TEXT_COLOR "#df8a07"
#else

#define TEXT_COL "white"
#define BUT_BG "#3333ff"
#define BUT_COLOR "yellow"
#define BUT_BORDER_COLOR "blue"
#define BUT_BORDER "5px"
#define TEXT_COLOR "#5599ff"
#endif

#define BUTT ".button_red {text-align: right; background-color:" BUT_BG ";color:" BUT_COLOR ";border-color:" BUT_BORDER_COLOR"; border-radius:" BUT_BORDER "}"
#define AUTO_SIZE "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
#define TEXTT  ".text_red {text-align: right;height:20px; width:90px; background-color:black;color: "  TEXT_COL " ;border:0}"
#define TEXTT1  ".text_red {text-align: right;height:20px; width:45px; background-color:black;color:"  TEXT_COL ";border:1}"
#define BUTTTEXTT  BUTT TEXTT
extern WebServer serverweb;
extern mount_t *telescope;
extern String ssi;
extern String pwd;
extern byte otab;
void initwebserver(void);

#endif // WEBSERVER_H_INCLUDED
