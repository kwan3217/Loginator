#include "guide.h"

const fp dOvershoot=1000; //Doesn't really matter, this is about 11 m

//From the vehicle state, figure out the desired heading
void Guide::guide() {
  nav.hasRMC=false;
  dr_rt=r_t-nav.r_r;
  targetBearing=atan2(dr_rt[0],dr_rt[1])*180.0/PI;
  coerceHeading(targetBearing);
  dr_rw1=config.waypoint[i_next]-nav.r_r;
  //Dot product of vector from current waypoint to target waypoint, with 
  //               vector from position         to target waypoint. 
  //When this is negative, we have passed the waypoint
  if(nav.gyroT>nav.predictT) {
    i_base++;
    if(i_base>config.nWaypoints) {
      i_base=0; //This will make it loop the course forever (if it didn't hit the brakes)
      runFinished=true;
    }
    setupNextWaypoint();
    hasNewWaypoint=true;
  }
}

void Guide::setupNextWaypoint() {
  i_next=i_base+1;
  dr_w0w1=config.waypoint[i_next]-config.waypoint[i_base];
  fp distBasepath2=dot(dr_w0w1,dr_w0w1);
  fp rdistBasepath=Q_rsqrt(distBasepath2);
  distBasepath=rdistBasepath*distBasepath2; //Convoluted, but it works and only requires one *
  n_w0w1=dr_w0w1*rdistBasepath;
  r_t=config.waypoint[i_base]+n_w0w1*(distBasepath+dOvershoot);
}

void Guide::begin() {
  setupNextWaypoint();
  targetBearing=atan2(r_t[0],r_t[1])*180.0/PI;
  coerceHeading(targetBearing);
  nav.startHdg=targetBearing;
}


