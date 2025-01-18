#include "conf.h"
#ifdef FYSECT_BRD
#include "fysetc.h"
#include <TMCStepper.h>

#ifdef FYSECT_BRD
extern TMC2209Stepper driver_ra, driver_dec, driver_z, driver_e;
#endif

void tmc_init(void)
{
  int ra_msteps,ra_mamps,dec_msteps,dec_mamps,z_msteps,z_mamps,e_msteps,e_mamps;
  if (SPIFFS.exists(TMC_FILE))
  { 
    File f = SPIFFS.open(TMC_FILE, "r");

    ra_msteps=f.readStringUntil('\n').toInt();
    ra_mamps=f.readStringUntil('\n').toInt();
    dec_msteps=f.readStringUntil('\n').toInt();
    dec_mamps=f.readStringUntil('\n').toInt();
    z_msteps=f.readStringUntil('\n').toInt();
    z_mamps=f.readStringUntil('\n').toInt();
    e_msteps=f.readStringUntil('\n').toInt();
    e_mamps=f.readStringUntil('\n').toInt();

    f.close();
  }
  else
  {
    ra_msteps=32;
    ra_mamps=300;
    dec_msteps=32;
    dec_mamps=300;
    z_msteps=8;
    z_mamps=100;
    e_msteps=8;
    e_mamps=100;
  }
#ifdef FYSECT_BRD
  driver_ra.microsteps(ra_msteps);
  driver_dec.microsteps(dec_msteps);
  driver_z.microsteps(z_msteps);
  driver_e.microsteps(e_msteps);

  driver_ra.rms_current(ra_mamps);
  driver_dec.rms_current(dec_mamps);
  driver_z.rms_current(z_mamps);
  driver_e.rms_current(e_mamps);
#endif

}
#endif
