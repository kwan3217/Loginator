#ifndef gps_h
#define gps_h

#include "Serial.h"
#include "gpio.h"

class GPS;
class State {
  public:
    int num;
    State(int Lnum):num(Lnum){};
    virtual void act(GPS&,char)=0;  
};
class Finish {
  public:
    virtual void act(GPS&)=0;  
};
class ExpectDollar:public State { public: ExpectDollar():State( 0){};void act(GPS&,char); };
class ExpectG     :public State { public: ExpectG     ():State( 1){};void act(GPS&,char); };
class ExpectP     :public State { public: ExpectP     ():State( 2){};void act(GPS&,char); };
class ThirdLetter :public State { public: ThirdLetter ():State( 3){};void act(GPS&,char); };
class ExpectZDA2  :public State { public: ExpectZDA2  ():State( 4){};void act(GPS&,char); };
class ExpectZDA3  :public State { public: ExpectZDA3  ():State( 5){};void act(GPS&,char); };
class ExpectGGA2  :public State { public: ExpectGGA2  ():State( 6){};void act(GPS&,char); };
class ExpectGGA3  :public State { public: ExpectGGA3  ():State( 7){};void act(GPS&,char); };
class ExpectVTG2  :public State { public: ExpectVTG2  ():State( 8){};void act(GPS&,char); };
class ExpectVTG3  :public State { public: ExpectVTG3  ():State( 9){};void act(GPS&,char); };
class ParseZDA    :public State { public: ParseZDA    ():State(10){};void act(GPS&,char); };
class ParseGGA    :public State { public: ParseGGA    ():State(11){};void act(GPS&,char); };
class ParseVTG    :public State { public: ParseVTG    ():State(12){};void act(GPS&,char); };
class WaitChecksum:public State { public: WaitChecksum():State(13){};void act(GPS&,char); };
class FinishZDA   :public Finish{                                    void act(GPS&     ); };
class FinishGGA   :public Finish{                                    void act(GPS&     ); };
class FinishVTG   :public Finish{                                    void act(GPS&     ); };
class GPS {
  private:
    int zdaHMS, zdaHMSScale, zdaDD, zdaMM, zdaYYYY;
    char numBuf[128]; 
    int numPtr;
    int ggaLat,ggaLon,ggaAlt,ggaAltScale;
//Given a string representing number with a decimal point, return the number multiplied by 10^(shift)
    static int parseNumber(char* in, int& shift);
//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in just degrees multiplied by 10^7
    static int parseMin(char* buf);
    ExpectDollar expectDollar; friend class ExpectDollar;
    ExpectG expectG;           friend class ExpectG;
    ExpectP expectP;           friend class ExpectP;
    ThirdLetter thirdLetter;   friend class ThirdLetter;
    ExpectZDA2 expectZDA2;     friend class ExpectZDA2;
    ExpectZDA3 expectZDA3;     friend class ExpectZDA3;
    ExpectGGA2 expectGGA2;     friend class ExpectGGA2;
    ExpectGGA3 expectGGA3;     friend class ExpectGGA3;
    ExpectVTG2 expectVTG2;     friend class ExpectVTG2;
    ExpectVTG3 expectVTG3;     friend class ExpectVTG3;
    ParseZDA parseZDA;         friend class ParseZDA;
    ParseGGA parseGGA;         friend class ParseGGA;
    ParseVTG parseVTG;         friend class ParseVTG;
    WaitChecksum waitChecksum; friend class WaitChecksum;
    FinishZDA finishZDA;       friend class FinishZDA;
    FinishGGA finishGGA;       friend class FinishGGA;
    FinishVTG finishVTG;       friend class FinishVTG;

    Finish* finishPacket;
    int pktState;
    char checksum;
    Stream& inf;
  public:
    bool hasPos;
//Latitude and longitude in 100ndeg (1e7 ndeg in 1 deg)
//Alt in scaled meters
    State* nmeaState;
    int lat,lon,alt,altScale;
    int PPS,lastPPS;
    uint32_t lastPPSTC,PPSTC;
    bool writePPS;
    GPS(Stream& Linf):finishPacket(0),pktState(0),checksum(0),inf(Linf),nmeaState(&expectDollar),lastPPS(0),writePPS(false) {};
    void begin();
    void handlePPS();
    void process();
    int handleHMS();
    int handlestoi();
    int handleNum(int& shift);
    int handleMin();
    bool parseFieldHMS(char in,int& result);
    bool parseFieldstoi(char in, int& result);
    bool parseFieldNum(char in, int& result, int& shift);
    bool parseFieldMin(char in, int& result);
    void acc(char in);
};

#endif
