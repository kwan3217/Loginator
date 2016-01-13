SIMOBJEXT=o64
LIBMAKE+=../libraries/sim/Makefile
ATTACH+=../libraries/sim/SimSd.dot

#Simulation harness
SIMCPPSRC+=../libraries/sim/LPC214x.cpp
SIMCPPSRC+=../libraries/sim/SimGpio.cpp
SIMCPPSRC+=../libraries/sim/SimI2c.cpp
SIMCPPSRC+=../libraries/sim/SimId.cpp
SIMCPPSRC+=../libraries/sim/SimScb.cpp
SIMCPPSRC+=../libraries/sim/SimSd.cpp
SIMCPPSRC+=../libraries/sim/SimSpi.cpp
SIMCPPSRC+=../libraries/sim/SimSsp.cpp
SIMCPPSRC+=../libraries/sim/SimUart.cpp
SIMCPPSRC+=../libraries/sim/SimTimer.cpp
SIMCPPSRC+=../libraries/sim/dprintf.cpp

CLANGINCLUDE=-I ../libraries/sim -I . $(addprefix -I ,$(call uniq,$(EXTRAINCDIRS))) -I /usr/include/c++/4.9 -I /usr/include/x86_64-linux-gnu/c++/4.9/
CLANGCOMP=-g -O0 -std=c++1y -c -save-temps -o $@           $< $(CLANGINCLUDE) -Wno-ignored-attributes -MMD -MP -MF .dep/$(@F).d
CLANGLINK=-g -O0 -o $@ -lm -lstdc++ $^ -L/usr/lib/gcc/x86_64-linux-gnu/5.2.1/ -L/usr/lib/gcc/x86_64-linux-gnu/4.9/ -Wl,-Map,$(@:.exe=.map) -Wno-ignored-attributes

#Rules to compile source into .o64 files
$(SIMCPPSRC:.cpp=.$(SIMOBJEXT)) $(CPPSRC:.cpp=.$(SIMOBJEXT)): %.$(SIMOBJEXT): %.cpp
	g++ $(CLANGCOMP) 

$(GENCPP:.CPP=.$(SIMOBJEXT)) : %.$(SIMOBJEXT): %.CPP
	g++ $(CLANGCOMP) 

#Simulated SD card stuff

#This uses DD to make a sparse output file, as described in 
#http://prefetch.net/blog/index.php/2009/07/05/creating-sparse-files-on-linux-hosts-with-dd/
sim/sdcard:
	dd if=/dev/zero of=$@ bs=1 count=0 seek=4096M
	mkfs.vfat -v -s 128 -f 1 -F 32 -n YukariSim $@

sdcard:
	mkdir sdcard

mount: sim/sdcard sdcard
	sudo mount sim/sdcard sdcard/ || true

umount:
	sudo umount sdcard/ || true

#Program annotated listing
%.lss64: %.exe
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	objdump -h $< > $@
	objdump -S -j .text $< |tail -n +4 >> $@
	objdump -s -j .vtable -j .rodata_str -j .rodata -j .ctors $< | tail -n +4 >> $@
	objcopy -O binary -j .rope $< $<.tmp
	if [ -s $<.tmp ] ;then  echo "Disassembly of section .rope:" >> $@ ; perl ../libraries/agc/agc_disasm.pl $<.tmp `$(OBJDUMP) -s -j .rope $< | head -n 5 | tail -n 1` >> $@ ; fi
	$(RM) -f $<.tmp 
	objcopy -O binary -j .source $< $<.tmp.zpaq
	if [ -s $<.tmp.zpaq ] ;then  echo "Source tarball contents:" >> $@ ; zpaq110 xn $<.tmp.zpaq $<.tmp.cpio ; cpio -ivt < $<.tmp.cpio >> $@ ; $(RM) $<.tmp.cpio; fi
	$(RM) -f $<.tmp.zpaq $<.tmp.cpio
	objdump -s                        -j .data $< | tail -n +4 >> $@
	objdump -S -j .text_lib $< |tail -n +4 >> $@
	objdump -s -j .ARM.exidx -j .ARM.extab -j .glue -j .vtable_lib -j .rtti_info -j .rtti_name -j .rodata_str_lib -j .rodata_lib -j .ctors_lib $< | tail -n +4 >> $@
	objdump -s                        -j .data_lib $< | tail -n +4 >> $@
	objdump -t $< | grep ^[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f] | sort >> $@

