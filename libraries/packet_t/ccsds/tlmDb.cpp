#include "tlmDb.h"
#include <fstream>
#include <iostream>

namespace tlmDb {

using namespace std;

static const Field fieldTC={
  "TC",       //name;  ///<Name of this field, exactly as-is in the spreadsheet. MUST be a valid C/C++ identifier
  "uint32_t", //type;  ///<Type of this field, exactly as-is in the spreadsheet. MUST be a valid C/C++ type. If it has empty brackets[], it is an unbounded array. If it has a bracket with a number, it is a bounded array.
  "",         //source;  ///<Code which is used to calculate the field value, in the robot execution context. MUST be a single C++ expression.
  "TC",       //unit;  ///<Units of this field, SHOULD be either an SI unit for physical values or DN or TC for digital values.
  "Timer value at relevant moment for this packet",// desc;  ///<Description of this field
  false,      // le; ///<true if the value is little-endian, false (normal case) if the value is big-endian. Read from spreadsheet, true iff first character of this field is in [Ll].
  "uint32_t", // pretype;
  "",         // array;
  "uint32_t"  // ntohType;
};

void Field::set(vector<string>& sfields) {
  if(sfields.size()> 6) source=sfields[ 6];
  if(sfields.size()> 7) name  =sfields[ 7];
  if(sfields.size()> 8) type  =sfields[ 8];
  if(sfields.size()> 9) unit  =sfields[ 9];
  if(sfields.size()>10) desc  =sfields[10];
                        le=(sfields.size()>11 && (sfields[11][0]=='l' || sfields[11][0]=='L'));
  pretype=type;
  ntohType=type;
  array="";
  for(int i=0;i<type.length();i++) if(type[i]=='[') {
	pretype=type.substr(0,i);
	ntohType=pretype+"[";
	array=type.substr(i);
  }
}

void Packet::set(vector<string>& sfields) {
  if(sfields.size()> 0) apidStr  =sfields[0];
  if(sfields.size()> 1) shortName=sfields[1];
  if(sfields.size()> 2) wrapRobot=(sfields[2][0]=='y' || sfields[2][0]=='Y');
  if(sfields.size()> 3) fileExt  =sfields[3];
  if(sfields.size()> 4) TC       =sfields[4];
  if(sfields.size()> 5) extractor=sfields[5];
  apid=stoi(apidStr,nullptr,0);
  fields.clear();
  fieldStart.clear();
  if(hasTC()) {
	add(fieldTC);
  }
}

void Packet::add(const Field& field) {
  if(fields.size()==0) {
	fieldStart.push_back(6);
  } else {
	int lastpos=fieldStart[fields.size()-1];
	string lasttype=fields[fields.size()-1].ntohType;
	int lastsize=typeSize.at(lasttype);
	fieldStart.push_back(lastpos+lastsize);
  }
  fields.push_back(field);
}

bool nextField(string line, int& ptr, string& field) {
  field="";
  //Our spreadsheets are written such that if a cell needs to be in quotes, then the entire value of the cell is in quotes.
  if(line[ptr]=='"') {
	ptr++; //Point past the starting quote
	while(line[ptr]!='"' && ptr<line.length()) {
	  field+=line[ptr];
	  ptr++;
	}
	//point past the ending quote and comma
	ptr+=2;
  } else {
	while(line[ptr]!=',' && ptr<line.length()) {
	  field+=line[ptr];
	  ptr++;
	}
	//point past the comma
	ptr++;
  }
  return ptr<line.length();
}

vector<string> parseCsv(string& line) {
  vector<string> result;
  string sfield;
  int ptr=0;
  //Read the CSV field headers
  while(nextField(line,ptr,sfield)) result.push_back(sfield);
  result.push_back(sfield); //Get the last field
  return result;
}


vector<Packet> read(istream& in) {
  vector<string> headers;
  vector<Packet> packets;

  string line,sfield;
  Packet packet;
  Field field;
  vector<string> sfields;
  getline(in,line);
  //Read the CSV field headers
  headers=parseCsv(line);
  //Read the first line, which must have an apid
  getline(in,line);
  sfields=parseCsv(line);
  packet.set(sfields);
  field.set(sfields);
  packet.add(field);
  while(in) {
	getline(in,line);
	if(line!="") { //getline reads the last (blank line) while the perl tlmDb.pl doesn't
	  sfields=parseCsv(line);
	  //Do we have a new packet?
	  if(sfields[0]!="") {
	    packets.push_back(packet);
	    packet.set(sfields);
	  }
	  field.set(sfields);
	  packet.add(field);
	}
  }
  //Store the last packet
  packets.push_back(packet);
  return packets;
}

vector<Packet> read(const string& infn) {
  ifstream inf;
  inf.open(infn);
  auto result=read(inf);
  inf.close();
  return result;
}

const map<string,string> ntoh={
	{"fp","ntohf"},
	{"int8_t",""},
	{"int16_t","ntohs"},
	{"int32_t","ntohl"},
	{"uint8_t",""},
	{"uint16_t","ntohs"},
	{"uint32_t","ntohl"},
	{"char[",""}
};

const map<string,string> format={
  {"fp","%f"},
  {"int8_t","%d"},
  {"int16_t","%d"},
  {"int32_t","%d"},
  {"uint8_t","%u"},
  {"uint16_t","%u"},
  {"uint32_t","%u"},
  {"char[","%s"}
};

const map<string,string> fillv={
  {"fp","fp"},
  {"int8_t",""},
  {"int16_t","16"},
  {"int32_t","32"},
  {"uint8_t",""},
  {"uint16_t","16"},
  {"uint32_t","32"},
  {"char[",""}
};

const map<string,int> typeSize={
  {"fp",4},
  {"int8_t",1},
  {"int16_t",2},
  {"int32_t",4},
  {"uint8_t",1},
  {"uint16_t",2},
  {"uint32_t",4},
  {"char[",1}
};

}
