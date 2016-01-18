PROJECT_BRIEF="Test matrix math, including Kalman filter"
ATTACH += 11Dof.brd 11Dof.sch Loginator.brd Loginator.sch GP-2106_Eval-v11.brd GP-2106_Eval-v11.sch 
SIMCPPSRC+=simMain.cpp
EXTRACLEAN+=simMain.exe simMain.map
include ../libraries/LPCduino/Makefile
include ../libraries/float/Makefile
include ../libraries/float/kalman/Makefile
include ../libraries/peripherals/Serial/Makefile
#include ../libraries/tarball/Makefile
include ../libraries/System/Makefile
include ../libraries/sim/Makefile

SIMOBJ = $(CPPSRC:.cpp=.$(SIMOBJEXT)) $(SIMCPPSRC:.cpp=.$(SIMOBJEXT)) $(GENCPP:.CPP=.$(SIMOBJEXT)) 

simMain.exe: $(SIMOBJ) simMain.o64 
	clang++ $(CLANGLINK)


