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
#include "dump.h"
#include <string.h>
#include "LPC214x.h"
#include "cdc.h"
//#include "gpio.h"

Base85 sourceDump(Serial,120);
USB usb;
CDC cdc(usb);

void setup() {
  Serial.begin(230400);
  sourceDump.dumpSource();
  Serial.println("Starting up");

  //Just check if the USB light works
//  set_pin(31, 0, 1);
//  gpio_write(31,0); //Run through a PMOS, so low turns it on
  usb.begin();
  cdc.begin();
  usb.SoftConnect(true);
  Serial.println("Through cdc.begin");
/*
  if(IOPIN0 & (1<<23))	{		//Check to see if the USB cable is plugged in
    main_msc();					//If so, run the USB device driver.
  } else {
    Serial.println("No USB Detected");
  }
*/
}

void loop() {
  usb.loop();
}


