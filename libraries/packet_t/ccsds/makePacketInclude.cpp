#include <fstream>
#include <iostream>
#include <map>
#include "tlmDb.h"

using namespace std;
using namespace tlmDb;

//Packet processing code - replaces tlmDb.pl, which means it can parse
//TelemetryDatabase.csv and produce the include files needed. May ultimately
//also replace extractPacket.cpp, meaning it can take any arbitrary
//TelemetryDatabase.csv and a recording and parse it into packets. A
//super-ambitious program would take just a recording, dig out the
//source, find the TelemetryDatabase.csv file, use it, and parse the rest of
//the recording.

void writeExtractVars(vector<Packet>& packets) {
  ofstream ouf;
  ouf.open("extract_vars.INC");
  for(auto i=packets.begin();i!=packets.end();i++) {
	auto packet=*i;
	ouf << "struct " << packet.shortName << " * " << packet.shortName << "=(struct " << packet.shortName << " *)buf;"<<endl;
  }
  ouf.close();
}

void writeExtractNames(vector<Packet>& packets) {
  ofstream ouf;
  ouf.open("extract_names.INC");
  ouf << "const char* packetNames[64]={"<<endl;
  vector<string> names;
  int max_apid=0;
  for(auto i=packets.begin();i!=packets.end();i++) {
	auto packet=*i;
	if(max_apid<packet.apid) max_apid=packet.apid;
  }
  for(int i=0;i<=max_apid;i++) names.push_back("");
  for(auto i=packets.begin();i!=packets.end();i++) {
	auto packet=*i;
	names[packet.apid]=packet.shortName;
  }
  for(int i=0;i<=max_apid;i++) {
	ouf << "\"" << names[i] << "\",";
  }
  ouf << "};"<<endl;
  ouf.close();
}

void writeExtractStr(vector<Packet>& packets) {
  ofstream ouf;
  ouf.open("extract_str.INC");
  for(auto i=packets.begin();i!=packets.end();i++) {
	auto packet=*i;
	ouf << "struct " << packet.shortName << " {" << endl;
	ouf << "  struct ccsdsHeader head;" << endl;
	if(packet.hasTC()) {
	  ouf << "  uint32_t TC __attribute__((packed));" << endl;
	}
	for(auto j=packet.fields.begin();j!=packet.fields.end();j++) {
	  auto field=*j;
      ouf << "  " << field.pretype << " " << field.name << field.array << " __attribute__((packed));"<<endl;
	}
    ouf << "};" << endl;
  }
  ouf.close();
}

void writeExtractBody(vector<Packet>& packets) {
  ofstream ouf;
  ouf.open("extract_body.INC");
  for(auto i=packets.begin();i!=packets.end();i++) {
	auto packet=*i;
	if(packet.fileExt!="") {
  	  ouf << "if(ccsds->apid==" << packet.apidStr << ") {" << endl;
	  ouf << "  static FILE* f" << packet.shortName << "=NULL;"<<endl;
	  ouf << "  if(f" << packet.shortName << "==NULL) {"<<endl;
	  if(packet.shortName==packet.fileExt) {
		ouf << "    sprintf(oufn,\"%s."<<packet.shortName<<"\",base);"<< endl;
	  } else {
		ouf << "    sprintf(oufn,\"%s."<<packet.shortName<<"."<<packet.fileExt<<"\",base);"<< endl;
	  }
	  ouf << "    f"<<packet.shortName<<"=fopen(oufn,\"wb\");"<<endl;
	}
	if(packet.extractor=="dump") {
	  ouf << "  }" << endl;
	  ouf << "  fwrite("<<packet.shortName<<"->"<<packet.fields[packet.fields.size()-1].name<<",1,ccsds->length+7-(";
	  ouf << packet.shortName<<"->"<<packet.fields[packet.fields.size()-1].name<<"-buf),f"<<packet.shortName<<");"<<endl;
      if(packet.hasTC()) {
        ouf << "  #include \"reverse_packet_"<<packet.shortName<<".INC\""<<endl;
        ouf << "  static unsigned int lastTC;"<<endl;
        ouf << "  static unsigned int min;"<<endl;
        ouf << "  if("<<packet.shortName<<"->TC<lastTC) min++;"<<endl;
        ouf << "  lastTC="<<packet.shortName<<"->TC;"<<endl;
      }
	} else if(packet.extractor=="csv") {
      ouf << "    fprintf(f"<<packet.shortName<<",\"";
      if(packet.hasTC()) {
        ouf << "TC,t,";
      }
      bool first=true;
      for(auto j=packet.fields.begin();j!=packet.fields.end();j++) {
        auto field=*j;
        if(!first) {
          ouf << ",";
        }
        first=false;
        ouf << field.name;
      }
      ouf << "\\n\");"<<endl;
      ouf << "  }"<<endl;
      if(packet.hasTC()) {
        ouf << "  #include \"reverse_packet_"<<packet.shortName<<".INC\""<<endl;
        ouf << "  static unsigned int lastTC;"<<endl;
        ouf << "  static unsigned int min;"<<endl;
        ouf << "  if("<<packet.shortName<<"->TC<lastTC) min++;"<<endl;
        ouf << "  lastTC="<<packet.shortName<<"->TC;"<<endl;
      }
      if(packet.fields[packet.fields.size()-1].ntohType=="char[") { //Bug fixed that was present in tlmDb.pl -- output is different, but this program's output is correct.
        //Special case - if the last field is a string, add a null-terminator
        ouf << "  "<<packet.shortName<<"->"<<packet.fields[packet.fields.size()-1].name<<"[ccsds->length-6]=0;\n";
      }
      ouf << "  fprintf(f"<<packet.shortName<<",\"";
      if(packet.hasTC()) {
        ouf << "%10u,%f,";
      }
      first=true;
      for(auto j=packet.fields.begin();j!=packet.fields.end();j++) {
        auto field=*j;
        if(!first) {
          ouf << ",";
        }
        first=false;
        ouf << format[field.ntohType];
      }
      ouf << "\\n\","<<endl;
      if(packet.hasTC()) {
        ouf << "    "<<packet.shortName<<"->TC,(double)(min*60)+(double)("<<packet.shortName<<"->TC)/60/1000000,"<<endl;
      }
      first=true;
      for(auto j=packet.fields.begin();j!=packet.fields.end();j++) {
        auto field=*j;
        ouf << "    "<<packet.shortName<<"->"<<field.name;
        if(j+1!=packet.fields.end()) {
          ouf << ",\n";
        } else {
          ouf << "\n";
        }
        first=false;
      }
      ouf << "  );\n";
    }
    ouf << "};\n";
  }
  ouf.close();
}

void writeExtractReverse(vector<Packet>& packets) {
  ofstream ouf;
  for(auto i=packets.begin();i!=packets.end();i++) {
	auto packet=*i;
    ouf.open("reverse_packet_"+packet.shortName+".INC");
    if(packet.hasTC()) {
      ouf << packet.shortName<<"->TC=ntohl("<<packet.shortName<<"->TC);"<<endl;
    }
    if(packet.extractor=="csv") {
      for(auto j=packet.fields.begin();j!=packet.fields.end();j++) {
        auto field=*j;
        if(ntoh[field.ntohType]!="") ouf << "  "<<packet.shortName<<"->"<<field.name<<"="<<ntoh[field.ntohType]<<"("<<packet.shortName<<"->"<<field.name<<");"<<endl;
      }
    }
    ouf.close();
  }
}

void writeExtractWrite(vector<Packet>& packets) {
  ofstream ouf;
  for(auto i=packets.begin();i!=packets.end();i++) {
	auto packet=*i;
    ouf.open("write_packet_"+packet.shortName+".INC");
    if(packet.wrapRobot) {
      ouf << "ccsds.start(sdStore,"<<packet.apidStr;
      if(packet.hasTC()) ouf<< "," <<packet.TC;
      ouf << ");" << endl;
    }
    for(auto j=packet.fields.begin();j!=packet.fields.end();j++) {
      auto field=*j;
      if(field.source != "") ouf << "ccsds.fill" << fillv[field.ntohType] << "(" << field.source << "); //" << field.desc << endl;
    }
    if(packet.wrapRobot) {
      ouf << "ccsds.finish(" << packet.apidStr << ");"<<endl;
    }
    ouf.close();
  }
}

int main(int argc, char** argv) {
  vector<Packet> packets=read(argv[1]);
  writeExtractVars(packets);
  writeExtractNames(packets);
  writeExtractStr(packets);
  writeExtractBody(packets);
  writeExtractReverse(packets);
  writeExtractWrite(packets);
}

