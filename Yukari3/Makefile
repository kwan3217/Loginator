PROJECT_BRIEF="Project Yukari 3.0, entry into AVC 2015"
HARDWARE=LOGINATOR
#ATTACH=Loginator.brd Loginator.sch
#ATTACH+=11Dof.brd 11Dof.sch
#ATTACH+=GP-2106_Eval-v11.brd GP-2106_Eval-v11.sch
#ATTACH+=gpl-3.0.html
#ATTACH+=CONFIG.TXT.head
#ATTACH+=sim/Makefile $(wildcard sim/*.cpp) $(wildcard sim/*.h) 
#ATTACH+=Yukari3.fzz
#ATTACH+=AVC2015.kml
#ATTACH+=parsekml.pl
CPPSRC+=navigate.cpp guide.cpp control.cpp
EXTRACLEAN+=CONFIG.TXT
include ../libraries/LPCduino/Makefile
include ../libraries/PWM/Makefile
include ../libraries/hmc5883/Makefile
include ../libraries/l3g4200d/Makefile
include ../libraries/Serial/Makefile
include ../libraries/sdhc/Makefile
include ../libraries/fat/Makefile
include ../libraries/FileCircular/Makefile
include ../libraries/Wire/StateTwoWire/Makefile
include ../libraries/gps/nmea/Makefile
include ../libraries/config/Makefile
include ../libraries/quaternion/Makefile
include ../libraries/System/Makefile
USB = /dev/sdd1
COURSE=Long Course

main.cpp: extract_str.inc

simall: simclean clean playback simhost extractPacket

format: CONFIG.TXT
	df $(USB)
	sudo fsck -vn $(USB) || true
	echo About to format $(USB)! Is this really what you want? Press enter to continue, ^C to abort
	read junk
	sudo umount -f $(USB) ; sudo mkfs.vfat -v -s 128 -f 1 -F 32 -n Yukari $(USB)
	sudo fsck -v $(USB)
	sudo mount $(USB) /mnt/usb
	sudo cp CONFIG.TXT /mnt/usb
	sleep 1
	sudo umount -f $(USB)

simhost: mount CONFIG.TXT source.o64
	make -f sim/Makefile simhost
	sudo cp -v CONFIG.TXT sdcard
	make umount

playback: 
	make -f sim/Makefile playback

simclean:
	make -f sim/Makefile clean

source.o64: source.cpio.max1.zpaq
	objcopy -I binary -O elf64-x86-64 $< $@ --rename-section .data=.source -B i386:x86-64

mount: sim/sdcard sdcard
	sudo mount sim/sdcard sdcard/ || true

sim/sdcard: 
	make -f sim/Makefile sim/sdcard

sdcard:
	mkdir sdcard

umount:
	sudo umount sdcard/ || true

.PHONY: CONFIG.TXT

CONFIG.TXT: AVC2015.kml CONFIG.TXT.head
	perl parsekml.pl AVC2015.kml "$(COURSE)" | cat CONFIG.TXT.head - > $@

