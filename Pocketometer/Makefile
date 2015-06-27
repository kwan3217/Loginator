PROJECT_BRIEF="Connect with and communicate with 11DoF sensors"
HARDWARE=LOGINATOR
#OPTLEVEL=3
LD_SCRIPT=../libraries/System/lowmem_divided.ld
ATTACH=Loginator.brd Loginator.sch 11Dof.brd 11Dof.sch GP-2106_Eval-v11.brd GP-2106_Eval-v11.sch gpl-3.0.html CONFIG.TXT mpu-9150_breakout.brd mpu-9150_breakout.sch
include ../libraries/LPCduino/Makefile
include ../libraries/PWM/Makefile
include ../libraries/mpu60x0/Makefile
include ../libraries/ak8975/Makefile
include ../libraries/bmp180/Makefile
include ../libraries/Serial/Makefile
include ../libraries/sdhc/Makefile
include ../libraries/fat/Makefile
include ../libraries/FileCircular/Makefile
include ../libraries/Wire/StateTwoWire/Makefile
include ../libraries/gps/nmea/Makefile
include ../libraries/config/Makefile
include ../libraries/System/Makefile
USB = /dev/sdc1

format:
	mount | grep $(USB)
	echo About to format $(USB), make sure this is the one you want then hit enter to proceed or ^C to cancel
	read junk
	sudo umount -f $(USB) ; sudo mkfs.vfat -v -s 128 -f 1 -F 32 -n Pocketometr $(USB)
	sudo fsck -v $(USB)
	sudo mount $(USB) /mnt/usb
	sudo cp CONFIG.TXT /mnt/usb
	sleep 1
	sudo umount -f $(USB)

