#ifndef nmea_h
#define nmea_h

class NMEA;
class State {
  public:
    int num;
    State(int Lnum):num(Lnum){};
    virtual void act(NMEA&,char)=0;  
};
class Finish {
  public:
    virtual void act(NMEA&)=0;  
};
class ExpectDollar:public State { public: ExpectDollar():State( 0){};void act(NMEA&,char); };
class ExpectG     :public State { public: ExpectG     ():State( 1){};void act(NMEA&,char); };
class ExpectP     :public State { public: ExpectP     ():State( 2){};void act(NMEA&,char); };
class ThirdLetter :public State { public: ThirdLetter ():State( 3){};void act(NMEA&,char); };
class ExpectZDA2  :public State { public: ExpectZDA2  ():State( 4){};void act(NMEA&,char); };
class ExpectZDA3  :public State { public: ExpectZDA3  ():State( 5){};void act(NMEA&,char); };
class ExpectGGA2  :public State { public: ExpectGGA2  ():State( 6){};void act(NMEA&,char); };
class ExpectGGA3  :public State { public: ExpectGGA3  ():State( 7){};void act(NMEA&,char); };
class ExpectVTG2  :public State { public: ExpectVTG2  ():State( 8){};void act(NMEA&,char); };
class ExpectVTG3  :public State { public: ExpectVTG3  ():State( 9){};void act(NMEA&,char); };
class ParseZDA    :public State { public: ParseZDA    ():State(10){};void act(NMEA&,char); };
class ParseGGA    :public State { public: ParseGGA    ():State(11){};void act(NMEA&,char); };
class ParseVTG    :public State { public: ParseVTG    ():State(12){};void act(NMEA&,char); };
class WaitChecksum:public State { public: WaitChecksum():State(13){};void act(NMEA&,char); };
class ExpectRMC2  :public State { public: ExpectRMC2  ():State(14){};void act(NMEA&,char); };
class ExpectRMC3  :public State { public: ExpectRMC3  ():State(15){};void act(NMEA&,char); };
class ParseRMC    :public State { public: ParseRMC    ():State(16){};void act(NMEA&,char); };
class FinishZDA   :public Finish{                                    void act(NMEA&     ); };
class FinishGGA   :public Finish{                                    void act(NMEA&     ); };
class FinishVTG   :public Finish{                                    void act(NMEA&     ); };
class FinishRMC   :public Finish{                                    void act(NMEA&     ); };
class NMEA {
public:
    char numBuf[128]; 
    int numPtr;
    int ggaHMS, ggaLat,ggaLon,ggaAlt,ggaAltScale;
    int rmcHMS, rmcLat,rmcLon,rmcSpd,rmcSpdScale,rmcHdg,rmcHdgScale,rmcDMY;
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
    ExpectRMC2 expectRMC2;     friend class ExpectRMC2;
    ExpectRMC3 expectRMC3;     friend class ExpectRMC3;
    ParseRMC parseRMC;         friend class ParseRMC;
    FinishZDA finishZDA;       friend class FinishZDA;
    FinishGGA finishGGA;       friend class FinishGGA;
    FinishVTG finishVTG;       friend class FinishVTG;
    FinishRMC finishRMC;       friend class FinishRMC;

    Finish* finishPacket;
    int pktState;
    char checksum;
    void acc(char in);
    int handleHMS();
    int handlestoi();
    int handleNum(int& shift);
    int handleMin();
    bool parseFieldHMS(char in,int& result);
    bool parseFieldstoi(char in, int& result);
    bool discardFieldstoi(char in);
    bool parseFieldNum(char in, int& result, int& shift);
    bool parseFieldMin(char in, int& result);
    int zdaHMS, zdaHMSScale, zdaDD, zdaMM, zdaYYYY;
    int vtgCourse,vtgCourseScale,vtgSpeedKt,vtgSpeedKtScale;
  public:
    bool hasPos;
//Latitude and longitude in 100ndeg (1e7 ndeg in 1 deg)
//Alt in scaled meters
    State* nmeaState;
    int HMS, DMY,lat,lon,alt,altScale,hdg,hdgScale,spd,spdScale;
    bool writeZDA,writeGGA,writeVTG,writeRMC;
    NMEA():finishPacket(0),pktState(0),checksum(0),
      nmeaState(&expectDollar),writeZDA(false),writeGGA(false),writeVTG(false) {};
    void process(const char in);
};

#endif
