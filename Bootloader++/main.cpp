#include "Serial.h"
#include "sdhc.h"
#include "dump.h"
#include "main_msc.h"

Base85 ih(&Serial);

SDHC sd(&SPI,7);

char buf[SDHC::BLOCK_SIZE];
extern char __xz_start__[];
extern char __xz_end__[];


void setup() {
  SDHC_info info;
  Serial.begin(115200);
  ih.region(__xz_start__,0,__xz_end__-__xz_start__,64);
  Serial.println("Starting up");
  bool sd_worked=sd.begin();
  Serial.printf("sd.begin %s. Status code %d\n",sd_worked?"Worked":"didn't work",sd.errno);
  if(!sd_worked) return;
  main_msc();
}

void loop() {

}


