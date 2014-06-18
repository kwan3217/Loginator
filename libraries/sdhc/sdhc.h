
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef SDHC_H
#define SDHC_H

#include "spi_user.h"
#include "Print.h"
//#define SDHC_PKT
#ifdef SDHC_PKT
#include "Circular.h"
#define FAILREC(x) {errno=(x);buf.fill32BE(errno);buf.mark();return false;}
#define SUCCEED    {buf.fill32BE(0);buf.mark();return true;}
#else
#define FAILREC(x) {errno=(x);return false;}
#define SUCCEED    {return true;}
#endif
#include "packet.h"

#define FAIL(x) {errno=(x);return false;}

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
  void print(Print &out);
  void fill(Packet& p);
};

class SDHC:spi_user {
private:
  static const int CMD_GO_IDLE_STATE         =  0; ///< response R1 
  static const int CMD_SEND_OP_COND          =  1; ///< response R1 
  static const int CMD_SEND_IF_COND          =  8; ///< response R7 
  static const int CMD_SEND_CSD              =  9; ///< response R1 
  static const int CMD_SEND_CID              = 10; ///< response R1 
static const int CMD_STOP_TRANSMISSION     = 0x0c; ///< response R1 
/* CMD13: response R2 */
static const int CMD_SEND_STATUS           = 0x0d;
/* CMD16: arg0[31:0]: block length, response R1 */
static const int CMD_SET_BLOCKLEN          = 0x10;
/* CMD17: arg0[31:0]: data address, response R1 */
static const int CMD_READ_SINGLE_BLOCK     = 0x11;
/* CMD18: arg0[31:0]: data address, response R1 */
static const int CMD_READ_MULTIPLE_BLOCK   = 0x12;
/* CMD24: arg0[31:0]: data address, response R1 */
static const int CMD_WRITE_SINGLE_BLOCK    = 0x18;
/* CMD25: arg0[31:0]: data address, response R1 */
static const int CMD_WRITE_MULTIPLE_BLOCK  = 0x19;
/* CMD27: response R1 */
static const int CMD_PROGRAM_CSD           = 0x1b;
/* CMD28: arg0[31:0]: data address, response R1b */
static const int CMD_SET_WRITE_PROT        = 0x1c;
/* CMD29: arg0[31:0]: data address, response R1b */
static const int CMD_CLR_WRITE_PROT        = 0x1d;
/* CMD30: arg0[31:0]: write protect data address, response R1 */
static const int CMD_SEND_WRITE_PROT       = 0x1e;
/* CMD32: arg0[31:0]: data address, response R1 */
static const int CMD_TAG_SECTOR_START      = 0x20;
/* CMD33: arg0[31:0]: data address, response R1 */
static const int CMD_TAG_SECTOR_END        = 0x21;
/* CMD34: arg0[31:0]: data address, response R1 */
static const int CMD_UNTAG_SECTOR          = 0x22;
/* CMD35: arg0[31:0]: data address, response R1 */
static const int CMD_TAG_ERASE_GROUP_START = 0x23;
/* CMD36: arg0[31:0]: data address, response R1 */
static const int CMD_TAG_ERASE_GROUP_END   = 0x24;
/* CMD37: arg0[31:0]: data address, response R1 */
static const int CMD_UNTAG_ERASE_GROUP     = 0x25;
/* CMD38: arg0[31:0]: stuff bits, response R1b */
static const int CMD_ERASE                 = 0x26;
/* ACMD41: arg0[31:0]: OCR contents, response R1 */
static const int CMD_SD_SEND_OP_COND       = 0x29;
/* CMD42: arg0[31:0]: stuff bits, response R1b */
static const int CMD_LOCK_UNLOCK           = 0x2a;
/* CMD55: arg0[31:0]: stuff bits, response R1 */
static const int CMD_APP=0x37;
/* CMD58: response R3 */
static const int CMD_READ_OCR              = 0x3a;
/* CMD59: arg0[31:1]: stuff bits, arg0[0:0]: crc option, response R1 */
static const int CMD_CRC_ON_OFF            = 0x3b;

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
  unsigned char card_type;
  unsigned char response[8];
  void send_byte(unsigned char b) {s->send_byte(b);};
  unsigned char rec_byte(void) {return s->rec_byte();};
  void send_command(unsigned char command, unsigned int arg, unsigned int response_len);
  void select_card() {s->select_cs(p0);};
  void unselect_card() {s->deselect_cs(p0);};
  uint32_t scale_block_address(uint32_t addr) {return (uint32_t)(card_type & SDHC_SPEC_SDHC ? addr : addr*512);};
#ifdef SDHC_PKT
  char buf_data[256];
#endif
public:
#ifdef SDHC_PKT
  Circular buf; 
#endif
  static const int BLOCK_SIZE=512;
  unsigned int errno;
  SDHC(HardSPI *Ls, int Lp0):spi_user(Ls,Lp0)
#ifdef SDHC_PKT
,buf(sizeof(buf_data),buf_data)
#endif
,errno(0) {};
  bool begin(void);
  bool available(void);

  bool read(uint32_t offset, char* buffer) {return read(offset,buffer,0,BLOCK_SIZE);}; 
  bool read(uint32_t offset, char* buffer, int start, int len);
  bool write(uint32_t offset, const char* buffer, uint32_t trace);
  bool get_info(SDHC_info& info);
};

#endif

