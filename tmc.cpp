#include "conf.h"
#include "tmc.h"
#include <TMCStepper.h>


#ifdef TMC_DRIVERS
TMC_DEVICE driver_ra = TMC_DEVICE(&TMC_SERIAL_PORT, TMC_R_SENSE TMC_DRIVER_RA_ADDRESS);
TMC_DEVICE driver_dec = TMC_DEVICE(&TMC_SERIAL_PORT, TMC_R_SENSE TMC_DRIVER_DEC_ADDRESS);
TMC_DEVICE driver_z = TMC_DEVICE(&TMC_SERIAL_PORT, TMC_R_SENSE TMC_DRIVER_Z_ADDRESS);
TMC_DEVICE driver_e = TMC_DEVICE(&TMC_SERIAL_PORT, TMC_R_SENSE TMC_DRIVER_E_ADDRESS);
#endif

tmcmotor_t tmcmotors[4] = { { 16, 1000, 1, 0, 0b01, 0 },
                            { 16, 1000, 1, 0, 0b11, 0 },
                            { 8, 400, 1, 0, 0b00, 0 },
                            { 8, 400, 1, 0, 0b10, 0 } };
tmc_enum_t tmc = RA;

bool tmc_readcnf(tmcmotor_t tm[]) {
  
  if (SPIFFS.exists(TMC_FILE)) {
    File f = SPIFFS.open(TMC_FILE, "r");
    for (uint8_t tmc = 0; tmc <= 3; tmc++) {
      tm[tmc].msteps = f.readStringUntil('\n').toInt();
      tm[tmc].mamps = f.readStringUntil('\n').toInt();
      tm[tmc].sp_trigger = f.readStringUntil('\n').toInt();
      tm[tmc].spread = f.readStringUntil('\n').toInt();
      tm[tmc].pol = f.readStringUntil('\n').toInt();
    }
    f.close(); 
    return true;
  }
  return false;
}
void tmc_boot(void) {

#ifdef TMC_DRIVERS

  Serial2.begin(115200, SERIAL_8N1, TMC_SERIAL_RX_PIN, TMC_SERIAL_TX_PIN);
  //Serial2.begin(115200, SERIAL_8N1, 35, 19);
  driver_ra.begin();
  driver_dec.begin();
  driver_z.begin();
  driver_e.begin();
  // driver_ra.pwm_autoscale();
  driver_ra.toff(5);
  driver_dec.toff(5);
  driver_z.toff(5);
  driver_e.toff(5);
  // driver_z.mstep_reg_select(1);
#endif
  tmcinit();
}

void tmcinit(void) {

 /* if (SPIFFS.exists(TMC_FILE)) {
    File f = SPIFFS.open(TMC_FILE, "r");
    for (uint8_t tmc = 0; tmc <= 3; tmc++) {
      tmcmotors[tmc].msteps = f.readStringUntil('\n').toInt();
      tmcmotors[tmc].mamps = f.readStringUntil('\n').toInt();
      tmcmotors[tmc].sp_trigger = f.readStringUntil('\n').toInt();
      tmcmotors[tmc].spread = f.readStringUntil('\n').toInt();
      tmcmotors[tmc].pol = f.readStringUntil('\n').toInt();
    }
    f.close();
  } */
 tmc_readcnf(tmcmotors);
#ifdef TMC_DRIVERS


  tmc_cmd(tmcmotors[RA], driver_ra);
  delay(50);
  tmc_cmd(tmcmotors[DE], driver_dec);
  delay(50);
  tmc_cmd(tmcmotors[F1], driver_z);
  delay(50);
  tmc_cmd(tmcmotors[F2], driver_e);


#endif
}

void tmc_cmd(tmcmotor_t tm, TMC_DEVICE& tmc) {
#ifdef TMC_DRIVERS

  if (tm.msteps > 1) tmc.microsteps(tm.msteps);
  if (tm.mamps > 0) tmc.rms_current(tm.mamps);
  if (tm.pol < 2) tmc.intpol(tm.pol);
  tmc.TPWMTHRS(tm.sp_trigger);
  if (tm.spread < 2) tmc.en_spreadCycle(tm.spread);

#endif
}
