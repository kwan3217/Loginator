LIBMAKE+=../libraries/packet/ccsds/Makefile
CPPSRC+=../libraries/packet/ccsds/ccsds.cpp
include ../libraries/packet/Makefile
EXTRAINCDIRS +=../libraries/packet/ccsds/
ATTACH+=TelemetryDatabase.ods
EXTRACLEAN+=$(wildcard write_packet_*.INC) $(wildcard reverse_packet_*.INC) extract_vars.INC extract_str.INC extract_body.INC extract_names.INC extractPacket.exe makePacketInclude.exe TelemetryDatabase.csv
EXTRACLEAN+=../libraries/packet/ccsds/makePacketInclude.o64
EXTRACLEAN+=../libraries/packet/ccsds/tlmDb.o64
EXTRACLEAN+=../libraries/packet/ccsds/extractPacket.o64
TLMDBSOURCE+=../libraries/packet/ccsds/makePacketInclude.cpp
TLMDBSOURCE+=../libraries/packet/ccsds/extractPacket.cpp
TLMDBSOURCE+=../libraries/packet/ccsds/tlmDb.cpp ../libraries/packet/ccsds/tlmDb.h
ATTACH+=$(TLMDBSOURCE)
EXTRADOC+=$(TLMDBSOURCE)

TelemetryDatabase.csv: TelemetryDatabase.ods
	DISPLAY= ssconvert TelemetryDatabase.ods TelemetryDatabase.csv

extract_str.INC: makePacketInclude.exe TelemetryDatabase.csv
	./makePacketInclude.exe TelemetryDatabase.csv

../libraries/packet/ccsds/extractPacket.o64: ../libraries/packet/ccsds/extractPacket.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
../libraries/packet/ccsds/makePacketInclude.o64: ../libraries/packet/ccsds/makePacketInclude.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
../libraries/packet/ccsds/tlmDb.o64: ../libraries/packet/ccsds/tlmDb.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
extractPacket.exe: ../libraries/packet/ccsds/extractPacket.o64 ../libraries/packet/ccsds/tlmDb.o64
	g++ -g -O0 -o $@ $^ 
	
makePacketInclude.exe: ../libraries/packet/ccsds/makePacketInclude.o64 ../libraries/packet/ccsds/tlmDb.o64
	g++ -g -O0 -o $@ $^ 

