#include <string>
#include <fstream>
#include <iostream>
#include "csv.h"
#include "dump.h"

using namespace std;

class CoutPrint:public Print {
  virtual void write(unsigned char out) override {cout << out;}
};

int main() {
  string line;
  vector<string> sfields;
  CoutPrint coutPrint;
  IntelHex dump(coutPrint);
  int part_type;
  int port_type;
  int port_num;
  int part_addr;
  int custom0;
  int custom1;
  int custom2;
  int custom3;

  unsigned int addr=0x7c000;
  dump.begin();
  dump.address(addr);
  while(cin) {
	getline(cin,line);
	if(line!="") { //getline reads the last (blank line) while the perl tlmDb.pl doesn't
	  sfields=parseCsv(line);
      //If the first field is non-zero length and starts with a digit
	  if(sfields[0].length()>0 && sfields[0][0]>='0' && sfields[0][0]<='9') {
		  dump.begin_line(32,addr & 0xFFFF,0);
		  part_type=sfields[0].length()>0?stoi(sfields[0]):0xFFFFFFFF;
		  dump.print_byte((part_type >> (8*0)) & 0xFF);
		  dump.print_byte((part_type >> (8*1)) & 0xFF);
		  dump.print_byte((part_type >> (8*2)) & 0xFF);
		  dump.print_byte((part_type >> (8*3)) & 0xFF);
		  port_type=sfields[1].length()>0?stoi(sfields[1]):0xFFFFFFFF;
		  dump.print_byte((port_type >> (8*0)) & 0xFF);
		  dump.print_byte((port_type >> (8*1)) & 0xFF);
		  dump.print_byte((port_type >> (8*2)) & 0xFF);
		  dump.print_byte((port_type >> (8*3)) & 0xFF);
		  port_num =sfields[2].length()>0?stoi(sfields[2]):0xFFFFFFFF;
		  dump.print_byte((port_num >> (8*0)) & 0xFF);
		  dump.print_byte((port_num >> (8*1)) & 0xFF);
		  dump.print_byte((port_num >> (8*2)) & 0xFF);
		  dump.print_byte((port_num >> (8*3)) & 0xFF);
		  part_addr=sfields[3].length()>0?stoi(sfields[3]):0xFFFFFFFF;
		  dump.print_byte((part_addr >> (8*0)) & 0xFF);
		  dump.print_byte((part_addr >> (8*1)) & 0xFF);
		  dump.print_byte((part_addr >> (8*2)) & 0xFF);
		  dump.print_byte((part_addr >> (8*3)) & 0xFF);
		  custom0  =sfields[4].length()>0?stoi(sfields[4]):0xFFFFFFFF;
		  dump.print_byte((custom0 >> (8*0)) & 0xFF);
		  dump.print_byte((custom0 >> (8*1)) & 0xFF);
		  dump.print_byte((custom0 >> (8*2)) & 0xFF);
		  dump.print_byte((custom0 >> (8*3)) & 0xFF);
		  custom1  =sfields[5].length()>0?stoi(sfields[5]):0xFFFFFFFF;
		  dump.print_byte((custom1 >> (8*0)) & 0xFF);
		  dump.print_byte((custom1 >> (8*1)) & 0xFF);
		  dump.print_byte((custom1 >> (8*2)) & 0xFF);
		  dump.print_byte((custom1 >> (8*3)) & 0xFF);
		  custom2  =sfields[6].length()>0?stoi(sfields[6]):0xFFFFFFFF;
		  dump.print_byte((custom2 >> (8*0)) & 0xFF);
		  dump.print_byte((custom2 >> (8*1)) & 0xFF);
		  dump.print_byte((custom2 >> (8*2)) & 0xFF);
		  dump.print_byte((custom2 >> (8*3)) & 0xFF);
		  custom3  =sfields[7].length()>0?stoi(sfields[7]):0xFFFFFFFF;
		  dump.print_byte((custom3 >> (8*0)) & 0xFF);
		  dump.print_byte((custom3 >> (8*1)) & 0xFF);
		  dump.print_byte((custom3 >> (8*2)) & 0xFF);
		  dump.print_byte((custom3 >> (8*3)) & 0xFF);
		  dump.end_line();
		  addr+=32;
		  dump.begin_line(32,addr & 0xFFFF,0);
		  for(int i=0;i<32;i++) dump.print_byte(i<sfields[8].length()?sfields[8][i]:0);
		  dump.end_line();
		  addr+=32;
	  }
	}
  }
  dump.end();
}



