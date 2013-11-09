#ifndef gps_h
#define gps_h

//GPS status
extern int GPShasPos;
extern int hasRMC;
//Latitude and longitude in 100ndeg (1e7 ndeg in 1 deg)
//Alt in cm
extern int lat;
extern int lon;
extern int alt;  //only works for SiRF for now
#define PTMF(ptm) ((*this).*(ptm))
/*
class NMEAParser;
class State {
  public:
    virtual void act(char)=0;  
}
class Finish {
  public:
    virtual void act(void)=0;  
}
*/
class NMEAParser {
  public:
//    typedef void (NMEAParser::*State)(char);
    typedef void (NMEAParser::*Finish)(void);
  private:
    int zdaHMS, zdaDD, zdaMM, zdaYYYY;
//Given a string representing number with a decimal point, return the number multiplied by 10^(shift)
    int parseNumber(char* in, int& shift);
//Given a string representing a number in the form dddmm.mmmm, return
//an integer representing that number in just degrees multiplied by 10^7
    int parseMin(char* buf);
    void expectDollar(char in);
    void expectG(char in);
    void expectP(char in);
    void ThirdLetter(char in);
    void expectZDA2(char in);
    void expectZDA3(char in);
    void parseZDA(char in);
    void waitChecksum(char in);

    void finishZDA();

    Finish finishPacket;
    int pktState;
    char checksum;
  public:
    bool hasPos;
//Latitude and longitude in 100ndeg (1e7 ndeg in 1 deg)
//Alt in cm
    State nmeaState;
    int lat,lon,alt;
    NMEAParser():nmeaState(&NMEAParser::expectDollar),finishPacket(0),pktState(0),checksum(0) {};
    void parseNMEA(char in) {checksum ^= in;PTMF(nmeaState)(in);}
};

#endif
