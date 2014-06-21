#include "guide.h"

#define dOvershoot 1000*1e-7 //Doesn't really matter, this is about 11 m

//From the vehicle state, figure out the desired heading
void Guide::guide() {
  nav.hasRMC=false;
  ddlatSteerTo=dlatSteerTo-nav.dLat;
  ddlonSteerTo=dlonSteerTo-nav.dLon;
  desiredHdg=atan2(ddlonSteerTo,ddlatSteerTo)*180.0/PI;
  ddlatToGo=config.dlatWaypoint[target]-nav.dLat;
  ddlonToGo=config.dlonWaypoint[target]-nav.dLon;
  coerceHeading(desiredHdg);
  //Dot product of vector from current waypoint to target waypoint, with 
  //               vector from position         to target waypoint. 
  //When this is negative, we have passed the waypoint
  dotp=(ddlatToGo)*(ddlatBasepath)+
       (ddlonToGo)*(ddlonBasepath);
  if(dotp<0) {
    currentBaseWaypoint++;
    if(currentBaseWaypoint>config.nWaypoints) {
      currentBaseWaypoint=0; //This will make it loop the course forever (if it didn't hit the brakes)
      runFinished=true;
    }
    setupNextWaypoint();
    hasNewWaypoint=true;
  }
}

void Guide::setupNextWaypoint() {
  target=currentBaseWaypoint+1;
  ddlatBasepath=config.dlatWaypoint[target]-config.dlatWaypoint[currentBaseWaypoint];
  ddlonBasepath=config.dlonWaypoint[target]-config.dlonWaypoint[currentBaseWaypoint];
  distBasepath=sqrt(ddlatBasepath*ddlatBasepath+ddlonBasepath*ddlonBasepath);
  nlatBasepath=ddlatBasepath/distBasepath;
  nlonBasepath=ddlonBasepath/distBasepath;
  dlatSteerTo=config.dlatWaypoint[currentBaseWaypoint]+nlatBasepath*(distBasepath+dOvershoot);
  dlonSteerTo=config.dlonWaypoint[currentBaseWaypoint]+nlonBasepath*(distBasepath+dOvershoot);
}

void Guide::begin() {
  setupNextWaypoint();
  desiredHdg=atan2(dlonSteerTo,dlatSteerTo)*180.0/PI;
  nav.startHdg=desiredHdg;
}


