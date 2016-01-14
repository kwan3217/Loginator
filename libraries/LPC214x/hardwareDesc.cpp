#include <string>
#include <fstream>
#include <iostream>
#include "csv.h"
#include "dump.h"
#include <locale> //for std::toupper

using namespace std;

class CoutPrint:public Print {
  virtual void write(unsigned char out) override {cout << out;}
};

void print_32(IntelHex dump, uint32_t value) {
  dump.print_byte((value >> (8*0)) & 0xFF);
  dump.print_byte((value >> (8*1)) & 0xFF);
  dump.print_byte((value >> (8*2)) & 0xFF);
  dump.print_byte((value >> (8*3)) & 0xFF);
}

vector<string> part_types={
  "SD",
  "MPU60x0",
  "L3G4200D",
  "AD799x",
  "ADXL345",
  "AK8975",
  "BMP180",
  "HMC5883L",
  "LED",
  "BMA180",
  "GPS",
  "Button",
  "Servo"
};

vector<string> port_types={
  "SPI",
  "I2C",
  "GPIO",
  "PWM",
  "UART"
};

vector<vector<string>> custom0_values {
	{},  //SD
	{},  //MPU60x0
	{},  //L3G4200D
	{},  //AD799x
	{},  //ADXL345
	{},  //AK8975
	{},  //BMP180
	{},  //HMC5883L
	{"red","green","blue","amber"},  //LED
	{},  //BMA180
	{},  //GPS
	{},  //Button
	{"steer","throttle"}  //Servo
};

int main() {
  ofstream ouf("hardwareDesc.H");
  ouf << "#ifndef hardwareDesc_H"<<endl;
  ouf << "#define hardwareDesc_H"<<endl;
  for(int i=0;i<part_types.size();i++) {
	string s=part_types[i];
	for(int j=0;j<s.length();j++) s[j]=toupper(s[j]);
	ouf << "const int PART_TYPE_" << s << "=" << i << ";" << endl;
  }
  for(int i=0;i<custom0_values.size();i++) {
	string s=part_types[i];
	for(int j=0;j<s.length();j++) s[j]=toupper(s[j]);
	for(int k=0;k<custom0_values[i].size();k++) {
  	  string s2=custom0_values[i][k];
	  for(int j=0;j<s2.length();j++) s2[j]=toupper(s2[j]);
	  ouf << "const int CUSTOM0_" << s << "_" << s2 << "=" << i << ";" << endl;
	}
  }
  ouf << "#endif" << endl;
  ouf.close();
  string line;
  vector<string> sfields;
  CoutPrint coutPrint;
  IntelHex dump(coutPrint);
  uint32_t part_type;
  uint32_t port_type;
  uint32_t const unknown=0xFFFF'FFFF;
  unsigned int addr=0x7c000;
  dump.begin();
  dump.address(addr);
  while(cin) {
	getline(cin,line);
	if(line!="") { //getline reads the last (blank line) while the perl tlmDb.pl doesn't
	  sfields=parseCsv(line);
      //If the first field is non-zero length and starts with a digit
	  if(sfields[0].length()>0) {
		part_type=unknown;
		//Check if we match one of the part types
		for(int i=0;i<part_types.size()-1;i++) if(sfields[0]==part_types[i]) part_type=i;
		//If not, do we match a number? If not, then this is either a header row or a mistake
		if(part_type==unknown && sfields[0][0]>='0' && sfields[0][0]<='9') part_type=stoi(sfields[0]);
		if(part_type!=unknown) {
		  dump.begin_line(32,addr & 0xFFFF,0);
		  print_32(dump,part_type);
		  port_type=unknown;
		  if(sfields[1].length()>0) {
            for(int i=0;i<port_types.size()-1;i++) if(sfields[1]==port_types[i]) port_type=i;
	        if(port_type==unknown && sfields[1][0]>='0' && sfields[1][0]<='9') port_type=stoi(sfields[1]);
		  }
		  print_32(dump,port_type);
		  print_32(dump,sfields[2].length()>0?stoi(sfields[2]):unknown);
		  print_32(dump,sfields[3].length()>0?stoi(sfields[3]):unknown);
		  int custom0=unknown;
		  if(sfields[4].length()>0) {
			for(int i=0;i<custom0_values[part_type].size();i++) if(sfields[4]==custom0_values[part_type][i]) custom0=i;
			if(custom0==unknown && sfields[4][0]>='0' && sfields[4][0]<='9') custom0=stoi(sfields[4]);
			if(custom0==unknown) cerr<<"Unrecognized custom0 "<<sfields[4]<<endl;
		  }
		  print_32(dump,custom0);
		  print_32(dump,sfields[5].length()>0?stoi(sfields[5]):unknown);
		  print_32(dump,sfields[6].length()>0?stoi(sfields[6]):unknown);
		  print_32(dump,sfields[7].length()>0?stoi(sfields[7]):unknown);
		  dump.end_line();
		  addr+=32;
		  dump.begin_line(32,addr & 0xFFFF,0);
		  for(int i=0;i<32;i++) dump.print_byte(i<sfields[8].length()?sfields[8][i]:0);
		  dump.end_line();
		  addr+=32;
		} else {
		  cerr<<"Unrecognized part type "<<sfields[0]<<endl;
		}
	  }
	}
  }
  dump.end();
}



