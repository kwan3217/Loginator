#ifndef main_h
#define main_h

#define ON   1
#define OFF  0

#define LP_LOAD          (1 >> 0)
#define LP_GPSLOCK       (1 >> 1)
#define LP_BLINKLOCK     (1 >> 2)
#define LP_AUTOBAUD      (1 >> 3)
#define LP_DUMPFIRMWARE  (1 >> 4)
#define LP_ALWAYS        (1 >> 5)
#define LP_PPS           (1 >> 6)

void blinklock(int maintainWatchdog, int code);
void set_light(int num, int onoff, int purpose);
void delay(int);

#endif
