LIBMAKE+=../libraries/agc/Makefile
CPPSRC+=../libraries/agc/agc_engine.cpp ../libraries/agc/agc_instr.cpp ../libraries/agc/agc_io.cpp
EXTRAINCDIRS +=../libraries/agc/
ROPESOURCE:=$(wildcard $(ROPE)/*.agc)
ATTACH+=$(ROPESOURCE) ../libraries/agc/agc_embedrope.pl ../libraries/agc/agc_disasm.pl ../libraries/agc/agc_map.pl
ifndef ROPEEXT
ROPEEXT=o
endif

#For this to work, yaYUL must be in the path. Build it using its Makefile
#cd workspace/Data/agc/yaYUL
#make
#sudo make install
$(ROPE).rope: $(ROPESOURCE)
	(cd $(ROPE); yaYUL --html $(ROPEROOT).agc > /dev/null)
	mv $(ROPE)/$(ROPEROOT).agc.bin $@

rope.bin: $(ROPE).rope
	pwd
	perl ../libraries/agc/agc_embedrope.pl $< > $@

$(ROPE).disasm: rope.bin
	perl ../libraries/agc/agc_disasm.pl $< > $@

$(ROPE).map: rope.bin
	perl ../libraries/agc/agc_map.pl $(ROPE) > $@ 2> rope.fullmap

rope.fullmap: $(ROPE).map

rope.bin.$(ROPEEXT): rope.bin $(ROPE).disasm $(ROPE).map
	$(OBJCOPY) -I binary -B $(BFDB) -O $(BFDO) --rename-section .data=.rope $< $@

EXTRAO+=rope.bin.$(ROPEEXT)

EXTRACLEAN+=$(ROPE).rope rope.bin $(ROPE).disasm $(ROPE).map rope.fullmap rope.bin.$(ROPEEXT) $(wildcard $(ROPE)/*.html) $(ROPE)/$(ROPE).agc.symtab 
