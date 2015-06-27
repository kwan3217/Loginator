#ifndef conparse_h
#define conparse_h

//references to configuration parameters
extern int uartMode[2];
extern int baud[2];
extern char trigger[2];
extern char frameEnd[2];
extern int rawSize[2];
extern int writeMode;
extern int adcFreq;
extern int adcBin;
extern int accBW;
extern int gyroPeriod;
extern char channelActive[9];
extern int nChannels;
extern int GPSSyncMode;
extern int GSense;
extern int autoGSense;
extern int powerSave;
extern int dumpFirmware;
extern int firmAddress;
extern int writeFirm;
extern int uartAcc;
extern int uartBFld;
extern int uartGyro;
extern char firmFile[80];
extern char ropeFile[80];
extern int ropeReady;

void f_uartMode1(char* value, int* dummy);
void f_uartBaud1(char* value, int* dummy);
void f_uartTrigger1(char* value, int* dummy);
void f_uartEnd1(char* value, int* dummy);
void f_stoi(char* value, int* p);
void f_writeMode(char* value, int* dummy);
void f_firmFile(char* value, int* dummy);
void f_firmAddr(char* value, int* dummy);
int readLogCon(void);
int writeLogCon(void);
int processLine(char* keyword, char* value);

#define PKT_NONE 0
#define PKT_NMEA 1
#define PKT_BINARY 2
#define PKT_SIRF 3
#define PKT_TEXT 4
#define PKT_BOTH 5

#define GPS_SYNC_LOW 1
#define GPS_SYNC_PPS 2

#endif
