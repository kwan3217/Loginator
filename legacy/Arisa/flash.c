/*
Write blocks of data to flash memory, using the In-Application Programmer (IAP)

*/

#include "flash.h"
#include "setup.h"

// Sector Lookup 

int flash_sector(char* x) {
  if((unsigned int)x < 0x8000) {
    return ((unsigned int)x) >> 12;
  } else if ((unsigned int)x < 0x78000) {
    return ((((unsigned int)x)-0x8000) >> 15) + 8;
  } else if((unsigned int)x < 0x7D000) {
    return ((((unsigned int)x)-0x78000) >> 12) + 22;
  } else {
    //In the boot block 0x7D000-0x7FFFF or not in flash at all
    return -1;
  }
}

// In-Application Programming Entry Point 

void (*iap_fn)(unsigned int[],unsigned int[])=(void*)0x7ffffff1; //odd address, so Thumb code

int erase_flash(int sector0, int sector1) {
  if(sector0<0) return IAP_INVALID_SECTOR;
  if(sector1<0) return IAP_INVALID_SECTOR;
  unsigned int command[5];
  unsigned int result[3];
  
  // Prepare proper Sector 
  command[0] = 50; //prepare command code
  command[1] = sector0;
  command[2] = sector1;
  iap_fn(command,result);
  
  if(result[0]!=IAP_CMD_SUCCESS) return 0x10+result[0];

  // Erase the sectors
  command[0] = 52; //prepare command code
  command[1] = sector0;
  command[2] = sector1;
  command[3]=CCLK/1000; //CCLK in kHz
  iap_fn(command,result);
  
  if(result[0]!=IAP_CMD_SUCCESS) return 0x20+result[0];
  
  return IAP_CMD_SUCCESS;
}

int write_flash(char* source, int length, char* dest) {
  unsigned int command[5];
  unsigned int result[3];
  
  int s=flash_sector(dest);
  if(s<0) return IAP_INVALID_SECTOR;
  
  // Prepare proper Sector 
  command[0] = 50; //prepare command code
  command[1] = s;
  command[2] = s;
  iap_fn(command,result);
  
  if(result[0]!=IAP_CMD_SUCCESS) return 0x10+result[0];

  // Now write data 
  command[0] = 51; //write command code
  command[1]=(unsigned int)dest;
  command[2]=(unsigned int)source;
  command[3]=length;
  command[4]=CCLK/1000; //CCLK in kHz
  iap_fn(command,result);

  if(result[0]!=IAP_CMD_SUCCESS) return 0x20+result[0];

  //We're done with this write, so return whatever code we got
  return result[0];
}

