LIBMAKE+=../libraries/tlmDb/Makefile
EXTRAINCDIRS +=../libraries/tlmDb/
ATTACH+=TelemetryDatabase.ods
EXTRACLEAN+=$(wildcard write_packet_*.INC) $(wildcard reverse_packet_*.INC) extract_vars.INC extract_str.INC extract_body.INC extract_names.INC extractPacket.exe makePacketInclude.exe TelemetryDatabase.csv
TLMDBSOURCE+=../libraries/tlmDb/makePacketInclude.cpp
TLMDBSOURCE+=../libraries/tlmDb/extractPacket.cpp
TLMDBSOURCE+=../libraries/tlmDb/tlmDb.cpp ../libraries/tlmDb/csv.cpp
TLMDBH+=../libraries/tlmDb/tlmDb.h ../libraries/tlmDb/csv.h
ATTACH+=$(TLMDBSOURCE) $(TLMDBH) 
EXTRADOC+=$(TLMDBSOURCE) $(TLMDBH)
EXTRACLEAN+=$(TLMDBSOURCE:.cpp=.o64)

extract_str.INC: makePacketInclude.exe TelemetryDatabase.csv
	./makePacketInclude.exe TelemetryDatabase.csv

../libraries/tlmDb/extractPacket.o64: ../libraries/tlmDb/extractPacket.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
../libraries/tlmDb/makePacketInclude.o64: ../libraries/tlmDb/makePacketInclude.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
../libraries/tlmDb/tlmDb.o64: ../libraries/tlmDb/tlmDb.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
../libraries/tlmDb/csv.o64: ../libraries/tlmDb/csv.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
extractPacket.exe: ../libraries/tlmDb/extractPacket.o64 ../libraries/tlmDb/tlmDb.o64 ../libraries/tlmDb/csv.o64  
	g++ -g -O0 -o $@ $^ 
	
makePacketInclude.exe: ../libraries/tlmDb/makePacketInclude.o64 ../libraries/tlmDb/tlmDb.o64 ../libraries/tlmDb/csv.o64
	g++ -g -O0 -o $@ $^ 

%.csv: %.ods
	DISPLAY= ssconvert $^ $@ 