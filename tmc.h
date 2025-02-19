#ifndef TMC_H_INCLUDED
#define TMC_H_INCLUDED 

//---------- TMC
// IMPORTANT for FYSETC E4: remove all factory shunts from the FYSETC board
// and use a jumper wire to connect the Z-MIN (GPIO15) pin
// to the TMC2209TX pin on the board (between SCL and TXD)
//

#define TMC_SERIAL_PORT Serial2
#define TMC_SERIAL_TX_PIN 15 // FYSETC Z-MIN pin, ESP32 GPIO 15
#define TMC_SERIAL_RX_PIN 35 // not really used, but needed
#define TMC_R_SENSE 0.11f 
#define TMC_TYPE 9
#if ( TMC_TYPE == 9 )
#define TMC_DEVICE TMC2209Stepper
#define TMC_DRIVER_RA_ADDRESS  ,0b11 // TMC2209 Driver address according to MS1 and MS2 (MS1 jumper ON)
#define TMC_DRIVER_DEC_ADDRESS ,0b01 // TMC2209 Driver address according to MS1 and MS2 (MS1 and MS2 jumpers ON)
#define TMC_DRIVER_Z_ADDRESS ,0b00 // TMC2209 Driver address according to MS1 and MS2 (no jumper)
#define TMC_DRIVER_E_ADDRESS ,0b10 // TMC2209 Driver address according to MS1 and MS2 (MS2 jumper ON)
#else
#define TMC_DEVICE TMC2208Stepper
#define TMC_DRIVER_RA_ADDRESS  
#define TMC_DRIVER_DEC_ADDRESS 
#define TMC_DRIVER_Z_ADDRESS 
#define TMC_DRIVER_E_ADDRESS 
#endif
void tmc_init(void);

#endif

