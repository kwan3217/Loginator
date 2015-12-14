#include "LPC214x.h"
#include "robot.h"
#include <inttypes.h>

unsigned int HW_TYPE() {return 3;}
unsigned int HW_SERIAL() {return 1;}
unsigned int MAMCR() {return 0;}
unsigned int MAMTIM() {return 0;}
unsigned int PLLSTAT(int channel) {return 0;}
unsigned int VPBDIV() {return 0;}
unsigned int PREINT() {return 0;}
unsigned int PREFRAC() {return 0;}
unsigned int CCR() {return 0;}



