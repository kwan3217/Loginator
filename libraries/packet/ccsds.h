#ifndef ccsds_h
#define ccsds_h

struct ccsdsHeader {
  uint16_t apid __attribute__((packed));
  uint16_t seq __attribute__((packed));
  uint16_t length __attribute__((packed));
};

#endif
