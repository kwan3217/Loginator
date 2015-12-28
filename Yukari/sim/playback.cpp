#include "playback.h"
//#include <time.h>
#include <string.h>
#include <stdio.h>
#include "navigate.h" //for clat
#include "sim.h"
#include "SimSd.h"
#include "Time.h"
#include "irq.h"
#include "Startup.h"
#include "PlaybackHmc.h"
#include "PlaybackGyro.h"

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

class SimAdcYukari: public SimAdc {
public:
  virtual uint32_t read_ADGDR(int port) override;
};

uint32_t SimAdcYukari::read_ADGDR(int port) {
  return 1 << 31;
}

PlaybackState playbackState;
SimUartYukari uart(playbackState);
SimGpio gpio;
SimSd simsd;
SimTime timer;
SimRtc rtc;
SimPwm pwm;
SimAdcYukari adc;
SimHmcYukari simhmc;
SimGyroYukari simgyro;
SimPeripherals peripherals(gpio,uart,timer,rtc,pwm,adc);

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

/* Simulate a delay by advancing the playbackState clock the right number of ticks */
void delay(unsigned int ms) {
  playbackState.ttc+=ms*60000;
  fprintf(stderr,"Delay %d ms, ttc now equals %lu\n",ms,playbackState.ttc);
}

int main(int argc, char** argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  playbackState.begin(argv[1],0);
  simsd.open("sim/sdcard");
  peripherals.spi.addSlave(0, 7,simsd);
  peripherals.ssp.addSlave(0,25,simgyro);
  peripherals.i2c.addSlave(0x1E,simhmc);
  peripherals.gpio.addListener(peripherals.spi,{7}); //Include in here all the SPI slaves' CS lines
  peripherals.gpio.addListener(peripherals.ssp,{25}); //Include in here all the SPI slaves' CS lines
  reset_handler_core(); //Do the stuff that the embedded reset_handler would do

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
  #include "extract_names.INC"

  fread(buf,1,6,inf);
  ccsds->apid=ntohs(ccsds->apid) & 0x07FF;
  ccsds->seq=ntohs(ccsds->seq) & 0x0FFF;
  ccsds->length=ntohs(ccsds->length);
  fread(buf+6,1,ccsds->length+1,inf);
  buf[ccsds->length+7]=0; //terminate a char* at the end of a packet
  dprintf(PLAYBACK,"Packet apid %d (0x%03x): %s\n",ccsds->apid,ccsds->apid,packetNames[ccsds->apid]);
  if(ccsds->apid==0x1A) {
    #include "reverse_packet_nmea.INC"
    int nmeaDataLength=ccsds->length+7-(nmea->nmeaData-buf);
    memcpy(gpsBuf,nmea->nmeaData,nmeaDataLength);
    timer.write_TTC(0,nmea->TC);
    gpsBuf[nmeaDataLength]=0;
    gpsTransPointer=0;
  }
  if(ccsds->apid==0x24) {
    #include "reverse_packet_nav2.INC"
	simgyro.setFromPacket(nav2->gx,nav2->gy,nav2->gz);
    xRate=-nav2->gx*sensX;
    yRate=-nav2->gy*sensY;
    zRate=-nav2->gz*sensZ;
    timer.write_TTC(0,nav2->TC);
    timer.write_TCR(0,channelTCGyro,timer.read_TTC(0));//Trigger a gyro reading
  }
  if(ccsds->apid==0x1C) {
    #include "reverse_packet_button.INC"
    timer.write_TTC(0,button->tcBtn);
    timer.write_TCR(0,channelTCBtn,timer.read_TTC(0));
  }
}

bool PlaybackState::done() {
  return feof(inf);
}

