#ifndef csv_h
#define csv_h

#include <string>
#include <vector>

bool nextField(std::string line, int& ptr, std::string& field);
std::vector<std::string> parseCsv(std::string& line);

#endif
