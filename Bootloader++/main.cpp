/** Bootloader for Loginator and other LPC214x circuits. 

\mainpage
When the device is plugged into USB and reset, this bootloader takes over and presents
the attached microSD card as a mass storage device. When USB is disconnected (as detected
by the VBUS->USB_IN (P0.23) so pushing the boot button works too) or never connected in
the first place, the program reads the card to see if there is a file called FW.SFE .
If present, the bootloader treats this as a user firmware image and copies it into
LPC214x flash rom at the appropriate place. Once done, or if there was no FW.SFE in
the first place, the program calls the user firmware by jumping to the reset vector
of that program.
*/

#include "Serial.h"
#include "sdhc.h"
#include "dump.h"
#include "Partition.h"
#include "cluster.h"
#include "direntry.h"
#include "file.h"
#include <string.h>
#include "LPC214x.h"
#include "usb.h"

Base85 sourceDump(Serial);
Hd sectorDump(Serial);
SDHC sd(&SPI,7);
Partition p(sd);
Cluster fs(p);
File f(fs);
MSC msc(sd);

char buf[SDHC::BLOCK_SIZE];

void setup() {
  SDHC_info info;
  Serial.begin(115200);
  sourceDump.dumpSource();
  Serial.println("Starting up");
  bool sd_worked=sd.begin();
  Serial.printf("sd.begin %s. Status code %d\n",sd_worked?"Worked":"didn't work",sd.errno);
//  if(!sd_worked) return;

//  sd.get_info(info);
//  info.print(Serial);

//  sd_worked=p.begin(1);
//  Serial.printf("p.begin %s. Status code %d\n",sd_worked?"Worked":"didn't work",sd.errno);
//  if(!sd_worked) return;
//  p.print(Serial);  

//  p.read(0,buf);
//  sectorDump.region(buf,0,sizeof(buf),16);

//  sd_worked=fs.begin();  
//  Serial.printf("fs.begin %s. Status code %d\n",sd_worked?"Worked":"didn't work",sd.errno);
//  fs.print(Serial,sectorDump);
//  if(!sd_worked) return;

//  Serial.println(f.openr("vg1_v02.txt")?"Opened":"Couldn't open");
//  int ofs=0;
//  while(f.read(buf)) {
//    sectorDump.region(buf,ofs,sizeof(buf),32);
//    ofs+=sizeof(buf);
//  }

  msc.begin();
  msc.loop();
/*
  if(IOPIN0 & (1<<23))	{		//Check to see if the USB cable is plugged in
    main_msc();					//If so, run the USB device driver.
  } else {
    Serial.println("No USB Detected");
  }
*/
}

void loop() {

}


