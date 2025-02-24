#include "conf.h"
#include "tmc.h"
#include <TMCStepper.h>


#ifdef TMC_DRIVERS

extern TMC_DEVICE driver_ra, driver_dec, driver_z, driver_e;

#endif

void tmc_init(void) {
  int ra_msteps, ra_mamps, dec_msteps, dec_mamps, z_msteps, z_mamps, e_msteps, e_mamps, cycle, ra_pwmtrigger, dec_pwmtrigger, z_pwmtrigger, e_pwmtrigger;
  if (SPIFFS.exists(TMC_FILE)) {
    File f = SPIFFS.open(TMC_FILE, "r");

    ra_msteps = f.readStringUntil('\n').toInt();
    ra_mamps = f.readStringUntil('\n').toInt();
    ra_pwmtrigger = f.readStringUntil('\n').toInt();
    dec_msteps = f.readStringUntil('\n').toInt();
    dec_mamps = f.readStringUntil('\n').toInt();
    dec_pwmtrigger = f.readStringUntil('\n').toInt();
    z_msteps = f.readStringUntil('\n').toInt();
    z_mamps = f.readStringUntil('\n').toInt();
    z_pwmtrigger = f.readStringUntil('\n').toInt();
    e_msteps = f.readStringUntil('\n').toInt();
    e_mamps = f.readStringUntil('\n').toInt();
    e_pwmtrigger = f.readStringUntil('\n').toInt();
    cycle = f.readStringUntil('\n').toInt();






    f.close();
  } else {
    ra_msteps = dec_msteps = 32;
    ra_mamps = dec_mamps = 300;
    z_msteps = e_msteps = 8;
    z_mamps = e_mamps = 100;
    cycle = 0;
    ra_pwmtrigger = dec_pwmtrigger = z_pwmtrigger = e_pwmtrigger = 0;
  }
#ifdef TMC_DRIVERS

  tmc_cmd(driver_ra, ra_msteps,ra_mamps,1,cycle,ra_pwmtrigger);
 // tmc_cmd(driver_dec, dec_msteps,dec_mamps,1,cycle,dec_pwmtrigger);
  //tmc_cmd(driver_z, z_msteps, z_mamps,1, cycle,z_pwmtrigger);
 // tmc_cmd(driver_e, e_msteps,e_mamps,1,cycle,e_pwmtrigger);


#endif
}
void tmc_cmd(TMC_DEVICE& tmc, uint16_t res, uint16_t current, uint8_t pol, uint8_t spread, uint32_t pwmTrigger) {
#ifdef TMC_DRIVERS
  if (res > 1) tmc.microsteps(res);
  if (current > 0) tmc.rms_current(current);
  if (pol < 2) tmc.intpol(pol);
  if (spread < 2) tmc.en_spreadCycle(spread);
  if (pwmTrigger < 1000) driver_z.TPWMTHRS(pwmTrigger);
#endif
}
