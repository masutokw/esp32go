#ifndef TMC_H_INCLUDED
#define TMC_H_INCLUDED 
#include <TMCStepper.h>
#include "conf.h"
//---------- TMC
// IMPORTANT for FYSETC E4: remove all factory shunts from the FYSETC board
// and use a jumper wire to connect the Z-MIN (GPIO15) pin
// to the TMC2209TX pin on the board (between SCL and TXD)
//

#define TMC_SERIAL_PORT Serial2
#ifdef ESP32_38
#define TMC_SERIAL_TX_PIN 19 // for esp32 38 pin
#define TMC_SERIAL_RX_PIN 16 //used
#else
#define TMC_SERIAL_TX_PIN 15 /// FYSETC Z-MIN pin, ESP32 GPIO 15
#define TMC_SERIAL_RX_PIN 35  // not really used, but needed
#endif
#define TMC_R_SENSE 0.11f 
#define TMC_TYPE 9
#if ( TMC_TYPE == 9 )
#define TMC_DEVICE TMC2209Stepper
#define TMC_DRIVER_RA_ADDRESS  ,0b01 // TMC2209 Driver address according to MS1 and MS2 (MS1 jumper ON)
#define TMC_DRIVER_DEC_ADDRESS ,0b11 // TMC2209 Driver address according to MS1 and MS2 (MS1 and MS2 jumpers ON)
#define TMC_DRIVER_Z_ADDRESS ,0b00 // TMC2209 Driver address according to MS1 and MS2 (no jumper)
#define TMC_DRIVER_E_ADDRESS ,0b10 // TMC2209 Driver address according to MS1 and MS2 (MS2 jumper ON)
#else
#define TMC_DEVICE TMC2208Stepper
#define TMC_DRIVER_RA_ADDRESS  
#define TMC_DRIVER_DEC_ADDRESS 
#define TMC_DRIVER_Z_ADDRESS 
#define TMC_DRIVER_E_ADDRESS 
#endif
typedef enum { RA,
               DE,
               F1,F2 } tmc_enum_t;

typedef struct{

   uint16_t msteps,mamps;
   uint8_t pol,spread,address;
   uint32_t sp_trigger;
  
}tmcmotor_t;
bool tmc_readcnf(tmcmotor_t tm[]);
void tmc_boot(void);
void tmcinit(void);
void tmc_cmd(TMC_DEVICE& tmc, uint16_t res, uint16_t current, uint8_t pol, uint8_t spread,uint32_t cycleTrigger);
//void tmc_cmd(tmcmotor_t tmcindex);
void tmc_cmd (tmcmotor_t tmcindex,TMC_DEVICE& tmc);
#endif

