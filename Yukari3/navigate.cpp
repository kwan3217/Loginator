#include "navigate.h"

static const fp tickSize=31; //Distance traveled by robot in one encoder tick, cm'

void Navigate::handleRMC(uint32_t TC, int Llat, int Llon, fp spd, fp Lhdg) {
  rmcT=TC/60e6;
  if(TC<lastTC) rmcT+=60; //Do this instead of updating minuteofs. Don't update lastTC
  rmcT+=minuteofs*60;
  hasRMC=true;
  rmcHdg=Lhdg;
  rmcSpd=spd;
  lat=Llat;
  lon=Llon;
  if(!hasButton) {
    firstLat=lat;
    firstLon=lon;
  } else {
    Vector<2> dr_r=r_r; //Keep track of last fix
    r_r[0]=(lon-firstLon)*clat;
    r_r[1]=lat-firstLat;
    predict(ppsT);
    dr_r=r_r-dr_r; //Relative position from last fix to current fix
    if((compassCountdown<=0) && rmcSpd>3) {
      if((dr_r[0]!=0)|(dr_r[1]!=0)) {
        dHdg=atan2(dr_r[0],dr_r[1])*180.0/PI;
        coerceHeading(dHdg);
        hdg=dHdg;
        hdgOfs=hdg-gyroHdg;
        coercedHeading(hdgOfs);
      }
    }
  }
}

void Navigate::predict(fp t) {
  if(isPredictValid()) {
    //See NRC section 15.2
    //r=m*t+b
    //  m=estimated velocity (b in NRC)
    //  b=estimated initial position at t=0 (a in NRC)
    fp S=1.0/nHistory;
    Vector<2> Sx;
    fp Sy=0;
    Vector<2> Sxx;
    Vector<2> Sxy;
    for(int i=0;i<nHistory;i++) {
      Sx+=history[i];
      Sy+=historyT[i];
      Sxx+=history[i]*history[i];
      Sxy+=history[i]*historyT[i];
    }
    Vector<2> Delta=S*Sxx-(Sx*Sx);
    predict_m=(S*Sxy-Sx*Sy)/Delta;
    predict_b=(Sxx*Sy-Sx*Sxy)/Delta;
  }
}

void Navigate::handlePPS(uint32_t TC) {
  ppsT=TC/60e6;
  if(TC<lastTC) ppsT+=60; //Do this instead of updating minuteofs. Don't update lastTC
  ppsT+=minuteofs*60;
}

bool Navigate::handleEncoder(uint32_t TC, uint16_t e0, uint16_t e1) {
  encT=TC/60e6;
  if(TC<lastTC) encT+=60; //Do this instead of updating minuteofs. Don't update lastTC
  encT+=minuteofs*60;
  if(e0>e0_max) e0_max=e0;
  if(e0<e0_min) e0_min=e0;
  if(e1>e1_max) e1_max=e1;
  if(e1<e1_min) e1_min=e1;
  encoderActive=hasButton&&((gyroT-tButton)>1);
  if(!encoderActive) return false;
  bool e0_high_old=e0_high;
  bool e1_high_old=e1_high;
  if(e0_high) {
    //Put the threshold closer to min than max
    uint16_t e0_thresh=e0_min*3/4+e0_max*1/4;
    e0_high=(e0>e0_thresh);
  } else {
    //Put the threshold closer to max than min
    uint16_t e0_thresh=e0_min*1/4+e0_max*3/4;
    e0_high=(e0>e0_thresh);
  }
  if(e1_high) {
    uint16_t e1_thresh=e1_min*3/4+e1_max*1/4;
    e1_high=(e1>e1_thresh);
  } else {
    uint16_t e1_thresh=e1_min*1/4+e1_max*3/4;
    e1_high=(e1>e1_thresh);
  }
  if(e0_high) {
    if(e1_high_old==e1_high) {
      //no edge
    } else {
      handleEncoderEdge(e1_high?1:-1);
      return true;
    }
  }
  return false;
}

void Navigate::handleEncoderEdge(int ticks) {
  r_r[0]+=ticks*config.tickSize*sin(hdg*PI/180.0);
  r_r[1]+=ticks*config.tickSize*cos(hdg*PI/180.0);
  predict(encT);
}

/** Set the gyro sensitivity.
@param fs - Gyro sensitivty setting. Gyro full-scale readings are 250deg/s*2^fs
*/
void Navigate::setSens(uint8_t fs) {
  fp FS=((fp)(250 << fs));
  fp ssens=FS/360.0; //rotations per second full scale
  ssens*=2*PI;   //radians per second full scale
  ssens/=32768;  //radians per second per DN
  sens=ssens*config.gyroScl; //Higher yscl means more rad/s per DN, makes it underturn
}

/** Set the gyro sensitivity.
@param ctrl4 - Gyro register 4 value. Sensitivity is in bits 4 and 5.
*/
void Navigate::handleGyroCfg(uint8_t ctrl4) {
  uint8_t fs=(ctrl4>>4) & 0x03;
  setSens(fs);
}

void Navigate::buttonPress() {
  hasButton=true;
  hdg=startHdg;
  hdgOfs=startHdg;
  compassCountdown=config.compassCountdownMax;
  tButton=gyroT;
}

void Navigate::handleGyro(uint32_t TC, Vector<3>& meas) {
  //Gyro packet
  gyroT=TC/60e6;
  if(TC<lastTC) minuteofs++; //Only update this here
  lastTC=TC; //Only update this here
  gyroT+=(minuteofs*60);
  deltaT=(gyroT-lastT)+60;
  lastT=gyroT;
  if (!hasButton) {
    if(loopAvgG==0) {
      //If we haven't been around the loop yet, just accumulate data
    } else if(loopAvgG==1) {
      //We have only been around once, Denominator is the number of points
      //we have overwritten once. This produces an average with a perfect
      //memory of the points we have seen and no bias due to points we haven't seen.
      avgG=(avgG*fp(head)+avgGsample[head])/fp(head+1);
    } else if(loopAvgG>1) {
      //We have been around more than once. Weight things such that point memory
      //decays exponentially with a time constant of navgG sample times.
      avgG=(avgG*fp(navgG-1)+avgGsample[head])/fp(navgG);
    }
    avgGsample[head]=meas;
    head++;
    if(head>=navgG) {
      head=0;
      loopAvgG++;
    }
  } else {
    //Calibrate the gyro readings by first subtracting off the average G then
    //multiplying by sensitivity.
    calG=sens*(meas-avgG);
    //propagate the quaternion
    e.integrate(calG,deltaT);
    if((calG[1]>config.maneuverRate)||(calG[1]<-config.maneuverRate)) {
      compassCountdown=config.compassCountdownMax;
      resetHistory();
    } else {
      if(compassCountdown>0) compassCountdown--;
    }
    //Figure the nose vector
    nose_mzr=e.b2r(nose_mz);
    nose_pyr=e.b2r(nose_py);
    gyroHdg=atan2(nose_mzr.z,nose_mzr.x)*180.0/PI+90;
    coerceHeading(gyroHdg);
    hdg=gyroHdg+hdgOfs;
    coerceHeading(hdg);
  }
  gyroPktCount++;
}

void Navigate::resetHistory() {
  historyPointer=0;
  historyLoops=0;
}

