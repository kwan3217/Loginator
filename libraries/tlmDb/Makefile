LIBMAKE+=../libraries/tlmDb/Makefile
EXTRAINCDIRS +=../libraries/tlmDb/
ATTACH+=TelemetryDatabase.ods
EXTRACLEAN+=$(wildcard write_packet_*.INC) $(wildcard reverse_packet_*.INC) extract_vars.INC extract_str.INC extract_body.INC extract_names.INC extractPacket.exe makePacketInclude.exe TelemetryDatabase.csv
EXTRACLEAN+=../libraries/tlmDb/makePacketInclude.o64
EXTRACLEAN+=../libraries/tlmDb/tlmDb.o64
EXTRACLEAN+=../libraries/tlmDb/extractPacket.o64
TLMDBSOURCE+=../libraries/tlmDb/makePacketInclude.cpp
TLMDBSOURCE+=../libraries/tlmDb/extractPacket.cpp
TLMDBSOURCE+=../libraries/tlmDb/tlmDb.cpp ../libraries/tlmDb/tlmDb.h
ATTACH+=$(TLMDBSOURCE)
EXTRADOC+=$(TLMDBSOURCE)

TelemetryDatabase.csv: TelemetryDatabase.ods
	DISPLAY= ssconvert TelemetryDatabase.ods TelemetryDatabase.csv

extract_str.INC: makePacketInclude.exe TelemetryDatabase.csv
	./makePacketInclude.exe TelemetryDatabase.csv

../libraries/tlmDb/extractPacket.o64: ../libraries/tlmDb/extractPacket.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
../libraries/tlmDb/makePacketInclude.o64: ../libraries/tlmDb/makePacketInclude.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
../libraries/tlmDb/tlmDb.o64: ../libraries/tlmDb/tlmDb.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
extractPacket.exe: ../libraries/tlmDb/extractPacket.o64 ../libraries/tlmDb/tlmDb.o64
	g++ -g -O0 -o $@ $^ 
	
makePacketInclude.exe: ../libraries/tlmDb/makePacketInclude.o64 ../libraries/tlmDb/tlmDb.o64
	g++ -g -O0 -o $@ $^ 

