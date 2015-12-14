#ifndef SimSd_h
#define SimSd_h

#include "sim.h"

class SimCid {
public:

  uint8_t manufacturer;            ///< A manufacturer code globally assigned by the SD card organization.
  char oem[2];               ///< A string describing the card's OEM or content, globally assigned by the SD card organization.
  char product[5];           ///< A product name.
  uint8_t revision;                ///< The card's revision, coded in packed BCD. For example, the revision value \c 0x32 means "3.2".
  uint32_t serial;                   ///< A serial number assigned by the manufacturer.
  uint8_t manufacturing_month;      ///< The year of manufacturing. A value of zero means year 2000.
  uint8_t manufacturing_year;     ///< The month of manufacturing.
};

class SimCsd {
public:
  static const int SDHC_FORMAT_HARDDISK    = 0; ///< The card's layout is harddisk-like, which means it contains a master boot record with a partition table.
  static const int SDHC_FORMAT_SUPERFLOPPY = 1; ///< The card contains a single filesystem and no partition table. 
  static const int SDHC_FORMAT_UNIVERSAL   = 2; ///< The card's layout follows the Universal File Format.
  static const int SDHC_FORMAT_UNKNOWN     = 3; ///< The card's layout is unknown.
               uint8_t  csd_structure;
  static const uint8_t  taac=0x0E;
  static const uint8_t  nsac=0x00;
  static const uint8_t  tran_speed=0x5A;
  static const uint16_t ccc=0b111110110101;
  static const uint8_t  read_bl_len=9;
  static const uint8_t  read_bl_partial=0;
  static const uint8_t  write_blk_misalign=0;
  static const uint8_t  read_blk_misalign=0;
               uint8_t  dsr_imp;
               uint32_t c_size;
  static const uint8_t  erase_blk_en=1;
  static const uint8_t  sector_size=0x7f;
  static const uint8_t  wp_grp_size=0;
  static const uint8_t  wp_grp_enable=0;
  static const uint8_t  r2w_factor=0b010;
  static const uint8_t  write_bl_len=9;
  static const uint8_t  write_bl_partial=0;
  static const uint8_t  file_format_grp=0;
               uint8_t  copy;
               uint8_t  perm_write_protect;
               uint8_t  tmp_write_protect;
  static const uint8_t  file_format=2;
};

void write_bits(char data[], int datalen, uint32_t val, int hibit, int lowbit);

class SimSd:public SimSpi, public SimGpioListener {
private:
  static const SimCid cid;
  SimCsd csd;
  SimGpio& gpio;
  int cs;
  FILE* card;
  uint8_t cmd;
  uint32_t arg;
  uint8_t crc;
  int cmdptr;
  char response[10];
  bool responseReady=false;
  int responsePtr,responseLen;
  int responseDelayBytes;
  char data[512];
  bool dataReceive=false;
  bool dataReady=false;
  int dataPtr,dataLen;
  int dataDelayBytes;
  bool nextCommandAppSpecific=false;
  int blocklen;
  int HCS;
  static const int isSDHC=1;//Card is SDHC (0 would be SD standard capacity SDSC)
  static const uint32_t OCR=(1     <<31) | //Card is powered up 
                            (isSDHC<<30) | //Card is SDHC or SDSC
                            (0     <<29) | //UHS-II status
                            (1     <<23) | //3.5-3.6V acceptable
                            (1     <<22) | //3.4-3.5V acceptable
                            (1     <<21) | //3.3-3.4V acceptable
                            (1     <<20) | //3.2-3.3V acceptable
                            (1     <<19) | //3.1-3.2V acceptable
                            (1     <<18) | //3.0-3.1V acceptable
                            (1     <<17) | //2.9-3.0V acceptable
                            (1     <<16) | //2.8-2.9V acceptable
                            (1     <<15) ; //2.7-2.8V acceptable

//Copied from sdhc.h embedded driver
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
public:
  SimSd(SimGpio& Lgpio, int Lcs):gpio(Lgpio),cs(Lcs) {};
  bool open(char* cardfn);
  void close();
  virtual void pinOut(int port, int pin, int value) override;
  virtual int pinIn(int port, int pin) override;
  virtual void pinMode(int port, int pin, bool out) override;
  virtual uint32_t read_S0SPDR() override;
  virtual void write_S0SPDR(uint32_t value) override;
  void executeCommand();
//  virtual uint32_t read_S0SPSR() override;
};

#endif
