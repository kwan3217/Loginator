#include <cstdio>
#include "crc.h"

int main() {
  const uint32_t crc=crc32("some-id");
  printf("The CRC32 is: %08x\n",crc);
}
