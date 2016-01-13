#include "sim.h"
#include <string>
#include <fstream>

using namespace std;

uint8_t get_i4(string s, int i) {
  uint8_t digit=0;
  if(s[i]<'A') {
	digit=s[i]-'0';
  } else {
    digit=s[i]-'A'+10;
  }
  return digit;
}

uint8_t get_i8(string s, int i) {
  uint8_t result=0;
  result=(get_i4(s,i+0) << 4) |
		 (get_i4(s,i+1) << 0) ;
  return result;
}

uint16_t get_i16(string s, int i) {
  uint16_t result;
  result=(((uint16_t)get_i8(s,i+0)) << 8) |
		 (((uint16_t)get_i8(s,i+2)) << 0) ;
  return result;
}

SimId::SimId(char* infn) {
  for(int i=0;i<sizeof(idData);i++) idData[i]=0xFF;
  ifstream inf;
  inf.open(infn);
  string line;
  unsigned int addr;
  int length;
  while(inf) {
	getline(inf,line);
	if(line!="" && line[0]==':' && line[8]=='0') { //Ignore address records
	  addr=get_i16(line,3)-0xC000;
	  length=get_i8(line,1);
	  for(int i=0;i<length;i++) idData[addr+i]=get_i8(line,i*2+9);
	}
  }
}
