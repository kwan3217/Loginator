PROJECT_BRIEF="Connect with and communicate with 11DoF sensors"
HARDWARE=LOGINATOR
ATTACH=Apid.ods Loginator.brd Loginator.sch 11Dof.brd 11Dof.sch
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
include ../libraries/System/Makefile

format:
	sudo umount -f /dev/sde1 ; sudo mkfs.vfat -v -s 128 -f 1 -F 32 -n Yukari /dev/sde1
	sudo fsck -v /dev/sde1

