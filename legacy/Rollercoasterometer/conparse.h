#ifndef conparse_h
#define conparse_h

//references to configuration parameters
extern int uartMode[2];
extern int baud[2];
extern char trigger[2];
extern char frameEnd[2];
extern int rawSize[2];
extern char timestamp[2];
extern int writeMode;
extern int adcFreq;
extern int adcBin;
extern char channelActive[9];
extern int nChannels;
extern int GPSSyncMode;
extern int GSense;
extern int autoGSense;
extern int powerSave;
extern int dumpFirmware;
extern int firmAddress;
extern int writeFirm;
extern char firmFile[80];

int readLogCon(void);
int writeLogCon(void);

#define PKT_NONE 0
#define PKT_NMEA 1
#define PKT_BINARY 2
#define PKT_SIRF 3
#define PKT_TEXT 4
#define PKT_BOTH 5

#define GPS_SYNC_LOW 1
#define GPS_SYNC_PPS 2

#endif
