#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <libgen.h>
#include "compass.h"

#define PI 3.1415926535897932
#define clat 0.765243525639  //cosine of latitude of AVC2014 start line
#define sigv2 50             //square of expected acceleration of heading, deg/sec^2
void Compass::handleRMC(uint32_t TC, uint32_t hms, int32_t Llat, int32_t Llon, int32_t spd, int32_t spdScale, int32_t hdg, int32_t hdgScale, int32_t dmy) {
  int h=hms/10000;
  int ms=hms % 10000;
  int m=ms/100;
  int s=ms%100;
  sod=h*3600+m*60+s;
  rmcHdg=hdg;
  for(int i=0;i<hdgScale;i++) rmcHdg/=10.0;
  rmcSpd=spd;
  for(int i=0;i<spdScale;i++) rmcSpd/=10.0;
  lat=((float)Llat)/1e7;
  lon=((float)Llon)/1e7;
  if(hasInit) {
    dLat=lat-firstLat;
    dLon=(lon-firstLon)*clat;
    filterRMC(rmcHdg);
  }
}

void Compass::setSens(uint8_t fs) {
  fp FS=((fp)(250 << fs));
  sens=FS/360.0; //rotations per second full scale
  sens*=2*PI;   //radians per second full scale
  sens/=32768;  //radians per second per DN
  sens*=yscl/32768; //include calibration scale factor
}

void Compass::handleGyroCfg(uint8_t ctrl4) {
  uint8_t fs=(ctrl4>>4) & 0x03;
  setSens(fs);
} 

void Compass::handleGyro(uint32_t TC, int16_t gx, int16_t gy, int16_t gz) {
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
  } else {
    //propagate the quaternion
    calGx=sens*(((fp)gx)-avgGx);
    calGy=sens*(((fp)gy)-avgGy);
    calGz=sens*(((fp)gz)-avgGz);
    e.integrate(calGx,calGy,calGz,deltaT);
    //Figure the nose vector
    nose_r=e.b2r(nose);
    fp lastGyroHdg=gyroHdg;
    gyroHdg=atan2(nose_r.z,nose_r.x)*180.0/PI;
    fp dGyroHdg=gyroHdg-lastGyroHdg;
    if((gyroHdg<-170) & (lastGyroHdg> 170)) dGyroHdg+=360;
    if((gyroHdg> 170) & (lastGyroHdg<-170)) dGyroHdg-=360;
    dGyroHdg/=deltaT;
    filterGyro(dGyroHdg,deltaT);
  }
  gyroPktCount++;
}

void Compass::initFilter() {
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
  hdg=rmcHdg;
  hasInit=true;
}

template<int m,int n,int p>
inline void mm(fp (&a)[m][n], fp (&b)[n][p], fp (&ab)[m][p]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<p;j++) {
      ab[i][j]=0;
      for(int k=0;k<n;k++) {
        ab[i][j]+=a[i][k]*b[k][j];
      }
    }
  }
}

template<int m,int n,int p>
inline void mmt(fp (&a)[m][n], fp (&bt)[p][n], fp (&abt)[m][p]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<p;j++) {
      abt[i][j]=0;
      for(int k=0;k<n;k++) {
        abt[i][j]+=a[i][k]*bt[j][k];
      }
    }
  }
}

//equivalent to a+=b
template<int m,int n>
inline void mpm(fp (&a)[m][n], fp (&b)[m][n]) {
  for(int i=0;i<m;i++) {
    for(int j=0;j<n;j++) {
      a[i][j]+=b[i][j];
    }
  }
}  

void Compass::filterGyro(fp dGyroHdg, fp dt) {
  //This requires propagating the state
  fp A[2][2];
  fp scratch[2][2];
  A[0][0]=1;A[0][1]=dt;
  A[1][0]=0;A[1][1]=1;
  //Time update of estimate
  fp xi[2][1];
  xi[0][0]=A[0][0]*hdg+A[1][0]*hdgRate;
  xi[1][0]=A[0][1]*hdg+A[1][1]*hdgRate;
  //Predicted measurement
  fp H[1][2];
  H[0][0]=0;H[0][1]=1;
  fp zi[1][1];
  zi[0][0]=H[0][0]*hdg+H[0][1]*hdgRate;
  //Process noise
  fp Q[2][2];
  Q[0][0]=dt*dt*dt*dt/4*sigv2;Q[1][0]=dt*dt*dt/2*sigv2;
  Q[0][1]=Q[1][0];            Q[1][1]=dt*dt*sigv2;
  //Time update of covariance
  //P=[A][P_i-1][A]
  fp (&AP)[2][2]=scratch;
  mm(A,P,AP);
  mmt(AP,A,P);
  mpm(P,Q); //P is now [P^-]

  //Calculate Kalman gain
  //S: P(2x2)#H^T(2x1)=S(2x1)
  fp K[2][1]; 
  fp (&S)[2][1]=K;
  mmt(P,H,S);

  //gamma: H(1x2)#S(2x1)=Gamma'(1x1)
  fp Gamma[1][1];
  //Gamma'(1x1)+R(1x1)=Gamma(1x1)
  mm(H,S,Gamma); 
//  Gamma[0][0]+=Rgyro;
  
  //K=S # Gamma^-1, but Gamma is scalar, so K=S/Gamma
  K[0][0]/=Gamma[0][0];
  K[0][1]/=Gamma[0][0];
}

void Compass::filterRMC(fp rmcHdg) {

}
