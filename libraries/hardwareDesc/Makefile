LIBMAKE+=../libraries/hardwareDesc/Makefile
EXTRAINCDIRS +=../libraries/hardwareDesc/
include ../libraries/dump/Makefile
include ../libraries/hostCsv/Makefile

HWDESCSOURCE+=../libraries/hardwareDesc/hardwareDesc.cpp ../libraries/Print/Print.cpp ../libraries/dump/dump.cpp
ATTACH+=$(HWDESCSOURCE) $(HWDESCCSV:.csv=.ods)
EXTRADOC+=$(HWDESCSOURCE)
EXTRACLEAN+=$(HWDESCSOURCE:.cpp=.o64) $(HWDESCCSV) hardwareDesc.H hardwareDesc.exe hardwareDesc.hex

../libraries/hardwareDesc/hardwareDesc.o64: $(HWDESCSOURCE)
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d

hardwareDesc.exe: ../libraries/hardwareDesc/hardwareDesc.o64 ../libraries/hostCsv/csv.o64 ../libraries/Print/Print.o64  ../libraries/dump/dump.o64
	g++ -g -O0 -o    $@ $^

hardwareDesc%hex hardwareDesc%H: $(HWDESCCSV) hardwareDesc.exe
	cat $(HWDESCCSV) | ./hardwareDesc.exe > hardwareDesc.hex
