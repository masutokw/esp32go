#include "conf.h"
#ifdef ENCODER
#include "AS5600.h"
#include "Wire.h"

AS5600 as5600;   

int init_encoder(void)
{
 as5600.begin();  
  
 return as5600.isConnected();
 }
int  read_raw_encoder(void)
{
 return as5600.rawAngle() * AS5600_RAW_TO_DEGREES;
}
#endif
