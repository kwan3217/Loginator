#ifndef gps_h
#define gps_h

#include "circular.h"
#include "float.h"

//GPS status
extern int GPSLight;
extern int GPShasPos;
extern int hasRMC;
extern int hasNewGPS;
//Latitude and longitude in 100ndeg (1e7 ndeg in 1 deg)
//Alt in cm
extern int lat;
extern int lon;
extern int alt;  //only works for SiRF for now
extern int GPSspd,GPScourse;
extern fp scourse,ccourse;

void parseNmea(circular* sirfBuf);
void parseSirf(circular* sirfBuf);
void displayCoords(void);

//The following functions were static
int countBits(int b);
int parseCommaPart(circular* buf, int* p0, char* out);
int skipCommaPart(circular* buf, int* p0);
char parseChar(circular* buf, int* p0);
int parseNumber(char* in, int* shift);
int parseMin(char* buf);
void parseGPRMC(circular* buf, int p);

#endif
