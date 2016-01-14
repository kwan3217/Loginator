PROJECT_BRIEF="Test matrix math, including Kalman filter"

SIMCPPSRC+=simMain.cpp
EXTRACLEAN+=simMain.exe simMain.map
include ../libraries/LPCduino/Makefile
include ../libraries/float/Makefile
include ../libraries/peripherals/Serial/Makefile
include ../libraries/System/Makefile
include ../libraries/sim/Makefile

SIMOBJ = $(CPPSRC:.cpp=.$(SIMOBJEXT)) $(SIMCPPSRC:.cpp=.$(SIMOBJEXT)) $(GENCPP:.CPP=.$(SIMOBJEXT)) 

simMain.exe: $(SIMOBJ) simMain.o64
	clang++ $(CLANGLINK)


