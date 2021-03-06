
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <string.h>
#include <stdio.h>
#include "sdhc.h"
#include "gpio.h" //for delay(), and gpio_read() for sdhc::available()
#include "Time.h" //for delay(), and gpio_read() for sdhc::available()
#include "hardwareDesc.h"

//#define SD_DEBUG
#ifdef SD_DEBUG
#endif

#include "Serial.h"
#include "dump.h"
Hd dump(Serial,32);

const uint32_t special_block=0x5B70;

int SDHC::i_hwDesc;

SDHC::SDHC() {
  i_hwDesc++;
  while(HW_ID_PART_TYPE(i_hwDesc)!=static_cast<unsigned int>(partType::unknown) &&
		HW_ID_PART_TYPE(i_hwDesc)!=static_cast<unsigned int>(partType::sdCard)) {
	i_hwDesc++;
  }
  if(HW_ID_PART_TYPE(i_hwDesc)==static_cast<unsigned int>(partType::unknown)) return;
  p0=HW_ID_ADDRESS(i_hwDesc); //Don't use set_p0 as that releases current p0
  port=SPIA[HW_ID_PORT_NUM(i_hwDesc)];
}

bool SDHC::begin() {
  // enable outputs for MOSI, SCK, SS, input for MISO 
  int debug_pt=0;
  Serial.print(debug_pt);debug_pt++;
  SPI.claim_cs(p0);
  Serial.print(debug_pt);debug_pt++;
  unselect_card();
  Serial.print(debug_pt);Serial.print(debug_pt);Serial.print(debug_pt);debug_pt++;
  // initialize SPI with lowest frequency; max. 400kHz during identification mode of card 
  SPI.begin(400000,1,1);
  Serial.print(debug_pt);debug_pt++;

  // initialization procedure 
  //SD RAW NOT AVAILABLE
  if(!available()) FAIL(1);

  unselect_card();

  // card needs 74 cycles minimum to start up 
  for(unsigned short i = 0; i < 10; ++i) {
    // wait 8 clock cycles 
    rec_byte();
  }

  // address card 
  select_card();

  // reset card 
  for(unsigned short i = 0; ; ++i) {
    send_command(CMD_GO_IDLE_STATE, 0, 1);
    if(response[0] == R1_IDLE_STATE) break;

    if(i == 0x1ff) {
      //response not idle after 512 tries;
      unselect_card();
      FAIL(2);
    }
  }

  card_type=0;

  // check for version of SD card specification 
  send_command(CMD_SEND_IF_COND, 0x100 /* 2.7V - 3.6V */ | 0xaa /* test pattern */,5);
  if((response[0] & R1_ILL_COMMAND) == 0) { //response is bits 39:32
    if((response[3] & 0x01) == 0) { //15:12 reserved 11:8 voltage
      // card operation voltage range doesn't match 
      unselect_card();
      FAIL(3);
    } 
    if(response[4] != 0xaa) { //7:0 check pattern
      // wrong test pattern 
      unselect_card();
      FAIL(4);
    }
    // card conforms to SD 2 card specification 
    card_type |= SDHC_SPEC_2;
  } else {
    // determine SD/MMC card type 
    send_command(CMD_APP, 0,1);
    send_command(CMD_SD_SEND_OP_COND, 0,1);
    if((response[0] & R1_ILL_COMMAND) == 0) {
      // card conforms to SD 1 card specification 
      card_type |= SDHC_SPEC_1;
    } else {
      // MMC card 

    }
  }


  /* wait for card to get ready */
  uint32_t arg = 0;
  if(card_type & SDHC_SPEC_2) arg = 0x40000000; //Set the HCS bit
  for(uint16_t i = 0; ; ++i) {
    if(card_type & (SDHC_SPEC_1 | SDHC_SPEC_2)) {
      send_command(CMD_APP, 0,1);
      send_command(CMD_SD_SEND_OP_COND, arg,1);
    } else {
      send_command(CMD_SEND_OP_COND, 0, 1);
    }

    if((response[0] & R1_IDLE_STATE) == 0) break;

    if(i == 0x7fff) {
      unselect_card();
      FAIL(5);
    }
  }

  //Check if this is a SDHC card
  if(card_type & SDHC_SPEC_2) {
    send_command(CMD_READ_OCR, 0,5);
    if(response[0]) {
      unselect_card();
      FAIL(6);
    }
    //CCS bit 30 in OCR - set if SDHC/SDXC
    if(response[1] & 0x40) card_type |= SDHC_SPEC_SDHC;
  }

  // set block size to 512 bytes 
  send_command(CMD_SET_BLOCKLEN, BLOCK_SIZE,1);
  if(response[0]) {
    unselect_card();
    //BLOCK SIZE SET ERR
    FAIL(7);
  }

  // deaddress card 
  unselect_card();

  // switch to higher SPI frequency 
  SPI.setfreq(5000000);  
#ifdef SDHC_PKT
  buf.empty();
#endif
  return true;
}

bool SDHC::available() {
  unsigned int i;
  SPI.release_cs(p0);
  delay(10);
  i = !gpio_read(p0);
  SPI.claim_cs(p0);
  return i == 0x00;
}

void SDHC::send_command(unsigned char command, unsigned int arg, unsigned int response_len) {
  unsigned int i;
  unsigned char crc;

  switch(command) {
    case CMD_GO_IDLE_STATE:
      crc=0x95;
      break;
    case CMD_SEND_IF_COND:
      crc=0x87;
      break;
    default:
      crc=0xff;
      break;
  }
#ifdef SD_DEBUG
  Serial.print("Command: ");
  Serial.print(command, DEC);
  Serial.print(" 0x");
  Serial.print(arg, HEX);
  Serial.print(" 0x");
  Serial.println(crc,HEX);
#endif
  /* wait some clock cycles */
  rec_byte();

  /* send command via SPI */
  send_byte(0x40 | command);
  send_byte((arg >> 24) & 0xff);
  send_byte((arg >> 16) & 0xff);
  send_byte((arg >> 8) & 0xff);
  send_byte((arg >> 0) & 0xff);
  send_byte(crc);

  /* receive response */
  for(i = 0; i < 10; ++i) {
    response[0] = rec_byte();
    if(response[0] != 0xff) break;
  }
  for(i=1;i<response_len;i++) {
    response[i]=rec_byte();
  }
#ifdef SD_DEBUG
  Serial.print("Response:");
  for(i=0;i<response_len;i++) {
    Serial.print(" ");
    Serial.print(response[i], HEX);
  }
  Serial.println();
#endif
}

//Read an SD card block, but only record part of it in LPC memory.
bool SDHC::read(uint32_t block, char* buffer, int start, int len) {
#ifdef SDHC_PKT
  buf.fill32BE(block);
  buf.fill32BE((TTC(0) & 0xFFFFFFF0) | 0);
#endif
  if(start+len>BLOCK_SIZE) FAILREC(8);

  // address card 
  select_card();

  // send single block request 
  send_command(CMD_READ_SINGLE_BLOCK, scale_block_address(block),1);
  if(response[0]) {
    unselect_card();
    FAILREC(100*response[0]+9);
  }

  // wait for data block (start byte 0xfe) 
  while(rec_byte() != 0xfe);

  // read byte block 
  for(int i=0;i<start;i++) rec_byte();
  SPI.rx_block(0xFF,buffer,len);
  for(int i=start+len;i<BLOCK_SIZE;i++) rec_byte();
            
  // read and ignore crc16 
  rec_byte();
  rec_byte();
            
  // deaddress card 
  unselect_card();

  // allow card some time to finish 
  rec_byte();

  SUCCEED;
}

bool SDHC::write(uint32_t block, const char* buffer, uint32_t trace) {
#ifdef SDHC_PKT
  buf.fill32BE(block);
  buf.fill32BE((TTC(0) & 0xFFFFFFF0) | 1);
#endif

//  Serial.print("Trace: ");Serial.println((unsigned int)trace,HEX,8);
//  Serial.print("Block: ");Serial.println((unsigned int)block,HEX,8);
//  dump.region(buffer,512);

  // address card 
  select_card();

  // send single block request 
  send_command(CMD_WRITE_SINGLE_BLOCK, scale_block_address(block),1);
  if(response[0]) {
    unselect_card();
    FAILREC(100*response[0]+10);
  }

  // send start byte 
  send_byte(0xfe);

  // write byte block 
  SPI.tx_block(buffer,BLOCK_SIZE);

  // write dummy crc16 
  send_byte(0xff);
  send_byte(0xff);

  // wait while card is busy 
  while(rec_byte() != 0xff);
  rec_byte();

  // deaddress card 
  unselect_card();

  SUCCEED;
}

bool SDHC::get_info(struct SDHC_info& info) {
  if(!available()) FAIL(11);

  memset(&info, 0, sizeof(info));

  select_card();

  /* read cid register */
  send_command(CMD_SEND_CID, 0,1);
  if(response[0]) {
    unselect_card();
    FAIL(100*response[0]+12);
  }
  while(rec_byte() != 0xfe);
  unsigned char i;
  for(i = 0; i < 18; ++i) {
    unsigned char b = rec_byte();
    switch(i) {
      case 0:
        info.manufacturer = b;
        break;
      case 1:
      case 2:
        info.oem[i - 1] = b;
        break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        info.product[i - 3] = b;
        break;
      case 8:
        info.revision = b;
        break;
      case 9:
      case 10:
      case 11:
      case 12:
        info.serial |= (unsigned int) b << ((12 - i) * 8);
        break;
      case 13:
        info.manufacturing_year = b << 4;
        break;
      case 14:
        info.manufacturing_year |= b >> 4;
        info.manufacturing_month = b & 0x0f;
        break;
    }
  }
  info.oem[3]=0;
  info.product[6]=0;

  // read csd register 
  unsigned char csd_read_bl_len = 0;
  unsigned char csd_c_size_mult = 0;
  unsigned int csd_c_size = 0;
  unsigned char csd_structure=0;
  send_command(CMD_SEND_CSD, 0,1);
  if(response[0]) {
    unselect_card();
    FAIL(100*response[0]+13);
  }
  while(rec_byte() != 0xfe);
  for(i = 0; i < 18; ++i) {
    unsigned char b = rec_byte();
    if(i == 0) {
      csd_structure = b >> 6;
    } else if(i == 14) {
      if(b & 0x40) info.flag_copy = 1;
      if(b & 0x20) info.flag_write_protect = 1;
      if(b & 0x10) info.flag_write_protect_temp = 1;
      info.format = (b & 0x0c) >> 2;
    } else {
      if(csd_structure == 0x01) {
        switch(i) {
          case 7:
            b &= 0x3f;
          case 8:
          case 9:
            csd_c_size <<= 8;
            csd_c_size |= b;
            break;
        }
        if(i == 9) {
          ++csd_c_size;
          info.capacity = (uint64_t) csd_c_size * 512 * 1024;
        }
      } else if(csd_structure == 0x00) {
        switch(i) {
          case 5:
            csd_read_bl_len = b & 0x0f;
            break;
          case 6:
            csd_c_size = b & 0x03;
            csd_c_size <<= 8;
            break;
          case 7:
            csd_c_size |= b;
            csd_c_size <<= 2;
            break;
          case 8:
            csd_c_size |= b >> 6;
            ++csd_c_size;
            break;
          case 9:
            csd_c_size_mult = b & 0x03;
            csd_c_size_mult <<= 1;
            break;
          case 10:
            csd_c_size_mult |= b >> 7;
            info.capacity = (uint32_t) csd_c_size << (csd_c_size_mult + csd_read_bl_len + 2);
            break;
        }
      }
    }
  }

  unselect_card();

  return true;
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
