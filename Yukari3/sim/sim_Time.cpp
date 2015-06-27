#include "Time.h"
#include "robot.h"
#include <time.h>
unsigned int CCLK=60000000,PCLK=60000000;

//Number of days in each month, with 0 as a placeholder. the previous month.
                    //       J  F  M  A  M  J  J  A  S  O  N  D
const char monthTable[]={ 0,31,28,31,30,31,30,31,31,30,31,30,31};

void set_rtc(int y, int m, int d, int h, int n, int s) {
  RTCYEAR()=y;
  RTCMONTH()=m;
  RTCDOM()=d;
  RTCDOY()=0;
  for(int i=0;i<m;i++)RTCDOY()+=monthTable[m];
  RTCDOY()+=d;
  if((m>2) & ((y-2000)%4==0)) RTCDOY()++;
  if(y>2000) {
    int DOC=RTCDOY()-1+y/4+(y-2001)*365+1; //Day in centry beginning 1 Jan 2001, which was Monday
    RTCDOW()=(DOC % 7); //0-Sunday -- 6-Saturday
  } else RTCDOW()=0;
  RTCHOUR()=h;
  RTCMIN()=n;
  RTCSEC()=s;
}
