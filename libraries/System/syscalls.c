
#define UART_DR(baseaddr) (*(unsigned int *)(baseaddr))
#define UART_FR(baseaddr) (*(((unsigned int *)(baseaddr))+6))
 
int _close(int file) { return -1; }
 
int _fstat(int file, void *st) {
 return 0;
}
 
int _isatty(int file) { return 1; }
 
int _lseek(int file, int ptr, int dir) { return 0; }
 
int _open(const char *name, int flags, int mode) { return -1; }
 
int _read(int file, char *ptr, int len) {
  return 0;
}
 
void* _sbrk(int incr) {
  return 0;
}
 
int _write(int file, char *ptr, int len) {
  return 0;
}
