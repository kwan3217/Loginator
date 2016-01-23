#ifndef navigate_h
#define navigate_h

#include <stdint.h>
#include "config.h"
#include "Quaternion.h"
static constexpr fp clat=/*cos(fp(40.071338*PI/180.0));//*/0.765243525639;  //cosine of latitude of AVC2014 start line
static constexpr fp cms=6378137.0*PI/180.0/1e7; ///<Length of a cm' in meters
static constexpr fp cmsToKnot=cms*3600/1852; ///< Multiply cm'/s by this number to get knots
static constexpr fp wheelBase=31; ///< distance between back and front wheels in cm'
/**Force heading to be between 0 and almost 360deg, used for absolute heading
  @param hdg heading to coerce, given in degrees
*/
inline void coerceHeading(fp& hdg) {
  if(hdg>360) hdg-=360;
  if(hdg<0)   hdg+=360;
}

/**Force delta-heading to be between almost -180 and 180 deg, used
   for heading differences (such as difference between desired and present heading)
  @param dhdg delta-heading to coerce, given in degrees
*/
inline void coercedHeading(fp& dhdg) {
  if(dhdg>=180) dhdg-=360;
  if(dhdg<-180) dhdg+=360;
}

/**Handles sensor input and manages vehicle state. If we had a sophisticated 
Kalman filter, it would live here. We have an ad-hoc heading 
estimator instead. */

class Navigate {
public:
  Config& config;
  Quaternion e;
  Quaternion nose_mz,nose_py,nose_mzr,nose_pyr;
  Vector<3> sens; ///<Gyro sensitivity in rad/s/DN
  fp hdg,gyroHdg,rmcHdg,dHdg; //Heading in degrees, filtered and straight from the gyro
  fp P[2][2];         //Heading estimate covariance
  fp rmcSpd; //Speed in knots
  fp gyroT; //Time of current compass calc, in seconds, does not wrap
  fp rmcT;  //Time of completion of reception and parsing of current RMC sentence, in seconds
  fp ppsT;  //Time of reception and parsing of PPS pulse, in seconds
  fp encT;  //Time of current encoder edge, in seconds
  fp lastT; //Time of last compass calc, in seconds
  fp deltaT;  //Time difference between compass calcs, in seconds
  int32_t sod,minuteofs;
  uint32_t gyroPktCount;
  uint32_t lastTC;
  Vector<3> avgG; ///< Average gyro reading to be subtracted from X gyro reading, DN. This is calculated during the quiet period before the button press.
  Vector<3> calG; ///< Calibrated gyro reading in rad/s, taking into account average-g subtraction and sensitivity
  int compassCountdown=0; //If compassCountdown<0, use RMC as compass reference
                          //Set compassCountdown to 400 if |calGy|>0.5, otherwise decrement
  //Lon is always uncorrected for cos(lat)
  //Easting is always corrected for it
  int lat; ///< Latitude in cm'
  int lon; ///< Longitude in cm'
  int firstLat; ///< Initial latitude in cm'
  int firstLon; ///< Initial longitude in cm', uncorrected for cos(lat)
  Vector<2> r_r; ///<Position of robot in cm' from initial point
  fp hdgOfs; //Difference between gyro heading and RMC heading, hdgOfs=gyroHdg-Hdg, so Hdg=gyroHdg+hdgOfs
  fp startHdg;
  bool hasButton,hasRMC;
  fp tButton;
//  static const int yscl=37564;
  //static const int yscl=32768;
  static const int navgG=50;
  Vector<3> avgGsample[navgG];
  int head,loopAvgG;
  Navigate(Config& Lconfig):config(Lconfig),nose_mz(0,0,-1,0),nose_py(0,1,0,0),e0_min(1023),e0_max(0),e1_min(1023),e1_max(0),predictT(9999) {};
  void handleRMC(uint32_t TC, int lat, int lon, fp spd, fp hdg);
  void handleGyroCfg(uint8_t ctrl4);
  void setSens(uint8_t fs);
  void handleGyro(uint32_t TC, Vector<3>& meas);
  void buttonPress();
  uint16_t e0_min, e0_max,e1_min,e1_max;
  bool e0_high,e1_high;
  bool encoderActive;
  bool handleEncoder(uint32_t TC, uint16_t e0, uint16_t e1);
  void handlePPS(uint32_t TC);
  void handleEncoderEdge(int ticks);
  static const int nHistory=5;
  int historyPointer;
  int historyLoops;
  Vector<2> history[nHistory];
  fp historyT[nHistory];
  fp predictT;
  void predict(fp T);
  void resetHistory();
  bool isPredictValid(){return historyLoops>0;};
  Vector<2> predict_m;
  Vector<2> predict_b;
};

#endif
