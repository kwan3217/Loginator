#ifndef crc_h
#define crc_h

#include <cstdint>

const uint32_t Poly=0xedb88320;

// Generate an entry to the CRC lookup table
constexpr uint32_t gen_crc_table(uint32_t c, int k=8) {
  return k>0?gen_crc_table(((c & 1)? Poly : 0) ^ (c >> 1),k-1):c;
}

#define makeCrcTableA(x) makeCrcTableB(x) makeCrcTableB(x + 128)
#define makeCrcTableB(x) makeCrcTableC(x) makeCrcTableC(x +  64)
#define makeCrcTableC(x) makeCrcTableD(x) makeCrcTableD(x +  32)
#define makeCrcTableD(x) makeCrcTableE(x) makeCrcTableE(x +  16)
#define makeCrcTableE(x) makeCrcTableF(x) makeCrcTableF(x +   8)
#define makeCrcTableF(x) makeCrcTableG(x) makeCrcTableG(x +   4)
#define makeCrcTableG(x) makeCrcTableH(x) makeCrcTableH(x +   2)
#define makeCrcTableH(x) makeCrcTableI(x) makeCrcTableI(x +   1)
#define makeCrcTableI(x) gen_crc_table(x),

constexpr uint32_t crc_table[] = { makeCrcTableA(0) };

// Constexpr implementation and helpers. This uses tail recursion instead of a loop
// so that it can be a constexpr.
constexpr uint32_t crc32_impl(const uint8_t* p, size_t len, uint32_t crc) {
    return len ?
            crc32_impl(p+1,len-1,(crc>>8)^crc_table[(crc&0xFF)^*p])
            : crc;
}

constexpr uint32_t crc32(const uint8_t* data, size_t length) {
    return ~crc32_impl(data, length, ~0);
}

constexpr size_t strlen_c(const char* str) {
    return *str ? 1+strlen_c(str+1) : 0;
}

constexpr uint32_t crc32(const char* str) {
    return crc32((const uint8_t*)str, strlen_c(str));
}

#endif

