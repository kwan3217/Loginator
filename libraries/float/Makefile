LIBMAKE+=../libraries/float/Makefile
CPPSRC+=../libraries/float/float.cpp
EXTRAINCDIRS +=../libraries/float/
PERL = ../libraries/float/makeSinShort.pl
../libraries/float/float.o: sinShort.h

sinShort.h: $(PERL)
	perl $< > $@
