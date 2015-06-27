PROJECT_BRIEF="Connect with and communicate with 11DoF sensors"
HARDWARE=LOGINATOR
OPT=0
#ATTACH=Loginator.brd Loginator.sch 11Dof.brd 11Dof.sch GP-2106_Eval-v11.brd GP-2106_Eval-v11.sch gpl-3.0.html CONFIG.TXT Rocketometer.brd Rocketometer.sch
include ../libraries/LPCduino/Makefile
include ../libraries/PWM/Makefile
include ../libraries/hmc5883/Makefile
include ../libraries/l3g4200d/Makefile
include ../libraries/mpu60x0/Makefile
include ../libraries/adxl345/Makefile
include ../libraries/ad799x/Makefile
include ../libraries/bmp180/Makefile
include ../libraries/Serial/Makefile
include ../libraries/sdhc/Makefile
include ../libraries/fat/Makefile
include ../libraries/FileCircular/Makefile
include ../libraries/Wire/StateTwoWire/Makefile
include ../libraries/gps/nmea/Makefile
include ../libraries/config/Makefile
include ../libraries/System/Makefile
USB = /dev/sdb1

format:
	sudo umount -f $(USB) ; sudo mkfs.vfat -v -s 128 -f 1 -F 32 -n Southwest $(USB)
	sudo fsck -v $(USB)
	sudo mount $(USB) /mnt/usb
	sudo cp CONFIG.TXT /mnt/usb
	sleep 1
	sudo umount -f $(USB)

