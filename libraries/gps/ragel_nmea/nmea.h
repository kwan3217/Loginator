#ifndef nmea_h
#define nmea_h

//We are from now on going to use floating point numbers without embarassment
//  -- No more separate "scale" part for certain variables
//We are keeping the factor of 10^7 in the lat and lon, because single-precision
//FP doesn't guarantee enough resolution.
#include "float.h"
class NMEA {
private:
    int shadowDeg;
    int buildInt;
    fp buildFrac;
    fp buildFloat;
    fp fracDigits;
    int shadowHH;
    int shadowNN;
    fp  shadowSS;
    int shadowLat;
    int shadowLon;
    fp shadowAlt;
    int shadowDD;
    int shadowMM;
    int shadowYYYY;
    fp shadowSpd;
    fp shadowHdg;
    static int parseMin(char* buf);
    bool countChecksum=false;
    int cs; //Used for the parser to hold its state
    char checksum;
    void finishZDA();
    void finishGGA();
    void finishVTG();
    void finishRMC();
  public:
    static const int M10=10000000; //1e7 as an integer
    bool hasPos;
//Alt in meters
    int HH;
    int NN;
    fp SS;
    int DD;
    int MM;
    int YYYY;
    int lat; ///<Latitude in cm', 10^7 cm'=1 degree
    int lon; ///<Longitude in cm', not scaled for cos(lat)
    fp alt;  ///<Altitude in meters
    fp hdg;  ///<Heading in degrees east of north
    fp spd;  ///<Speed in knots
    bool writeZDA,writeGGA,writeVTG,writeRMC;
    NMEA();
    void process(const char in);
};

#endif
