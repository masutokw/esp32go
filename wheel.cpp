#include "wheel.h"
#include <SPIFFS.h>
#include "conf.h"
wheel_t wheel[9] = { { "Lum", 0, 0 }, { "Red", 1000, 0 }, { "Green", 2000, 0 }, { "Blue", 3000, 0 }, { "IR", 4000, 0 }, { "IRc", 5000, 0 }, { "HA", 6000, 0 }, { "P", 7000, 0 }, { "OIII", 8000, 0 } };
uint8_t wheel_index;
uint8_t wheel_slots=9;
 int read_wheel_config(void) {
File f;

if (!SPIFFS.exists(WHEEL_FILE)) return -1;

f = SPIFFS.open(WHEEL_FILE, FILE_READ);
String s;
for (byte n = 0; n < 9;n++){
    s = f.readStringUntil('\n');
    s.toCharArray(wheel[n].name, s.length() + 1);
    s = f.readStringUntil('\n');
    wheel[n].value = s.toInt();
    s = f.readStringUntil('\n');
    wheel[n].offset = s.toInt();
  
  }
  f.close();
  return 0;
  
}
void write_wheel_config(void) {
  File f;
  char temp[100];
byte n=0;
  f = SPIFFS.open(WHEEL_FILE, FILE_WRITE,true);
  for (n = 0; n < 9;n++){
      snprintf(temp,100, "%s\n%d\n%d\n", wheel[n].name, wheel[n].value, wheel[n].offset);
      f.print(temp);
    }
      f.print("#\n");
      f.close();
      }
void init_wheel_counters(uint8_t slots,long int max_steps) {
  if (slots==0) slots=9;
long int delta=max_steps/slots; 

for (uint8_t n=0;n<9;n++)
  wheel[n].value= (n<slots)? n*delta :0;
}