#ifdef OLED_13
 #include "SH1106.h"
#else
 #include "SSD1306.h"
#endif
#include "mount.h"
#include "misc.h"
void oledDisplay();
void oled_initscr(void);
void oled_waitscr(void);
void oled_check(void);
