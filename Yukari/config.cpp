#include "readconfig.h"
#include "config.h"
#include "ccsds.h"
#include "Serial.h"

Config config;

template<> const char ReadConfig<CCSDS,HardwareSerial<0>,HardSPI0>::configFilename[]="CONFIG.TXT";

template<> const char* const ReadConfig<CCSDS,HardwareSerial<0>,HardSPI0>::tagNames[]={
"GSENS",
"GODR",
"GBW",
"P",
"I",
"D",
"WP",
"THR",
"SCL",
"CCOUNT",
"CSPD",
"MANV",
"TCUT",
"TICK",
nullptr};

template<> const ReadConfigEntry   ReadConfig<CCSDS,HardwareSerial<0>,HardSPI0>::entries[]={
{ReadConfig::typeInt,&config.gyroSens           ,nullptr           }, //GSENS
{ReadConfig::typeInt,&config.gyroODR            ,nullptr           }, //GODR
{ReadConfig::typeInt,&config.gyroBW             ,nullptr           }, //GBW
{ReadConfig::typeFp ,&config.P                  ,nullptr           }, //P
{ReadConfig::typeFp ,&config.I                  ,nullptr           }, //I
{ReadConfig::typeFp ,&config.D                  ,nullptr           }, //D
{ReadConfig::typeV2 , config.waypoint           ,&config.nWaypoints}, //WP
{ReadConfig::typeInt,&config.throttle           ,nullptr           }, //THR
{ReadConfig::typeV3 ,&config.gyroScl            ,nullptr           }, //SCL
{ReadConfig::typeInt,&config.compassCountdownMax,nullptr           }, //CCOUNT
{ReadConfig::typeFp ,&config.compassSpeed       ,nullptr           }, //CSPD
{ReadConfig::typeFp ,&config.maneuverRate       ,nullptr           }, //MANV
{ReadConfig::typeFp ,&config.tCutoff            ,nullptr           }, //TCUT
{ReadConfig::typeFp ,&config.tickSize           ,nullptr           }, //TICK
};



