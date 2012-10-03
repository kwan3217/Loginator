#ifndef HEAP_H
#define HEAP_H

//Because of C++ template are not the same as Java generics, you can't do a 
//clean separation of Heap.cpp and Heap.h, so we will do this one Java style
//in one file, with all code inline

template <typename priority, typename T>
struct pqnode {
  priority pty;
  T elt;
};

typedef unsigned int position;
template <typename priority, typename T>
class Heap {
private:
  static const position pqmaxsize=7;
  position pqsize;
  void siftup(position pos) {
    position j=pos/2,k=pos;
    priority py=pqueue[pos].pty;
  
    pqueue[0]=pqueue[pos];
    while(compare(pqueue[j].pty,py)>0) {
      pqueue[k]=pqueue[j];                                               
      k=j;j=j/2;
    }
    pqueue[k]=pqueue[0];
  };
  void siftdown(position pos, position n) {
    position i=pos,j=2*pos;
    pqnode<priority,T> save=pqueue[pos];
    bool finished=false;

    while(j<=n && !finished) {
      if(j<n) {
        if(compare(pqueue[j].pty,pqueue[j+1].pty)>0){
          j++;
        }
      }
      if(compare(save.pty,pqueue[j].pty)<=0){
        finished=true;
      } else {
        pqueue[i]=pqueue[j];
        i=j;j=2*i;
      }
    }
    pqueue[i]=save;
  };
//Returns a value such that the return value compared to zero has the same
//relation as a compared to b. 
  virtual int compare(priority a, priority b) {
    if(a<b) return -1;
    if(a==b) return 0;
    /*if(a>b)*/ return 1; //Trichotomoy
  };
  pqnode<priority,T> pqueue[pqmaxsize+1];
public:
  Heap(): pqsize(0) {}; //Takes place of create();
  void push(T entry, priority py){
    pqsize++;
    pqueue[pqsize].elt=entry;
    pqueue[pqsize].pty=py;
    siftup(pqsize);
  };
  void pop() {
    pqueue[1]=pqueue[pqsize];
    pqsize--;                        
    siftdown(1,pqsize);
  };
  pqnode<priority,T>& peek() {
    return pqueue[1];
  }
  void clear() {pqsize=0;};
  bool empty() {return pqsize==0;};
  bool full()  {return pqsize==pqmaxsize;};
};

#endif
