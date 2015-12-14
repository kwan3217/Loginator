SIMOBJEXT=o64
LIBMAKE+=../libraries/sim/Makefile

#Simulation harness
SIMCPPSRC+=../libraries/sim/LPC214x.cpp
SIMCPPSRC+=../libraries/sim/SimUart.cpp
SIMCPPSRC+=../libraries/sim/SimGpio.cpp
SIMCPPSRC+=../libraries/sim/SimScb.cpp
SIMCPPSRC+=../libraries/sim/SimSpi.cpp
SIMCPPSRC+=../libraries/sim/SimSd.cpp

CLANGINCLUDE=-I ../libraries/sim -I . $(addprefix -I ,$(call uniq,$(EXTRAINCDIRS))) -I /usr/include/c++/4.9 -I /usr/include/x86_64-linux-gnu/c++/4.9/
CLANGE   =-g -O0 -std=c++1y -E -o $(<:.cpp=.e) $< $(CLANGINCLUDE) -Wno-ignored-attributes
CLANGCOMP=-g -O0 -std=c++1y -c -o $@           $< $(CLANGINCLUDE) -Wno-ignored-attributes
CLANGLINK=-g -O0 -o $@ -lm -lstdc++ $^ -L/usr/lib/gcc/x86_64-linux-gnu/5.2.1/ -L/usr/lib/gcc/x86_64-linux-gnu/4.9/ -Wno-ignored-attributes

#Rules to compile source into .o64 files
$(SIMCPPSRC:.cpp=.$(SIMOBJEXT)) $(CPPSRC:.cpp=.$(SIMOBJEXT)): %.$(SIMOBJEXT): %.cpp
	clang++ $(CLANGE) 
	clang++ $(CLANGCOMP) 

$(GENCPP:.CPP=.$(SIMOBJEXT)) : %.$(SIMOBJEXT): %.CPP
	clang++ $(CLANGE) 
	clang++ $(CLANGCOMP) 

#Simulated SD card stuff

sim/sdcard:
	dd if=/dev/zero of=$@ bs=1M count=4096
	mkfs.vfat -v -s 128 -f 1 -F 32 -n YukariSim $@

sdcard:
	mkdir sdcard

mount: sim/sdcard sdcard
	sudo mount sim/sdcard sdcard/ || true

umount:
	sudo umount sdcard/ || true


