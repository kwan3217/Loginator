
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sdhc.h"

//#define SD_DEBUG
#ifdef SD_DEBUG
#endif

#include "Serial.h"
#include "dump.h"
FILE* card;
Hd dump(Serial,32);

const uint32_t special_block=0x5B70;

bool SDHC::begin() {
  card=fopen("sim/sdcard","rb+");
  if(!card) FAIL(1);
  return true;
}

bool SDHC::available() {
  return true;
}

void SDHC::send_command(unsigned char command, unsigned int arg, unsigned int response_len) {

}

//Read an SD card block, but only record part of it in LPC memory.
bool SDHC::read(uint32_t block, char* buffer, int start, int len) {
  fseek(card,block*BLOCK_SIZE+start,SEEK_SET);
  if(!fread(buffer,len,1,card)) FAILREC(1);
  SUCCEED;
}

bool SDHC::write(uint32_t block, const char* buffer, uint32_t trace) {
  fseek(card,block*BLOCK_SIZE,SEEK_SET);
  if(!fwrite(buffer,BLOCK_SIZE,1,card)) FAILREC(2);
  SUCCEED;
}

bool SDHC::get_info(struct SDHC_info& info) {
  memset(&info, 0, sizeof(info));
  info.manufacturer = 'K';
  info.oem[0] = 'W';
  info.oem[1] = 'A';
  info.oem[2] = 'N';
  info.product[0]='S';
  info.product[1]='I';
  info.product[2]='M';
  info.product[3]='C';
  info.product[4]='R';
  info.product[5]='D';
  info.revision = 0;
  info.serial=3217;
  info.manufacturing_year = 15;
  info.manufacturing_month = 6;

  info.oem[3]=0;
  info.product[6]=0;

  // read csd register 
  info.flag_copy = 0;
  info.flag_write_protect = 0;
  info.flag_write_protect_temp = 0;
  info.format = SDHC_info::SDHC_FORMAT_UNKNOWN;

  struct stat sb;
  int fd=fileno(card);
  if(fd<0) FAILREC(3);
  if(fstat(fd,&sb)<0) FAILREC(4);
  
  info.capacity = sb.st_size;
  SUCCEED;
}

void SDHC_info::print(Print &out) {
  out.print("Manufacturer: ");
  out.println(manufacturer,DEC);
  out.print("OEM:          ");
  out.println((char*)oem);
  out.print("Product:      ");
  out.println((char*)product);
  out.print("Revision:     ");
  out.print(revision>>4,DEC);
  out.print(".");
  out.println(revision & 0x0F,DEC);
  out.print("Serial:       ");
  out.println(serial,DEC);
  out.print("Mfg Yr/Month: ");
  out.print(manufacturing_year+2000,DEC);
  out.print("/");
  out.println(manufacturing_month,DEC);
  out.print("Capacity:     ");
#ifdef U64
  out.print((unsigned long long)(capacity),DEC);
#else
  out.print((unsigned int)(capacity>>32),HEX);
  out.print("_");
  out.println((unsigned int)(capacity & 0xFFFFFFFF),HEX);
#endif
  out.print("Copied:       ");
  out.println(flag_copy,DEC);
  out.print("Write prot:   ");
  out.println(flag_write_protect,DEC);
  out.print("Tmp write prot:");
  out.println(flag_write_protect_temp,DEC);
  out.print("Format:        ");
  out.println(format,DEC);
}

void SDHC_info::fill(Packet& p) {
  p.fill(manufacturer);
  p.fill(oem,3);
  p.fill(product,6);
  p.fill(revision);           
  p.fill32(serial);           
  p.fill(manufacturing_year);  
  p.fill(manufacturing_month);
  p.fill64(capacity);         
  p.fill(flag_copy);          
  p.fill(flag_write_protect); 
  p.fill(flag_write_protect_temp);
  p.fill(format);                  
}
