#include "sim.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "navigate.h" //for clat
#include "LPC214x.h"

RobotState* state;

int main() {
  SimState simState(40.0905805064,-105.185665511,311.264);
  state=&simState;
  setup(); //Run robot setup code
  while(simState.t()<300) {
    simState.propagate(1);
    loop();
  }
}

SimState::SimState(double Llat0,double Llon0,double Lhdg):
  lat0(int(Llat0*1e7)),lon0(int(Llon0*1e7)),
  x(0),y(0),hdg(Lhdg*PI/180.0),spd(0),
  shadowTTC(ttc),shadowX(x),shadowY(y),shadowHdg(hdg),shadowSpd(spd),
  steer(0),cmdSpd(0),cmdSteer(0)
{
  stateLog=fopen("simState.csv","w");
  nmeaLog=fopen("simState.nmea","w");
  struct timespec tp;
//  clock_gettime(CLOCK_REALTIME,&tp);
//  rtc0=double(tp.tv_sec)+double(tp.tv_nsec)/1e9;
  rtc0=1434823200.0;//Exactly 06/20/2015 at 18:00:00UTC (noon MDT)
}

static double chaseValue(double value, double cmdValue, double maxValue, double valueTime, double dt) {
  if(value==cmdValue) return value;
  //Time to crank all the way from the current value to the new value. 
  //Term before * is the fraction of the max value we have yet to travel
  double tFullValue=fabs(value-cmdValue)/maxValue*valueTime;
  if(tFullValue<dt) {
    value=cmdValue;
  } else {
    double valueRate=maxValue/valueTime; 
    //Figure the rate in units/s
    value=value+(cmdValue>value?1:-1)*dt*valueRate;
  }
  return value;
}

/**Force heading to be between 0 and almost 360deg, used for absolute heading
  @param hdg heading to coerce, given in degrees
*/
static inline void coerceHeadingRad(double& hdg) {
  if(hdg>2*PI) hdg-=2*PI;
  if(hdg<0)    hdg+=2*PI;
}

/**Force delta-heading to be between almost -180 and 180 deg, used
   for heading differences (such as difference between desired and present heading)
  @param dhdg delta-heading to coerce, given in degrees
*/
inline void coercedHeadingRad(double& dhdg) {
  if(dhdg>=PI) dhdg-=2*PI;
  if(dhdg<-PI) dhdg+=2*PI;
}

static inline void addChecksum(char* gpsBuf) {
  unsigned char sum=0;
  int i=1;
  while(gpsBuf[i]!='*') {
    sum^=gpsBuf[i];
    i++;
  }
  i++; //skip the *
  char checksumBuf[3];
  sprintf(checksumBuf,"%02X",sum);
  gpsBuf[i]=checksumBuf[0];i++;
  gpsBuf[i]=checksumBuf[1];
}

void SimState::propagate(int ms) {
  //Timing
  int dttc=ms*60000;
  double dt=double(dttc)/60e6;
  double rtc_a=ttc/60e6+rtc0; 
  ttc+=dttc;
  double t=ttc/60e6;
  double rtc=t+rtc0;
  fprintf(stateLog,"%lu,%u,%0.6f,%d,%0.6f",ttc,TTC(0),t,dttc,dt);
  //Update speed and steering
  spd=chaseValue(spd,cmdSpd,maxSpd,spdTime,dt);
  fprintf(stateLog,",%0.6f,%0.6f",cmdSpd,spd);
  steer=chaseValue(steer,cmdSteer,maxSteer,steerTime,dt);
  fprintf(stateLog,",%0.6f,%0.6f",cmdSteer,steer);

  //Update position
  double dx,dy;
  //Figure turning radius
  //Both the front and back wheels are tangent to circles centered
  //on the center of curvature. Therefore, a line perpendicular to both
  //is at the center of curvature. In body coordinates, the center
  //of curvature is directly to the side of the back wheel, and therefore
  // on the line y=0 (y=mx+b,m=0,b=0) is on the turning circle. Also, 
  //the center of curvature is on the line perpendicular to the steering wheels.
  //This is on the line y=mx+b, b=wheelbase, m=0 if straight ahead, -inf if
  //turned 90deg to the right, and +inf if 90deg to left, therefore m=-tan(steer)
  //since steer is positive to the right.
  //So, solve 0=-tan(steer)*x+b
  //-b=-tan(steer)*x
  //b=tan(steer)*x
  //b/tan(steer)=x=r, turning radius from back wheels.
  double r=wheelBase/tan(steer);
  //Figure yaw rate
  //The yaw rate is just the speed divided by the turning radius
  yRate=spd/r;
  //Update heading
  hdg+=yRate*dt;
  coerceHeadingRad(hdg);
  fprintf(stateLog,",%0.6f,%0.6f",r,yRate);
  //We take a shortcut here and presume that since the time step is small, the 
  //heading change is also small and we can use the final heading as an 
  //approximate value for the whole time step.
  dx=sin(hdg)*dt*spd;
  dy=cos(hdg)*dt*spd;
  x+=dx;
  y+=dy;
  fprintf(stateLog,",%0.6f,%0.6f,%0.6f,%0.6f,%0.6f",hdg*180.0/PI,dx,dy,x,y);
  double lat=(lat0+y)/1e7;
  double lon=(lon0+x*clat)/1e7;
  fprintf(stateLog,",%0.6f,%0.6f",lat,lon);
 

  //If the RTC second has rolled over, record GPS fix
  if(int(rtc)!=int(rtc_a)) {
    //Handle second rollover
    incRTC();
    //Record GPS fix at this time. This is the point to add error if you want
    shadowX=x;
    shadowY=y;
    shadowSpd=spd;
    shadowHdg=hdg;
    hasGGA=false;
    hasRMC=false;
  }

  double printLat=int(fabs(lat))*100+60.0*(fabs(lat)-int(fabs(lat)));
  double printLon=int(fabs(lon))*100+60.0*(fabs(lon)-int(fabs(lon)));
  double printSpd=shadowSpd*cmsToKnot;
  double printHdg=shadowHdg*180.0/PI;
  //If necessary, make GGA sentence
  double frac=rtc-int(rtc);
  if(!hasGGA && frac>ggaTime) {
    hasGGA=true;
    time_t rtctt=(time_t)rtc;
    struct tm *ptm=gmtime(&rtctt);
    
    sprintf(gpsBuf,"$GPGGA,%02d%02d%02d,%09.4f,%c,%010.4f,%c,1,08,1.0,1600.0,M,0.0,M,,*FF\r\n",
            ptm->tm_hour,ptm->tm_min,ptm->tm_sec,
	    printLat,lat>0?'N':'S',
	    printLon,lon>0?'E':'W');
    int s=strlen(gpsBuf);
    unsigned char sum=0;
    for(int i=1;i<s-5;i++) {
      sum^=gpsBuf[i];
    }
    addChecksum(gpsBuf);
    fprintf(nmeaLog,"%s",gpsBuf);
    gpsTransPointer=0;
  }

  //If necessary, make RMC sentence
  if(!hasRMC && frac>rmcTime) {
    time_t rtctt=(time_t)rtc;
    struct tm *ptm=gmtime(&rtctt);
    hasRMC=true;
    
    sprintf(gpsBuf,"$GPRMC,%02d%02d%02d,A,%09.4f,%c,%010.4f,%c,%.2f,%.2f,%02d%02d%02d,000.0,E*FF\r\n",
            ptm->tm_hour,ptm->tm_min,ptm->tm_sec,
	    printLat,lat>0?'N':'S',
	    printLon,lon>0?'E':'W',
	    printSpd,
	    printHdg,
	    ptm->tm_mday,ptm->tm_mon+1,(ptm->tm_year+1900)%100);
    int s=strlen(gpsBuf);
    unsigned char sum=0;
    for(int i=1;i<s-5;i++) {
      sum^=gpsBuf[i];
    }
    addChecksum(gpsBuf);
    fprintf(nmeaLog,"%s",gpsBuf);
    gpsTransPointer=0;
  }
  
  //Trigger a gyro reading if necessary.
  const int gyroRate=10*60000; //10ms readout cadence
  if(ttc>=gyroLastTTC+gyroRate) {
    gyroLastTTC+=gyroRate;
    TCR[0][3]=TTC(0);
  }
  
  fprintf(stateLog,"\n");
}
                                     //JanFebMarAprMayJunJulAugSepOctNovDec
static const int monthTable[]={0x7FFFFFFF,31,28,31,30,31,30,31,31,30,31,30,31};

void SimState::incRTC() {
  rtcSec++;
  while(rtcSec>=60) {
    rtcSec-=60;
    rtcMin++;
    while(rtcMin>=60) {
      rtcMin-=60;
      rtcHour++;
      while(rtcHour>=24) {
        rtcHour-=24;
        rtcDom++;
        while(rtcDom>monthTable[rtcMonth]+((rtcYear%4==0 && rtcMonth==2)?1:0)) {
          rtcDom-=monthTable[rtcMonth]+((rtcYear%4==0 && rtcMonth==2)?1:0);
          rtcMonth++;
          while(rtcMonth>12) {
            rtcMonth-=12;
            rtcYear++;
          }
        }
      }
    }
  }
}


