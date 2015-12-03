#ifndef HARDTWOWIRE_H
#define HARDTWOWIRE_H

#include "Wire.h"
#include "LPC214x.h"

template<class T,int port>class StateTwoWire;
template<class T,int port>void state00(StateTwoWire<T,port>& that);
template<class T,int port>void state08(StateTwoWire<T,port>& that);
template<class T,int port>void state10(StateTwoWire<T,port>& that);
template<class T,int port>void state18(StateTwoWire<T,port>& that);
template<class T,int port>void state20(StateTwoWire<T,port>& that);
template<class T,int port>void state28(StateTwoWire<T,port>& that);
template<class T,int port>void state30(StateTwoWire<T,port>& that);
template<class T,int port>void state38(StateTwoWire<T,port>& that);
template<class T,int port>void state40(StateTwoWire<T,port>& that);
template<class T,int port>void state48(StateTwoWire<T,port>& that);
template<class T,int port>void state50(StateTwoWire<T,port>& that);
template<class T,int port>void state58(StateTwoWire<T,port>& that);
template<class T,int port>void stateIn(StateTwoWire<T,port>& that);
template<class T,int port>void stateSl(StateTwoWire<T,port>& that);
template<class T,int port>void stateF8(StateTwoWire<T,port>& that);

template<class T,int port>
class StateTwoWire:public TwoWire<StateTwoWire<T,port>> {
  private:
    friend class TwoWire<StateTwoWire<T,port>>;
    friend void state00<T,port>(StateTwoWire<T,port>& that);
    friend void state08<T,port>(StateTwoWire<T,port>& that);
    friend void state10<T,port>(StateTwoWire<T,port>& that);
    friend void state18<T,port>(StateTwoWire<T,port>& that);
    friend void state20<T,port>(StateTwoWire<T,port>& that);
    friend void state28<T,port>(StateTwoWire<T,port>& that);
    friend void state30<T,port>(StateTwoWire<T,port>& that);
    friend void state38<T,port>(StateTwoWire<T,port>& that);
    friend void state40<T,port>(StateTwoWire<T,port>& that);
    friend void state48<T,port>(StateTwoWire<T,port>& that);
    friend void state50<T,port>(StateTwoWire<T,port>& that);
    friend void state58<T,port>(StateTwoWire<T,port>& that);
    friend void stateIn<T,port>(StateTwoWire<T,port>& that);
    friend void stateSl<T,port>(StateTwoWire<T,port>& that);
    friend void state58<T,port>(StateTwoWire<T,port>& that);
    /*virtual*/ void twi_init(unsigned int freq);
    uint8_t address;
    const char* dataWrite;
    char* dataRead;
    uint8_t lengthWrite;
    uint8_t lengthRead;
    volatile bool done;
    /*virtual*/ uint8_t twi_readFrom(uint8_t Laddress, char* Ldata, uint8_t Llength);
    /*virtual*/ uint8_t twi_writeTo(uint8_t Laddress, const char* Ldata, uint8_t Llength, uint8_t wait);
    static const int AA   =(1 << 2);
    static const int SI   =(1 << 3);
    static const int STO  =(1 << 4);
    static const int STA  =(1 << 5);
    static const int EN =(1 << 6);
    static StateTwoWire *thisPtr[2];
    static void IntHandler0();
    static void IntHandler1();
    void wait_si() {while(!(I2CCONSET(port) & SI)) ;}
    void stateDriver();
    typedef void (*State)(StateTwoWire<T,port>&);
    static const State state[];
  public:
    StateTwoWire();
};

#include "StateTwoWire.inc"

template<class T, int port>
const typename StateTwoWire<T,port>::State StateTwoWire<T,port>::state[]={&state00<T,port>,&state08<T,port>,
                              &state10<T,port>,&state18<T,port>,
                              &state20<T,port>,&state28<T,port>,
                              &state30<T,port>,&state38<T,port>,
                              &state40<T,port>,&state48<T,port>,
                              &state50<T,port>,&state58<T,port>,
                              &stateSl<T,port>,&stateSl<T,port>,
                              &stateSl<T,port>,&stateSl<T,port>,
                              &stateSl<T,port>,&stateSl<T,port>,
                              &stateSl<T,port>,&stateSl<T,port>,
                              &stateSl<T,port>,&stateSl<T,port>,
                              &stateSl<T,port>,&stateSl<T,port>,
                              &stateSl<T,port>,&stateSl<T,port>,
                              &stateIn<T,port>,&stateIn<T,port>,
                              &stateIn<T,port>,&stateIn<T,port>,
                              &stateIn<T,port>,&stateF8<T,port>
};


#endif




