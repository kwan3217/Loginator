# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp)
# Every Loginator sketch uses its own main.cpp, so we reference it here.
#CPPSRC += ../libraries/tarball/tarball.cpp
LIBMAKE+=../libraries/tarball/Makefile
EXTRAINCDIRS+=../libraries/tarball/
ZPAQDIR=../libraries/tarball

HDEP = $(subst sinShort.h,,$(shell cat .dep/* | sed -n 's/\([^:]*.h\)\s*:/\1/gp' | sort | uniq))
INCDEP = $(subst sinShort.h,,$(shell cat .dep/* | sed -n 's/\([^:]*.inc\)\s*:/\1/gp' | sort | uniq))
ALLSRC = $(SRC) $(SRCARM) $(CPPSRC) $(CPPSRCARM) $(ASRC) $(ASRCARM) $(PERL)
ATTACH+= $(ALLSRC) $(HDEP) $(INCDEP) $(LD_SCRIPT) $(LIBMAKE) ../libraries/System/Doxyfile.template $(ZPAQDIR)/max.cfg gccversion.txt
SATTACH=$(sort $(ATTACH))
DDATTACH=$(addprefix $(TARGETBASE)/,$(SATTACH))
CTARBALL = source.cpio.max1.zpaq
SOURCEOBJ = $(CTARBALL).$(OBJEXT)
ifndef TESTCPIOOBJ
TESTCPIOOBJ=$(TARGET).elf
endif

testcpio: $(TESTCPIOOBJ)
	$(OBJCOPY) -O binary -j .source $< test.cpio.max.zpaq
	zpaq xn test.cpio.max.zpaq  test.cpio
	rm -rf cpio
	mkdir cpio
	(cd cpio; cpio -ivd < ../test.cpio)

tarball: $(CTARBALL)

sourceobj: $(SOURCEOBJ)

gccversion.txt:
	echo == C compiler version == > $@
	$(CC) --version --verbose >> $@ 2>&1
	echo >> $@
	echo == C++ compiler version == >> $@
	$(CPP) --version --verbose >> $@ 2>&1
	echo >> $@
	echo == Perl version == >> $@
	perl --version -v >> $@
	echo >> $@
	echo == ragel version == >> $@
	ragel --version >> $@
	echo >> $@
	echo == ssconvert version == >> $@
	DISPLAY= ssconvert --version >> $@

source.tar: $(SATTACH)
	tar chvf $@ -C .. $(DDATTACH)

%.cpio: %.tar
	cat $< | (mkdir /tmp/$<; cd /tmp/$<; tar x; (find . -type f)|cpio -oL )> $@
	rm -rf /tmp/$< 
#	(cd ..; (for i in $(DDATTACH); do echo $$i; done) | cpio -ovL > $(TARGETBASE)/$@)

%.lr: %
	lrzip -n $< -S .lr

%.xz: %
	xz -kf $<

%.bz2: %
	bzip2 -c1 $< > $@ 

%.bz9: %
	bzip2 -c9 $< > $@

%.Z: %
	compress -c < $< > $@

%.gz: %
	gzip < $< > $@

%.gz9: %
	gzip -9 < $< > $@

%.lzo: %
	lzop -f $<

%.lz: %
	lzip -kf $<

%.zip: %
	$(RM) $@
	zip $@ $< 

%.7z: %
	$(RM) $@
	7z a $@ $< 

%.mid1.zpaq: %
	$(RM) $@
	zpaq110 tc$(ZPAQDIR)/mid.cfg $@ $<

%.max1.zpaq: %
	$(RM) $@
	zpaq110 tc$(ZPAQDIR)/max.cfg $@ $<

%.mid6.zpaq: %
	$(RM) $@
	zpaq660 a $@ $<

%.max6.zpaq: %
	$(RM) $@
	zpaq660 a $@ $< -method 6

%.cfg.fragile.zpaq: %
	$(RM) $@
	zpaq660 a $@ $< -method x7.0f$(ZPAQDIR)/max -fragile

%.max6.fragile.zpaq: %
	$(RM) $@
	zpaq660 a $@ $< -method 6 -fragile

ARC=.cpio .tar
FILT=.nofilt .lr
COMP=.xz .bz2 .bz9 .Z .gz .gz9 .lzo .lz .zip .7z .mid1.zpaq .max1.zpaq .mid6.zpaq .max6.zpaq .cfg.fragile.zpaq .max6.fragile.zpaq
AF=$(subst .nofilt,,$(foreach j,$(ARC),$(addprefix $j,$(FILT))))
AFC=$(foreach j,$(AF),$(addprefix $j,$(COMP)))
TEST_ARCHIVE = $(addprefix source,$(AF)) $(addprefix source,$(AFC))

print_test_archive:
	@echo All archivers:   $(ARC)
	@echo All filters:     $(FILT)
	@echo All compressors: $(COMP)
	@echo All archive+filter: $(AF)
	@echo All archive+filter+comp: $(AFC)
	@echo Test archive: $(TEST_ARCHIVE)

test_archive: $(TEST_ARCHIVE)

ifndef BFDO
BFDO=elf32-littlearm 
BFDB=arm
endif

$(SOURCEOBJ): $(CTARBALL)
	$(OBJCOPY) -I binary -O $(BFDO) $< $@ --rename-section .data=.source -B $(BFDB)

