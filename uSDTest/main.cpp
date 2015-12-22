#include "Serial.h"
#include "sdhc.h"
#include "dump.h"
#include "partition.h"

Base85 ih(&Serial);

SDHC sd(&SPI,7);
Partition p(&sd);
char buf[SDHC::BLOCK_SIZE];
extern char __xz_start__[];
extern char __xz_end__[];


void setup() {
  SDHC_info info;
  Serial.begin(115200);
  ih.region(__xz_start__,0,__xz_end__-__xz_start__,32);
  Serial.println("Starting up");
  bool sd_worked=sd.begin();
  Serial.println(sd_worked?"sd.begin Worked":"sd.begin didn't work");
  Serial.println(sd.errnum,DEC);
  if(!sd_worked) return;

  sd.get_info(&info);
  info.print(Serial);

  sd_worked=p.begin(0);
  Serial.println(sd_worked?"p.begin worked":"p.begin didn't work");
  Serial.println(sd.errnum,DEC);
  if(!sd_worked) return;
  
  p.print(Serial);

  sd_worked=p.read(0,buf);
  Serial.println(sd_worked?"p.read worked":"p.read didn't work");
  Serial.println(sd.errnum,DEC);
  if(!sd_worked) return;

  ih.region(buf,0,sizeof(buf),16);

}

void loop() {

}


