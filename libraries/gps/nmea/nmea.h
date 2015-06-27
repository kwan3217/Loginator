#ifndef nmea_h
#define nmea_h

//We are from now on going to use floating point numbers without embarassment
//  -- No more separate "scale" part for certain variables
//We are keeping the factor of 10^7 in the lat and lon, because single-precision
//FP doesn't guarantee enough resolution. 
#include "float.h"
class NMEA;

typedef void (*State )(NMEA&,char);
typedef void (*Finish)(NMEA&);

void expectDollar(NMEA&,char);
void expectG     (NMEA&,char);
void expectP     (NMEA&,char);
void thirdLetter (NMEA&,char);
void expectZDA2  (NMEA&,char);
void expectZDA3  (NMEA&,char);
void expectGGA2  (NMEA&,char);
void expectGGA3  (NMEA&,char);
void expectVTG2  (NMEA&,char);
void expectVTG3  (NMEA&,char);
void parseZDA    (NMEA&,char);
void parseGGA    (NMEA&,char);
void parseVTG    (NMEA&,char);
void waitChecksum(NMEA&,char);
void expectRMC2  (NMEA&,char);
void expectRMC3  (NMEA&,char);
void parseRMC    (NMEA&,char);
void finishZDA   (NMEA&     );
void finishGGA   (NMEA&     );
void finishVTG   (NMEA&     );
void finishRMC   (NMEA&     );
class NMEA {
private:
    char numBuf[128]; 
    int numPtr;
    fp ggaHMS;
    int ggaLat,ggaLon;
    fp ggaAlt;
    fp rmcHMS;
    int rmcDMY; 
    int rmcLat,rmcLon;
    fp rmcSpd,rmcHdg;
//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in just degrees
    static int parseMin(char* buf);
    friend void expectDollar(NMEA&,char);
    friend void expectG     (NMEA&,char);
    friend void expectP     (NMEA&,char);
    friend void thirdLetter (NMEA&,char);
    friend void expectZDA2  (NMEA&,char);
    friend void expectZDA3  (NMEA&,char);
    friend void expectGGA2  (NMEA&,char);
    friend void expectGGA3  (NMEA&,char);
    friend void expectVTG2  (NMEA&,char);
    friend void expectVTG3  (NMEA&,char);
    friend void parseZDA    (NMEA&,char);
    friend void parseGGA    (NMEA&,char);
    friend void parseVTG    (NMEA&,char);
    friend void waitChecksum(NMEA&,char);
    friend void expectRMC2  (NMEA&,char);
    friend void expectRMC3  (NMEA&,char);
    friend void parseRMC    (NMEA&,char);
    friend void finishZDA   (NMEA&     );
    friend void finishGGA   (NMEA&     );
    friend void finishVTG   (NMEA&     );
    friend void finishRMC   (NMEA&     );

    Finish finishPacket;
    int pktState;
    char checksum;
    void acc(char in);
    fp handleHMS();
    int handlestoi();
    fp handleNum();
    int handleMin();
    bool parseFieldHMS(char in, fp& result);
    bool parseFieldstoi(char in, int& result);
    bool discardFieldstoi(char in);
    bool parseFieldNum(char in, fp& result);
    bool parseFieldMin(char in, int& result);
    fp zdaHMS;
    int zdaDD, zdaMM, zdaYYYY;
    fp vtgCourse,vtgSpeedKt;
  public:
    bool hasPos;
//Alt in meters
    State nmeaState;
    int HMS, DMY;
    int lat; ///<Latitude in cm', 10^7 cm'=1 degree
    int lon; ///<Longitude in cm', not scaled for cos(lat)
    fp alt;  ///<Altitude in meters
    fp hdg;  ///<Heading in degrees east of north
    fp spd;  ///<Speed in knots
    bool writeZDA,writeGGA,writeVTG,writeRMC;
    NMEA():finishPacket(0),pktState(0),checksum(0),
      nmeaState(&expectDollar),writeZDA(false),writeGGA(false),writeVTG(false) {};
    void process(const char in);
};

#endif
