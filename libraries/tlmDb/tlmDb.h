#ifndef tlmDb_h
#define tlmDb_h

#include <vector>
#include <string>
#include <map>

namespace tlmDb {

class FieldDef {
public:
  std::string fill;
  std::string ntoh;
  std::string format;
  int typeSize;
  FieldDef(std::string Lfill,std::string Lntoh, std::string Lformat, int LtypeSize):
	  fill(Lfill), ntoh(Lntoh), format(Lformat), typeSize(LtypeSize) {};
};

class Field {
public:
  std::string name;  ///<Name of this field, exactly as-is in the spreadsheet. MUST be a valid C/C++ identifier
  std::string specialFill;
  std::string type;  ///<Type of this field, exactly as-is in the spreadsheet. MUST be a valid C/C++ type. If it has empty brackets[], it is an unbounded array. If it has a bracket with a number, it is a bounded array.
  std::string source;  ///<Code which is used to calculate the field value, in the robot execution context. MUST be a single C++ expression.
  std::string unit;  ///<Units of this field, SHOULD be either an SI unit for physical values or DN or TC for digital values.
  std::string desc;  ///<Description of this field
  bool   le; ///<true if the value is little-endian, false (normal case) if the value is big-endian. Read from spreadsheet, true iff first character of this field is in [Ll].
  int    arraySize(); ///< Number of elements in field. One if scalar or one-element array, greater if array of known size, negative if unbounded array. Calculated from spreadsheet type field
  int    elementSize(); ///<size of field in bytes. If the field is an array, this is the size of one element. Calculated from spreadsheet type field
  void set(std::vector<std::string>& sfields);
  std::string pretype;
  std::string array;
  std::string ntohType;
};

class Packet {
public:
  std::string apidStr;
  int apid;         ///< Application Process ID for this packet
  std::string shortName; ///< Name of this packet, must be a valid C/C++ identifier
  std::string buffer;   ///< String name of buffer to fill. Non-blank if we want
                    ///to automatically generate code to start and end the
                    ///packet. This is typically blank when the header is
                    ///generated in one routine, most commonly in main.cpp, and
                    ///the body is generated somewhere else, like a sensor routine.
  bool wrapRobot() {return buffer!="";};
  std::string fileExt;   ///< Extension to be used by extractPacket when dumping this packet type
  std::string TC;        ///< C++ expression, which when evaluated in the embedded code in the
                    ///right context, produces the timestamp to use on this packet. If the
                    ///packet doesn't require a timestamp, this field is blank
  std::string extractor; ///< One of "csv", "source", "dump". Used to control how the
                    ///extractPacket ground support program creates extracted packet tables.
  std::vector<Field> fields;
  void set(std::vector<std::string>& sfields);
  bool hasTC() {return TC!="";};
  std::vector<int> fieldStart;
  void add(const Field& field);
};

bool nextField(std::string line, int& ptr, std::string& field);
std::vector<std::string> parseCsv(std::string& line);
std::vector<Packet> read(std::istream& in);
std::vector<Packet> read(const std::string& infn);

extern const std::map<std::string,FieldDef> fieldDef;

}

#endif
