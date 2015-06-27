#include "Serial.h"
#include "sdhc.h"
#include "dump.h"
#include "partition.h"

IntelHex ih(&Serial);
Base85 b85(&Serial);

extern char __xz_start__[];
extern char __xz_end__[];

const char Leviathan[]=
  "Man is distinguished, not only by his reason, but by this singular passion "
  "from other animals, which is a lust of the mind, that by a perseverance of "
  "delight in the continued and indefatigable generation of knowledge, "
  "exceeds the short vehemence of any carnal pleasure.";

void setup() {
  SDHC_info info;
  Serial.begin(115200);
  ih.region(Leviathan,0,sizeof(Leviathan)-1,32);
  b85.region(Leviathan,0,sizeof(Leviathan)-1,64);

}

void loop() {

}


