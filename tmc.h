#ifndef TMC_H_INCLUDED
#define TMC_H_INCLUDED 

 #define X_STEP 27 //focus
 #define X_DIR 26  //free
 #define Y_STEP 33 //focus
 #define Y_DIR 32  //focus
 #define Z_STEP 14 //focus
 #define Z_DIR 12  //focus
 #define E_STEP 16 //enable o res
 #define E_DIR 17 //enable o res 
 #define MOTOR_EN 25 //pin mode

//---------- TMC
// IMPORTANT for FYSETC E4: remove all factory shunts from the FYSETC board
// and use a jumper wire to connect the Z-MIN (GPIO15) pin
// to the TMC2209TX pin on the board (between SCL and TXD)
//
#define TMC_SERIAL_PORT Serial2
#define TMC_SERIAL_TX_PIN 15 // FYSETC Z-MIN pin, ESP32 GPIO 15
#define TMC_SERIAL_RX_PIN 35 // not really used, but needed
#define TMC_R_SENSE 0.11f 

#define TMC_DRIVER_RA_ADDRESS 0b01 // TMC2209 Driver address according to MS1 and MS2 (MS1 jumper ON)
#define TMC_DRIVER_DEC_ADDRESS 0b11 // TMC2209 Driver address according to MS1 and MS2 (MS1 and MS2 jumpers ON)
#define TMC_DRIVER_Z_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2 (no jumper)
#define TMC_DRIVER_E_ADDRESS 0b10 // TMC2209 Driver address according to MS1 and MS2 (MS2 jumper ON)

void tmc_init(void);

#endif