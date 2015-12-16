#include "playback.h"
//#include <time.h>
#include <string.h>
#include <stdio.h>
#include "navigate.h" //for clat
#include "sim.h"
#include "SimSd.h"
#include "Time.h"
#include "irq.h"

//I think that we can disallow LPC214x.h access here. All peripheral stuff
//should be through the SimPeripheral object and its children.
class SimUartYukari: public SimUart{
private:
  RobotState& state;
public:
  SimUartYukari(RobotState& Lstate):state(Lstate) {};
  // receive data ready (bit 0) is 1 whenever there is GPS data
  // Transmitter holding register empty (bit 5) is always 1 since we can always send a byte
  virtual uint32_t read_ULSR(int Lport) override {return ((state.hasGPS()?1:0) << 0) | 
                                                ((                 1) << 5);};
  virtual uint32_t read_URBR(int Lport) override {
    char result=state.gpsBuf[state.gpsTransPointer];
    state.gpsTransPointer++;
    return result;
  }
};

class SimTimeYukari: public SimTime{
  RobotState& state;
public:
  SimTimeYukari(RobotState& Lstate):state(Lstate) {};
  virtual uint32_t read_TTC(int Lport) override {return state.ttc%(uint32_t(60)*1000000*60);};
  virtual void write_TTC(int Lport, uint32_t value) override {printf("Do you really need to write to TTC?");}
};

PlaybackState playbackState;
extern SimSd simsd;
SimUartYukari uart(playbackState);
SimGpio gpio;
SimI2c i2c;
SimSd simsd(gpio,4);
SimSsp ssp;
SimTimeYukari time(playbackState);
SimRtc rtc;
SimPwm pwm;
SimAdc adc;
SimPeripherals peripherals(gpio,uart,i2c,simsd,ssp,time,rtc,pwm,adc);

#include "ccsds.h"
#include "extract_str.INC"
int16_t ntohs(int16_t in) {
  return ((in&0xFF00)>>8) | ((in&0x00FF)<<8);
}

int32_t ntohl(int32_t in) {
  return ((in&0xFF000000)>>24) |
         ((in&0x00FF0000)>> 8) |
         ((in&0x0000FF00)<< 8) |
         ((in&0x000000FF)<<24);
}

float ntohf(float in) {
  int iin=*(int*)(&in);
  iin=ntohl(iin);
  return *(float*)(&iin);
}

void reset_handler() {
  setup_pll(0,5); //Set up CCLK PLL to 5x crystal rate
  // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
  //MAMTIM=0x3; //VCOM?
  MAMCR()=0x2;
  MAMTIM()=0x4; //Original
  setup_clock();

  IRQHandler::begin(); //Can't call before ctors are run
}

/* Simulate a delay by advancing the playbackState clock the right number of ticks */
void delay(unsigned int ms) {
  playbackState.ttc+=ms*60000;
  fprintf(stderr,"Delay %d ms, ttc now equals %lu\n",ms,playbackState.ttc);
}

int main(int argc, char** argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  playbackState.begin(argv[1],0);
  simsd.open("sim/sdcard");
  gpio.addListener(simsd,{7});
  reset_handler(); //Do the stuff that the embedded reset_handler would do

  setup(); //Run robot setup code
  while(!playbackState.done()) {
    playbackState.propagate(1);
    loop();
  }
}

char buf[65536+7];
void PlaybackState::begin(char* infn, int fs) {
  inf=fopen(infn,"rb");
  fread(buf,1,8,inf); //Skip the KwanSync marker
  fp FS=((fp)(250 << fs));
  double sens=FS/360.0; //rotations per second full scale
  sens*=2*PI;   //radians per second full scale
  sens/=32768;  //radians per second per DN
  sensX=sens; //Everyone gets nominal sensitivity
  sensY=sens;
  sensZ=sens;
}

void PlaybackState::propagate(int ms) {
  struct ccsdsHeader* ccsds=(struct ccsdsHeader*)buf;
  #include "extract_vars.INC"

  fread(buf,1,6,inf);
  ccsds->apid=ntohs(ccsds->apid) & 0x07FF;
  ccsds->seq=ntohs(ccsds->seq);
  ccsds->length=ntohs(ccsds->length);
  fread(buf+6,1,ccsds->length+1,inf);
  buf[ccsds->length+7]=0; //terminate a char* at the end of a packet
  if(ccsds->apid==0x1A) {
    #include "reverse_packet_nmea.INC"
    int nmeaDataLength=ccsds->length+7-(nmea->nmeaData-buf);
    memcpy(gpsBuf,nmea->nmeaData,nmeaDataLength);
    ttc=nmea->TC;
    gpsBuf[nmeaDataLength]=0;
    gpsTransPointer=0;
  }
  if(ccsds->apid==0x24) {
    #include "reverse_packet_nav2.INC"
    xRate=-nav2->gx*sensX;
    yRate=-nav2->gy*sensY;
    zRate=-nav2->gz*sensZ;
    ttc=nav2->TC;
    TCR[0][channelTCGyro]=peripherals.time.read_TTC(0); //Trigger a gyro reading
  }
  if(ccsds->apid==0x1C) {
    #include "reverse_packet_button.INC"
    ttc=button->TC;
    TCR[0][channelTCBtn]=peripherals.time.read_TTC(0);
  }
}

bool PlaybackState::done() {
  return feof(inf);
}

