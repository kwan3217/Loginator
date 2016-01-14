#include "csv.h"

using namespace std;

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

std::vector<std::string> parseCsv(std::string& line) {
  vector<string> result;
  string sfield;
  int ptr=0;
  //Read the CSV field headers
  while(nextField(line,ptr,sfield)) result.push_back(sfield);
  result.push_back(sfield); //Get the last field
  return result;
}

