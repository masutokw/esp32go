#ifndef WHEEL_H_INCLUDED
#define WHEEL_H_INCLUDED
#include "conf.h"
 typedef struct
{char  name[9];
unsigned int value,offset;
 } wheel_t;
void write_wheel_config(void);
void init_wheel_counters(uint8_t slots,long int max_steps);
#endif

  