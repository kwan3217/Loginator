LIBMAKE+=../libraries/hostCsv/Makefile
EXTRAINCDIRS +=../libraries/hostCsv/
ATTACH+=../libraries/hostCsv/csv.cpp ../libraries/hostCsv/csv.h 
EXTRADOC+=../libraries/hostCsv/csv.cpp ../libraries/hostCsv/csv.h
EXTRACLEAN+=../libraries/hostCsv/csv.o64

../libraries/hostCsv/csv.o64: ../libraries/hostCsv/csv.cpp
	g++ -g -O0 -c -o $@ $< -std=c++14 -I . $(addprefix -I ,$(EXTRAINCDIRS)) -MMD -MP -MF .dep/$(@F).d
	
%.csv: %.ods
	DISPLAY= ssconvert $^ $@ 