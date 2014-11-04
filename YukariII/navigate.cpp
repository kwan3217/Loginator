#include "navigate.h"

void Navigate::handleRMC(uint32_t TC, fp Llat, fp Llon, fp spd, fp Lhdg) {
  hasRMC=true;
  rmcHdg=Lhdg;
  rmcSpd=spd;
  lat=Llat;
  lon=Llon;
  if(firstLat==0) {
	firstLat=lat;
	firstLon=lon;
  }
  if(hasInit) {
    fp ddLat=-dLat;
    fp ddLon=-dLon;
    dLat=lat-firstLat;
    dLon=(lon-firstLon)*clat;
    ddLat=dLat+ddLat;
    ddLon=dLon+ddLon;
    if((compassCountdown<0) && rmcSpd>3) {
      if((ddLat!=0)|(ddLon!=0)) {
        dHdg=atan2(ddLon,ddLat)*180.0/PI;
        coerceHeading(dHdg);
        hdg=dHdg;
        hdgOfs=hdg-gyroHdg;
        coercedHeading(hdgOfs);
      }
    }
  }
}

void Navigate::setSens(uint8_t fs) {
  fp FS=((fp)(250 << fs));
  sens=FS/360.0; //rotations per second full scale
  sens*=2*PI;   //radians per second full scale
  sens/=32768;  //radians per second per DN
  sens*=config.yscl;//Higher yscl means more rad/s per DN, makes it underturn
  sens/=32768; //include calibration scale factor
}

void Navigate::handleGyroCfg(uint8_t ctrl4) {
  uint8_t fs=(ctrl4>>4) & 0x03;
  setSens(fs);
} 

void Navigate::handleGyro(uint32_t TC, int16_t gx, int16_t gy, int16_t gz) {
  //Gyro packet
  gyroT=TC/60e6;
  if(gyroT<lastT) {
    deltaT=(gyroT-lastT)+60;
    minuteofs++;
  } else {
    deltaT=(gyroT-lastT);
  }
  lastT=gyroT;
  gyroT+=(minuteofs*60);
  if (!hasInit) {
    //Collect average G data
    avgGsample[0][head]=gx;
    avgGsample[1][head]=gy;
    avgGsample[2][head]=gz;
    head++;
    if(head>=navgG*2) {
      head=0;
      initFilter();
    }
  } else {
    //propagate the quaternion
    calGx=sens*(((fp)gx)-avgGx);
    calGy=sens*(((fp)gy)-avgGy);
    calGz=sens*(((fp)gz)-avgGz);
    e.integrate(calGx,calGy,calGz,deltaT);
    if((calGy>0.8)||(calGy<-0.8)) {
      compassCountdown=400;
    } else {
      compassCountdown--;
    }
    //Figure the nose vector
    nose_r=e.b2r(nose);
    gyroHdg=atan2(nose_r.z,nose_r.x)*180.0/PI+90;
    coerceHeading(gyroHdg);
    hdg=gyroHdg+hdgOfs;
    coerceHeading(hdg);
  }
  gyroPktCount++;
}

void Navigate::initFilter() {
  firstLat=lat;
  firstLon=lon;
  for(int i=0;i<navgG;i++) {
    int j=head+i-navgG;
    if(j<0) j+=navgG*2;
    avgGx+=avgGsample[0][j];
    avgGy+=avgGsample[1][j];
    avgGz+=avgGsample[2][j];
  }
  avgGx/=navgG;
  avgGy/=navgG;
  avgGz/=navgG;
  hdg=startHdg;
  hdgOfs=startHdg;
  hasInit=true;
}


