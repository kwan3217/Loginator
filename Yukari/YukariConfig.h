#ifndef YukariConfig_h
#define YukariConfig_h
const unsigned char channelThrottle=6;    //PWM6, attached to RX1
const unsigned char channelSteer=4; //PWM4, attached to TX1
const int right=-1;
const int left=-right;
const int forward=1;
const int reverse=-forward;
const unsigned int channelTCPPS=0;
const unsigned int channelTCGyro=3;
const unsigned int channelTCBtn=2;
const unsigned int adEncoder0=6; //Use AD pin 6
const unsigned int adEncoder1=7;
#endif
