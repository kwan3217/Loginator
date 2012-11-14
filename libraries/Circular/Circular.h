#ifndef Circular_h
#define Circular_h

//Circular buffer with attempt at atomic message write
//The buffer consists of a block of memory which conceptually contains
//two parts - data which is ready to be flushed, and data which is not
//yet ready. The buffer writer decides when the data which wasn't ready
//becomes so, and when it is, it is added to the data which is ready.
//The buffer gets written to by the generic fill() function. 
//A special drain() function empties data out of the buffer somehow and
//moves the head ptr up.
//All circular buffers are 1024 characters.
class Circular {
protected:
  int N;
  char* buf;
  //Location of the next slot to be written to
  int volatile head;
  //Location of the next slot not yet ready to be flushed
  int volatile mid;
  //Location of the next slot to be flushed
  int volatile tail;
  
public:
  Circular(int LN, char* Lbuf):N(LN),buf(Lbuf),head(0),mid(0),tail(0) {}

  //Is there no space to write another char?
  bool isFull();
  //Is there at least one char ready to be read?
  bool isEmpty();
  //Add a char to the buffer, not ready to be flushed yet
  bool fill(char in);

  void empty();

  bool fill(const char* in);
  bool fill(const char* in, int len);

  //Mark all current unready data as ready
  void mark();
  //Get the next character ready to be flushed
  char get();
  //Get all ready data from one buffer and copy it to another (as ready also)
  bool drain(Circular& to);
  //Get the number of characters which aren't ready yet
  int unreadylen();
  //Get the number of characters which are ready
  int readylen();

  char peekTail(int ahead);
  char peekMid(int ahead);
  char peekHead(int ahead);

  void pokeTail(int ahead, char poke);
  void pokeMid(int ahead, char poke);
  void pokeHead(int ahead, char poke);

  char* volatile headPtr() {return buf+head;}
  char* volatile tailPtr() {return buf+tail;}
  char* volatile midPtr()  {return buf+mid;}
};

#endif

