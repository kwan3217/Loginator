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

class SimHmcYukari: public SimI2cSlave {
private:
  bool getAddr=false;
  int addr;
  uint8_t reg[13];
public:
  virtual void start() override {getAddr=true;dprintf(SIMHMC,"HMC5883 received start\n");};
  virtual void stop() override {dprintf(SIMHMC,"HMC5883 received stop\n");};
  virtual void repeatStart() override {getAddr=true;dprintf(SIMHMC,"HMC5883 received repeated start\n");};
  virtual uint8_t readByte(bool ack) override;
  virtual void writeByte(uint8_t write, bool& ack) override;
};

uint8_t SimHmcYukari::readByte(bool ack) {
  //Ack will be false on last byte, true on others
  uint8_t result=reg[addr];
  dprintf(SIMHMC,"Reading %d (0x%02x) from register %d (0x%02x) on HMC5883\n",result,result,addr,addr);
  addr++;
  return result;
}

void SimHmcYukari::writeByte(uint8_t write, bool& ack) {
  ack=true;
  if(getAddr) {
	addr=write;
	getAddr=false;
    dprintf(SIMHMC,"Addressing register %d (0x%02x) on HMC5883\n",write,write);
  } else {
    dprintf(SIMHMC,"Writing %d (0x%02x) to register %d (0x%02x) on HMC5883\n",write,write,addr,addr);
    addr++;
  }
}

class SimGyroYukari: public SimSpiSlave {
private:
  uint8_t reg[0x38+1]; //Highest numbered register, plus one for register 0
  bool getAddr=false;
  bool read=false;
  bool multi=false;
  int addr;
public:
  SimGyroYukari() {reg[0x0F]=0b1101'0011;};
  void setFromPacket(int16_t x, int16_t y, int16_t z);
  virtual void csOut (int value) override {getAddr=true;dprintf(SIMGYRO,"csOut=%d (%s)\n",value,value==0?"selected":"deselected");};
  virtual int  csIn  (         ) override {dprintf(SIMGYRO,"csIn=%d\n",1);return 1;};
  virtual void csMode(bool out ) override {dprintf(SIMGYRO,"csMode=%s\n",out?"out":"in");};
  virtual uint8_t transfer(uint8_t value) override;
};

void SimGyroYukari::setFromPacket(int16_t x, int16_t y, int16_t z) {
  dprintf(SIMGYRO,"Measurement: x=%d, y=%d, z=%d\n",x,y,z);
  reg[0x28]=uint8_t( x       & 0xFF);
  reg[0x29]=uint8_t((x >> 8) & 0xFF);
  reg[0x2A]=uint8_t( y       & 0xFF);
  reg[0x2B]=uint8_t((y >> 8) & 0xFF);
  reg[0x2C]=uint8_t( z       & 0xFF);
  reg[0x2D]=uint8_t((z >> 8) & 0xFF);
}

uint8_t SimGyroYukari::transfer(uint8_t value) {
  uint8_t result=0xFF;
  if(getAddr) {
	getAddr=false;
    addr=value & ((1<<6)-1);
    read=(value & 1<<6)!=0;
    multi=(value & 1<<7)!=0;
    dprintf(SIMGYRO,"Addressing: Received 0x%02x, Read=%d, Multi=%d, Addr=%d, sending 0x%02x\n",value,read,multi,addr,result);
  } else {
	if(read) {
      dprintf(SIMGYRO,"Reading register 0x%02x, value=%d (0x%02x)\n",addr,value);
      result=reg[addr];
	} else {
      dprintf(SIMGYRO,"Writing register 0x%02x, value=%d (0x%02x)\n",addr,value);
      reg[addr]=value;
	}
	if(multi) {
	  addr++;
      dprintf(SIMGYRO,"Auto-increment register address, now Writing register 0x%02x\n",addr);
	}
  }
  return result;
}

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
  peripherals.spi.addSlave(0, 7,simsd);
  peripherals.ssp.addSlave(0,25,simgyro);
  peripherals.i2c.addSlave(0x1E,simhmc);
  peripherals.gpio.addListener(peripherals.spi,{7}); //Include in here all the SPI slaves' CS lines
  peripherals.gpio.addListener(peripherals.ssp,{25}); //Include in here all the SPI slaves' CS lines
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

