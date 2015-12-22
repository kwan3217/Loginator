template<class Pk,class Pr, class S> inline
bool FileCircular<Pk,Pr,S>::drainCore() {
  if(!ouf.append(buf+tail)) FAIL(ouf.errnum*100+1);
  fullState=false;
  tail=(tail+blockSize)%N;
  return true;
}
