#ifndef guidance_h
#define guidance_h

#include "float.h"

void initGPSGuidance(int Llat0, int Llon0);
void setWaypoint(int Llatw, int Llonw);
void updatePos(int Llat, int Llon);
void getWaypointVector(fp* sb, fp* cb, fp* distance);

#endif