#ifndef HEAP_H
#define HEAP_H

//Because C++ template are not the same as Java generics, you can't do a
//clean separation of Heap.cpp and Heap.h, so we will do this one Java style
//in one file, with all code inline

template <typename priority, typename T>
struct pqnode {
  priority pty;
  T elt;
};

template <typename priority, typename T, int S>
class Heap {
private:
  typedef unsigned int position;
  position pqsize;
  pqnode<priority,T> pqueue[S+1];
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
  int compare(priority a, priority b) {
    if(a<b) return -1;
    if(a==b) return 0;
    /*if(a>b)*/ return 1; //Trichotomoy
  };
public:
  Heap(): pqsize(0) {}; //Takes place of create();
/** Decide if the heap is empty
 @return true if there are no entries in the heap, false otherwise
*/
  bool empty() {return pqsize==0;};
/** Decide if the heap is full
 @return true if no more entries can be pushed, false otherwise
*/
  bool full()  {return pqsize==S;};
/** Add an entry to the heap at the proper priority
 @param entry the value to be added to the heap
 @param py the priority of the entry
 @return true if the entry was added, falsw otherwise (heap is full)
*/
  bool push(T entry, priority py){
    if(full()) return false;
    pqsize++;
    pqueue[pqsize].elt=entry;
    pqueue[pqsize].pty=py;
    siftup(pqsize);
    return true;
  };
/** Remove the best priority item from the heap
 @return true if an entry was removed, false otherwise
*/
  bool pop() {
    if(empty()) return false;
    pqueue[1]=pqueue[pqsize];
    pqsize--;
    siftdown(1,pqsize);
    return true;
  };
/** Look at the top of the heap
  @return A pqnode describing the value on the top of the heap and its priority
*/
  pqnode<priority,T>& peek() {
    return pqueue[1];
  }
  void clear() {pqsize=0;};
};

#endif
