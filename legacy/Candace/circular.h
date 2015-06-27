#ifndef circular_h
#define circular_h

class circular;

#include "float.h"

//Circular buffer with attempt at atomic message write
//The buffer consists of a block of memory which conceptually contains
//two parts - data which is ready to be flushed, and data which is not
//yet ready. The buffer writer decides when the data which wasn't ready
//becomes so, and when it is, it is added to the data which is ready.
//The buffer gets written to by the generic fill() function. 
//A special drain() function empties data out of the buffer somehow and
//moves the head ptr up.
//All circular buffers are 1024 characters.
class circular {
private:
  char buf[1024];
  //Location of the next slot to be written to
  int volatile head;
  //Location of the next slot not yet ready to be flushed
  int volatile mid;
  //Location of the next slot to be flushed
  int volatile tail;
  
  //Some formatting stuff, not directly used by the circular buffer
public:
  int volatile dataDigits;
  int volatile dataDec;
  int volatile dataAsciiz;
  int volatile dataComma;
  int volatile dataBase85;
  char volatile dataDelim;

  circular():head(0),mid(0),tail(0),dataBase85(0) {}

  //Is there no space to write another char?
  int isFull();
  //Is there at least one char ready to be read?
  int isEmpty();
  //Add a char to the buffer, not ready to be flushed yet
  int fill(char in);

  void empty();

  //Built upon fill
  int fillString(const char* in);
  int fillDec(int in);
  int fill0Dec(int in, int len);
  int fillHex(unsigned int in, int len);
  int fillStringn(const char* in, int len);
  int fillShort(short in);
  int fillInt(int in);
  void fillError(const char* msg, unsigned int a, unsigned int b);

  //Mark all current unready data as ready
  void mark();
  //Get the next character ready to be flushed
  char get();
  //Get all ready data from one buffer and copy it to another (as ready also)
  int drain(circular* to);
  //Get the number of characters which aren't ready yet
  int unreadylen();
  //Get the number of characters which are ready
  int readylen();

  char peekTail(int ahead);
  char peekMid(int ahead);
  short peekMidShort(int ahead);
  int peekMidInt(int ahead);
  char peekHead(int ahead);

  void pokeTail(int ahead, char poke);
  void pokeMid(int ahead, char poke);
  void pokeHead(int ahead, char poke);

  void send(int port);
  char* volatile headPtr() {return buf+head;}
  char* volatile tailPtr() {return buf+tail;}
  char* volatile midPtr()  {return buf+mid;}
  void tailSwitch(int len) {tail=len-tail;}
};



#endif

