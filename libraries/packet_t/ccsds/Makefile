LIBMAKE+=../libraries/packet/ccsds/Makefile
CPPSRC+=../libraries/packet/ccsds/ccsds.cpp
include ../libraries/packet/Makefile
EXTRAINCDIRS +=../libraries/packet/ccsds/
ATTACH+=../libraries/packet/ccsds/tlmDb.pl TelemetryDatabase.ods ../libraries/packet/ccsds/extractPacket.cpp
EXTRACLEAN+=$(wildcard write_packet_*.INC) $(wildcard reverse_packet_*.INC) extract_vars.INC extract_str.INC extract_body.INC extractPacket TelemetryDatabase.csv
EXTRADOC+=../libraries/packet/ccsds/extractPacket.cpp

TelemetryDatabase.csv: TelemetryDatabase.ods
	DISPLAY= ssconvert TelemetryDatabase.ods TelemetryDatabase.csv

extract_str.INC: ../libraries/packet/ccsds/tlmDb.pl TelemetryDatabase.csv
	perl ../libraries/packet/ccsds/tlmDb.pl TelemetryDatabase.csv

extractPacket: ../libraries/packet/ccsds/extractPacket.cpp extract_str.INC
	clang -g -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS))

