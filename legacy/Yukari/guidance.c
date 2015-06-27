#include "guidance.h"
#include <math.h>

static const fp clatref=0.765312484; //Cosine of latitude of Sparkfun, 40.0652degN
static const fp mperlat=111035.86e-7;
static const fp mperlon= 85312.55e-7;
static int lat0, lon0, de,dn,lat,lon;

void initGPSGuidance(int Llat0, int Llon0) {
  lat0=Llat0;
  lon0=Llon0;
}

//Position of waypoint from initial point
//de - delta-easting in meters
//dn - delat-northing in meters
void setWaypoint(int Lde, int Ldn) {
  de=Lde;
  dn=Ldn;
}

//Read the current position from the gps in kdeg
void updatePos(int Llat, int Llon) {
  lat=Llat;
  lon=Llon;
}

//Calculate distance and bearing to waypoint
//returns
//  sb - sine of course to waypoint (course is 0 at true north and increases to the east)
//  cb - cosine of course to waypoint
//  distance - distance to waypoint in meters
void getWaypointVector(fp* sb, fp* cb, fp* distance) {
  fp e=((fp)(lon-lon0))*mperlon-de;
  fp n=((fp)(lat-lat0))*mperlon-dn;
  *distance=sqrt(e*e+n*n);
  fp oodist=1.0/ *distance;
  *sb=e*oodist;
  *cb=n*oodist;
}

