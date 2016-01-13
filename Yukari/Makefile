PROJECT_BRIEF="Project Yukari 4.0, entry into AVC 2016"
HARDWARE=LOGINATOR
ATTACH=Loginator.brd Loginator.sch
ATTACH+=11Dof.brd 11Dof.sch
ATTACH+=GP-2106_Eval-v11.brd GP-2106_Eval-v11.sch
ATTACH+=gpl-3.0.html
ATTACH+=CONFIG.TXT.head
ATTACH+=Yukari.fzz
ATTACH+=AVC2015.kml
ATTACH+=parsekml.pl

#Embedded files other than main.cpp
CPPSRC+=navigate.cpp guide.cpp control.cpp config.cpp

#Simulated Robot files
SIMCPPSRC+=sim/robot.cpp
SIMCPPSRC+=sim/playback.cpp
SIMCPPSRC+=sim/PlaybackGyro.cpp
SIMCPPSRC+=sim/PlaybackHmc.cpp
#SIMCPPSRC+=sim/simhost.cpp
HWDESCCSV=LoginatorHardware.csv LoginatorOnboard.csv 11DoF.csv GPS.csv Yukari.csv

EXTRACLEAN+=CONFIG.TXT
include ../libraries/LPCduino/Makefile
include ../libraries/PWM/Makefile
include ../libraries/sensors/hmc5883/Makefile
include ../libraries/sensors/l3g4200d/Makefile
include ../libraries/peripherals/Serial/Makefile
include ../libraries/file/sdhc/Makefile
include ../libraries/file/fat/Makefile
include ../libraries/file/FileCircular/Makefile
include ../libraries/peripherals/Wire/StateTwoWire/Makefile
include ../libraries/gps/ragel_nmea/Makefile
include ../libraries/config/Makefile
include ../libraries/float/quaternion/Makefile
include ../libraries/System/Makefile
include ../libraries/sim/Makefile
USB = /dev/sdd1
COURSE=Long Course

#Make the first file compiled (mentioned above) dependent on the packet parser stuff
navigate.cpp: extract_str.INC

playback.cpp: CONFIG.TXT

simall: clean playback simhost extractPacket

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

SIMOBJ = $(CPPSRC:.cpp=.$(SIMOBJEXT)) $(SIMCPPSRC:.cpp=.$(SIMOBJEXT)) $(GENCPP:.CPP=.$(SIMOBJEXT)) 

playback.exe: $(SIMOBJ) sim/playback.o64 sim/PlaybackHmc.o64 sim/PlaybackGyro.o64 source.cpio.max1.zpaq.o64
	clang++ $(CLANGLINK)
	make CONFIG.TXT
	make mount
	sudo cp -v CONFIG.TXT sdcard
	make umount

test_nmea: sim/test_nmea.o64 ../libraries/gps/ragel_nmea/nmea.o64 ../libraries/System/Stringex.o64 ../libraries/float/float.o64
	clang++ -g -O0 -o $@ -lm $^ -L/usr/lib/gcc/x86_64-linux-gnu/5.2.1/  -Wno-ignored-attributes

simhost.exe: $(SIMOBJ) sim/simhost.o64 source.cpio.max1.zpaq.o64
	clang++ -g -O0 -o $@ -lm $^ -L/usr/lib/gcc/x86_64-linux-gnu/5.2.1/ -Wno-ignored-attributes
	make CONFIG.TXT
	make mount
	sudo cp -v CONFIG.TXT sdcard
	make umount

.PHONY: CONFIG.TXT

CONFIG.TXT: AVC2015.kml CONFIG.TXT.head
	perl parsekml.pl AVC2015.kml "$(COURSE)" | cat CONFIG.TXT.head - > $@

