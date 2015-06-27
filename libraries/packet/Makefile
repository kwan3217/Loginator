LIBMAKE+=../libraries/packet/Makefile
CPPSRC+=../libraries/packet/packet.cpp
include ../libraries/Circular/Makefile
include ../libraries/float/Makefile
EXTRAINCDIRS +=../libraries/packet/
ATTACH+=../libraries/packet/tlmDb.pl TelemetryDatabase.ods ../libraries/packet/extractPacket.cpp
#This one isn't directly used by the robot code, so attach it explicitly
ATTACH+=../libraries/packet/ccsds.h
EXTRACLEAN+=$(wildcard write_packet_*.inc) $(wildcard reverse_packet_*.inc) extract_vars.inc extract_str.inc extract_body.inc extractPacket TelemetryDatabase.csv

TelemetryDatabase.csv: TelemetryDatabase.ods
	DISPLAY= ssconvert TelemetryDatabase.ods TelemetryDatabase.csv

extract_str.inc: ../libraries/packet/tlmDb.pl TelemetryDatabase.csv
	perl ../libraries/packet/tlmDb.pl TelemetryDatabase.csv

extractPacket: ../libraries/packet/extractPacket.cpp extract_str.inc
	clang -g -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS))

