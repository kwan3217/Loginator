#ifndef FLASH_H
#define FLASH_H

#define IAP_CMD_SUCCESS          0
#define IAP_INVALID_COMMAND      1
#define IAP_SRC_ADDR_ERROR       2
#define IAP_DST_ADDR_ERROR       3
#define IAP_SRC_ADDR_NOT_MAPPED  4
#define IAP_DST_ADDR_NOT_MAPPED  5
#define IAP_COUNT_ERROR          6
#define IAP_INVALID_SECTOR       7
#define IAP_SECTOR_NOT_BLANK     8
#define IAP_SECTOR_NOT_PREPARED  9
#define IAP_COMPARE_ERROR       10
#define IAP_BUSY                11

int write_flash(char* source, int length, char* dest);
int erase_flash(int sector0, int sector1);
int flash_sector(char* x);

#endif
