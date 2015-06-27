#include "agc_io.h"

extern const agc_t::fixedbanks _binary_rope_bin_start;
agc_io State(_binary_rope_bin_start);

extern "C" {
void __dso_handle() {}
void _exit() {}
void _sbrk() {}
void _kill() {}
void _getpid() {}
void _write() {}
void _close() {}
void _lseek() {}
void _read() {}
void _fstat() {}
void _isatty() {}
void __exidx_start() {}
void __exidx_end() {}
}

void setup() {

}

void loop() {
  State.step();
}

