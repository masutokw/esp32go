#include "conf.h"
#ifdef IR_CONTROL
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include "mount.h"
#include "focus.h"
#include "ir_control.h"
#include "tb6612.h"
extern mount_t *telescope;


extern stepper focus_motor;
const uint16_t kRecvPin = IR_PIN;
byte cmd_map [31] = {91, 88, 94, 86, 90, 18, 2, 13, 75, 74, 79, 83, 11, 87, 78, 6, 15, 30, 68, 17, 95, 76, 77, 73, 64, 72, 69, 31, 92, 25, 89};
int obj = 0;
int n = 0;
double ra, dec;
IRrecv irrecv(kRecvPin);
decode_results results;
uint32_t last, lastpresstime, lasti, truecode;
char ir_state = 0;
void ir_init(void)
{
  String s;
  irrecv.enableIRIn();  // Start the receiver
  File f;
  if (SPIFFS.exists(IR_FILE)) {
  f = SPIFFS.open(IR_FILE, "r");
    if (f ) {
      for (uint8_t i = 0; i < 31; i++) {
        s = f.readStringUntil('\n');
        cmd_map[i] = s.toInt();
      } f.close();
    }
  }
}



void ir_read(void)
{
  uint32_t  code;
  if (irrecv.decode(&results))
  {
    if (results.value == 0xFFFFFFFFFFFFFFFF) code = 0xFFFF;
    else
    { truecode = results.command;
#if REMOTE_T == 0

      lasti = last = code =  get_IR_lcode(truecode);
#else
      lasti = last = code = truecode;
#endif
    }
    switch (code)
    {
      // print() & println() can't handle printing long longs. (uint64_t)
      case EAST:
        mount_move(telescope, 'e');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case WEST:
        mount_move(telescope, 'w');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case OK:
        mount_stop(telescope, 'w');
        mount_stop(telescope, 's');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case NORTH:
        mount_move(telescope, 'n');
        lastpresstime = millis();
        ir_state = 1;
        break; ;
      case SOUTH:
        mount_move(telescope, 's');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case S_EAST:
        mount_move(telescope, 's');
        mount_move(telescope, 'e');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case N_EAST:
        mount_move(telescope, 'n');
        mount_move(telescope, 'e');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case S_WEST:
        mount_move(telescope, 's');
        mount_move(telescope, 'w');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case N_WEST:
        mount_move(telescope, 'n');
        mount_move(telescope, 'w');
        lastpresstime = millis();
        ir_state = 1;
        break;
      case FOCUS_F:
        lastpresstime = millis();
        ir_state = 1;
        gotofocuser(focus_motor.max_steps, focus_motor.speed);
        break;
      case FOCUS_B:
        lastpresstime = millis();
        ir_state = 1;
        gotofocuser(0, focus_motor.speed);
        break;
      case GUIDE:
        telescope->srate = 0;
        break;
      case CENTER:
        telescope->srate = 1;
        break;
      case FIND:
        telescope->srate = 2;
        break;
      case SLEW:
        telescope->srate = 3 ;
        break;
      case TRACK:
        telescope->track = 1;
        break;
      case TRACK_OFF:
        mount_track_off(telescope);
        break;
      case B_1 :
        n = n * 10 + 1;
        break;
      case B_2 :
        n = n * 10 + 2;
        break;
      case B_3 :
        n = n * 10 + 3;
        break;
      case B_4 :
        n = n * 10 + 4;
        break;
      case B_5 :
        n = n * 10 + 5;
        break;
      case B_6 :
        n = n * 10 + 6;
        break;
      case B_7 :
        n = n * 10 + 7;
        break;
      case B_8 :
        n = n * 10 + 8;
        break;
      case B_9 :
        n = n * 10 + 9;
        break;
      case B_0 :
        n = n * 10;
        break;
      case GO_TO:
        if ((n < 111) && (n > 0))
        {
          ln_get_equ_prec(Messier[n - 1][0], Messier[n - 1][1], JD2000 + 7665.0, &ra, &dec);//cal
          //telescope->ra_target = ra / (RAD_TO_DEG); telescope->dec_target = dec / (RAD_TO_DEG);
          //goto_ra_dec(telescope, ra / (RAD_TO_DEG), dec / (RAD_TO_DEG));
          if (telescope->mount_mode)
          {
            goto_ra_dec(telescope, ra / (RAD_TO_DEG), dec / (RAD_TO_DEG));
          }
          else {
            telescope->ra_target = ra / (RAD_TO_DEG);
            telescope->dec_target = dec / (RAD_TO_DEG);
            mount_slew(telescope);
          }

        }
        break;
      case CLEAR :
        n = 0;
        break;
      case FLIP_W:
        if (telescope->mount_mode == EQ) meridianflip(telescope, 0);
        break;
      case FLIP_E:
        if (telescope->mount_mode == EQ) meridianflip(telescope, 1);
        break;

      case 0xFFFF:
        lastpresstime = millis();
        ir_state = 1;
        break; //mount_move(telescope, 's');
      default:
        ir_state = 0;
        break;
    }
    // last = code;
    irrecv.resume();  // Receive the next value
  }
  else if ((millis() - lastpresstime > 200) && (ir_state))
  {
    ir_state = 0;
    switch (last)
    {
      case EAST:
      case WEST:
        mount_stop(telescope, 'w');
        break;
      case NORTH:
      case SOUTH:
        mount_stop(telescope, 's');
        break;
      case S_EAST:
      case N_EAST:
      case S_WEST:
      case N_WEST:
        mount_stop(telescope, 'w');
        mount_stop(telescope, 's');
        break;
      case FOCUS_F:
      case FOCUS_B:
        stopfocuser();;
        break;
      default:
        break;

    }
    last = 0xFFFF;
  }
}
uint32_t get_IR_lcode(uint32_t bcode)
{
  uint32_t n;
  for (n = 0; n < 31; n++)
    if (cmd_map[n] == bcode) return n;
  return 0xff;


}
#endif
