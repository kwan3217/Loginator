#ifndef GNC_h
#define GNC_h

#include <stdint.h>
#include "float.h"
#include "navigate.h"
#include "config.h"

/** Code and state to do vehicle guidance

Guidance concepts:
 - __Guidance__ - That part of the program concerned with figuring out which direction
   the robot *should be* travelling. This is not to be confused with
   navigation, which calculates where the robot is and which direction
   it *is* travelling, and control, which is concerned with steering
   to make the guidance desired heading a reality.
 - __cm'__ - One part in 10^7 of a degree of latitude. This happens to be just over
   one SI centimeter, so we call it a modified centimeter. We chose this
   fraction because it maps +-180 to +-1,800,000,000, which just fits
   within a 32-bit integer. The next order of magnitude would not.
 - __Initial...__ - Position, velocity, heading, etc at the time that the robot is
   reset. The GPS runs through the controller reset, so it will
   maintain its position fix, in particular its heading, so when
   setting the robot on the starting line, approach the starting line
   in the direction the robot will be pointed when started.
 - __Northing__, __easting__ - Imagine a plane tangent to the Earth ellipsoid with its
   origin at the initial position of the robot. Northing is
   the distance north of the origin (Y coordinate) and
   easting is the distance east of the origin (X coordinate).
 - __heading__ - angle between direction of current robot motion and true North, in
   degrees east of North (standard navigation convention, used by GPS).
 - __bearing__ - angle at robot between true north and some point, in degrees east
   of North.
 - __lat...__, __lon...__ - absolute latitude and longitude in cm' (longitude is not corrected for cos(lat))
 - __x...__, __y...__ - position relative to the origin, in cm' (easting is corrected for cos(lat))
 - dx..., dy... - position relative to some other position, in cm'
 - nx..., ny... - normalized bearing of some point relative to some other point.
   nx=sin(bearing), ny=cos(bearing), and together they make a unit vector
 - Waypoint - positions which are on the race course. The robot will try to drive
   a course directly between each waypoint in order. Waypoint 0 is
   the start/finish line, and must be included in the waypoint list.
 - Base waypoint (w0) - the waypoint we have just passed
 - Next waypoint (w1) - The waypoint we are headed for
 - Basepath - A ray from the base waypoint through the next waypoint, in analogy
   to a baseball basepath from one base to the next. When treated as a
   vector, this is from the base waypoint to the next.
 - Target - A point along the basepath a certain constant distance past the next
   waypoint. The vehicle steers towards this point. I have nightmares
   about my robot circling the next waypoint because it is inside the
   turning circle, and the robot can never reach it.
 - Passing a waypoint - Draw a line perpendicular to the basepath through the next
   waypoint. If the robot is on the opposite side of this line
   from the base waypoint, we have passed the waypoint and need
   to update waypoints (and stop the robot if the next waypoint
   was the last waypoint).
 - Dot product - Given two vectors a_v and b_v, the dot product is ax*bx+ay*by=|a||b|cos(theta)
   where theta is the angle between the vectors, pretending they have the same tail.
   This value is always positive if the angle is acute, since the vector lengths
   are always positive and cos(acute)>0 and is always negative if this angle is
   obtuse since cos(obtuse)<0. If the dot product of the vector from the base to the next
   waypoint, with the vector from the robot to the next waypoint, is negative,
   then the robot has passed the waypoint.
*/
class Guide {
public:
  Navigate& nav; ///<Source of current position and heading
  Config& config;
  int i_base; ///< Index of the base waypoint
  int i_next; ///< Index of the next waypoint
  Vector<2> r_t;   ///<Position of target.
  Vector<2> dr_w0w1; ///<Vector from base waypoint to target.
  Vector<2> dr_rt; ///<Vector from robot to target.
  Vector<2> dr_rw1; ///<Vector from robot to next waypoint.
  fp targetBearing; ///<Bearing of target. We should make the robot heading match this in order to travel directly to the target.
  fp distBasepath; ///<Distance along basepath from current waypoint to next, cm'
  Vector<2> n_w0w1;  ///<Normalized vector along basepath
  bool runFinished; ///<Set to true by guidance code when we have passed the last waypoint
  bool hasNewWaypoint; ///<Set to true by guidance code when we have a new waypoint - must be cleared externally when this condition is dealt with
  void begin(); ///<Initialize guidance by setting the start line as the base waypoint and waypoint 1 as the next, then calculating the basepath
  void guide(); ///<Calculate the target bearing, and switch waypoints if we have passed the next waypoint
  void setupNextWaypoint(); ///<Calculate the basepath
  Guide(Navigate& Lnav, Config& Lconfig):nav(Lnav),config(Lconfig) {};
  void buttonPress() {};
};

#endif
