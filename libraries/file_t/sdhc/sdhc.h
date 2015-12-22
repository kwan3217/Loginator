
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef SDHC_H
#define SDHC_H

#include "Print.h"
#include "HardSPI.h"
//#define SDHC_PKT
#ifdef SDHC_PKT
#include "Circular.h"
#define FAILREC(x) {errnum=(x);buf.fill32BE(errnum);buf.mark();return false;}
#define SUCCEED    {buf.fill32BE(0);buf.mark();return true;}
#else
#define FAILREC(x) {errnum=(x);return false;}
#define SUCCEED    {return true;}
#endif
#include "packet.h"

#define FAIL(x) {errnum=(x);return false;}

#define ASSERT(x,y) {bool result=(x);if(!result) FAIL((y));return result;}

inline uint32_t tr(uint8_t system, uint8_t function, uint8_t call) {
  return ((uint32_t)system)<<16 | ((uint32_t)function)<<8 | ((uint32_t)call);
}

/**
 * \addtogroup sdhc
 *
 * @{
 */
/**
 * \file
 * MMC/SD raw access header.
 *
 * \author Roland Riegel and Chris Jeppesen
 */

/**
 * This struct is used by sdhc_get_info() to return
 * manufacturing and status information of the card.
 */
template <class Pk, class Pr>
class SDHC_info {
public:
  static const int SDHC_FORMAT_HARDDISK    = 0; ///< The card's layout is harddisk-like, which means it contains a master boot record with a partition table.
  static const int SDHC_FORMAT_SUPERFLOPPY = 1; ///< The card contains a single filesystem and no partition table. 
  static const int SDHC_FORMAT_UNIVERSAL   = 2; ///< The card's layout follows the Universal File Format.
  static const int SDHC_FORMAT_UNKNOWN     = 3; ///< The card's layout is unknown.

  unsigned char manufacturer;            ///< A manufacturer code globally assigned by the SD card organization.
  char oem[3+1];                ///< A string describing the card's OEM or content, globally assigned by the SD card organization.
  char product[6+1];            ///< A product name.
  unsigned char revision;                ///< The card's revision, coded in packed BCD. For example, the revision value \c 0x32 means "3.2".
  unsigned int serial;                   ///< A serial number assigned by the manufacturer.
  unsigned char manufacturing_year;      ///< The year of manufacturing. A value of zero means year 2000.
  unsigned char manufacturing_month;     ///< The month of manufacturing.
  uint64_t capacity;                     ///< The card's total capacity in bytes.
  unsigned char flag_copy;               ///<  Defines wether the card's content is original or copied. A value of \c 0 means original, \c 1 means copied.
  unsigned char flag_write_protect;      ///<  Defines wether the card's content is write-protected.
                                         ///  \note This is an internal flag and does not represent the
                                         ///   state of the card's mechanical write-protect switch.
  unsigned char flag_write_protect_temp; ///< Defines wether the card's content is temporarily write-protected.
                                         /// \note This is an internal flag and does not represent the
                                         ///  state of the card's mechanical write-protect switch.
  unsigned char format;                  ///< The card's data layout. See the \c SDHC_FORMAT_* constants for details. \note This value is not guaranteed to match reality.
  void print(Pr& out);
  void fill(Pk& p);
};

template<class Pk,class Pr>
inline void SDHC_info<Pk,Pr>::print(Pr &out) {
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

template<class Pk, class Pr>
void SDHC_info<Pk,Pr>::fill(Pk& p) {
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

template<class Pk,class Pr, class S>
class SDHC {
private:
  static const int CMD_GO_IDLE_STATE         = 0x00; ///< CMD00: response R1 
  static const int CMD_SEND_OP_COND          = 0x01; ///< CMD01: response R1 
  static const int CMD_SEND_IF_COND          = 0x08; ///< CMD08: response R7 
  static const int CMD_SEND_CSD              = 0x09; ///< CMD09: response R1 
  static const int CMD_SEND_CID              = 0x0A; ///< CMD10: response R1 
  static const int CMD_STOP_TRANSMISSION     = 0x0c; ///< CMD12: response R1 
  static const int CMD_SEND_STATUS           = 0x0d; ///< CMD13: response R2
  static const int CMD_SET_BLOCKLEN          = 0x10; ///< CMD16: arg0[31:0]: block length, response R1 
  static const int CMD_READ_SINGLE_BLOCK     = 0x11; ///< CMD17: arg0[31:0]: data address, response R1 
  static const int CMD_READ_MULTIPLE_BLOCK   = 0x12; ///< CMD18: arg0[31:0]: data address, response R1 
  static const int CMD_WRITE_SINGLE_BLOCK    = 0x18; ///< CMD24: arg0[31:0]: data address, response R1 
  static const int CMD_WRITE_MULTIPLE_BLOCK  = 0x19; ///< CMD25: arg0[31:0]: data address, response R1 
  static const int CMD_PROGRAM_CSD           = 0x1b; ///< CMD27: response R1 */
  static const int CMD_SET_WRITE_PROT        = 0x1c; ///< CMD28: arg0[31:0]: data address, response R1b */
  static const int CMD_CLR_WRITE_PROT        = 0x1d; ///< CMD29: arg0[31:0]: data address, response R1b */
  static const int CMD_SEND_WRITE_PROT       = 0x1e; ///< CMD30: arg0[31:0]: write protect data address, response R1 */
  static const int CMD_TAG_SECTOR_START      = 0x20; ///< CMD32: arg0[31:0]: data address, response R1 */
  static const int CMD_TAG_SECTOR_END        = 0x21; ///< CMD33: arg0[31:0]: data address, response R1 */
  static const int CMD_UNTAG_SECTOR          = 0x22; ///< CMD34: arg0[31:0]: data address, response R1 */
  static const int CMD_TAG_ERASE_GROUP_START = 0x23; ///< CMD35: arg0[31:0]: data address, response R1 */
  static const int CMD_TAG_ERASE_GROUP_END   = 0x24; ///< CMD36: arg0[31:0]: data address, response R1 */
  static const int CMD_UNTAG_ERASE_GROUP     = 0x25; ///< CMD37: arg0[31:0]: data address, response R1 */
  static const int CMD_ERASE                 = 0x26; ///< CMD38: arg0[31:0]: stuff bits, response R1b */
  static const int CMD_SD_SEND_OP_COND       = 0x29; ///< ACMD41: arg0[31:0]: OCR contents, response R1 */
  static const int CMD_LOCK_UNLOCK           = 0x2a; ///< CMD42: arg0[31:0]: stuff bits, response R1b */
  static const int CMD_APP                   = 0x37; ///< CMD55: arg0[31:0]: stuff bits, response R1 */
  static const int CMD_READ_OCR              = 0x3a; ///< CMD58: response R3 */
  static const int CMD_CRC_ON_OFF            = 0x3b; ///< CMD59: arg0[31:1]: stuff bits, arg0[0:0]: crc option, response R1 */

  // command responses 
  // R1: size 1 byte 
  static const int R1_IDLE_STATE    = 1 << 0;
  static const int R1_ERASE_RESET   = 1 << 1;
  static const int R1_ILL_COMMAND   = 1 << 2;
  static const int R1_COM_CRC_ERR   = 1 << 3;
  static const int R1_ERASE_SEQ_ERR = 1 << 4;
  static const int R1_ADDR_ERR      = 1 << 5;
  static const int R1_PARAM_ERR     = 1 << 6;
  // status bits for card types 
  static const int SDHC_SPEC_1=1 << 0;
  static const int SDHC_SPEC_2=1 << 1;
  static const int SDHC_SPEC_SDHC=1 << 2;
  S& spi;
  unsigned char card_type;
  unsigned char response[8];
  void send_byte(unsigned char b) {spi.send_byte(b);};
  unsigned char rec_byte(void) {return spi.rec_byte();};
  void send_command(unsigned char command, unsigned int arg, unsigned int response_len);
  void select_card() {spi.select_cs(p0);};
  void unselect_card() {spi.deselect_cs(p0);};
  uint32_t scale_block_address(uint32_t addr) {return (uint32_t)(card_type & SDHC_SPEC_SDHC ? addr : addr*512);};
#ifdef SDHC_PKT
  char buf_data[256];
#endif
public:
#ifdef SDHC_PKT
  Circular buf; 
#endif
  int p0;
  static const int BLOCK_SIZE=512;
  unsigned int errnum;
  bool begin(void);
  bool available(void);
  SDHC(S& Lspi):spi(Lspi) {};
//  bool read(uint32_t offset, char* buffer) {return read(offset,buffer,0,BLOCK_SIZE);}; 
  bool read(uint32_t offset, char* buffer, int start=0, int len=BLOCK_SIZE);
  bool write(uint32_t offset, const char* buffer, uint32_t trace);
  bool get_info(SDHC_info<Pk,Pr>& info);
};

#include "sdhc.inc"

#endif

