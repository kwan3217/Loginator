LIBMAKE+=../libraries/LPC214x/Makefile
EXTRAINCDIRS +=../libraries/LPC214x/
include ../libraries/tlmDb/Makefile

HWDESCSOURCE+=../libraries/LPC214x/hardwareDesc.cpp
ATTACH+=$(HWDESCSOURCE) $(HWDESCCSV:.csv=.ods)
EXTRADOC+=$(HWDESCSOURCE)
EXTRACLEAN+=$(HWDESCSOURCE:.cpp=.o64)

../libraries/LPC214x/hardwareDesc.o64: $(HWDESCSOURCE)
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d

hardwareDesc.exe: ../libraries/LPC214x/hardwareDesc.o64 ../libraries/tlmDb/csv.o64  
	g++ -g -O0 -o    $@ $^ 

hardwareDesc.hex: $(HWDESCCSV) hardwareDesc.exe
	cat $(HWDESCCSV) | ./hardwareDesc.exe > $@
