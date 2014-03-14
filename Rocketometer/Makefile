MEDIA="/media/MICROSD 8GB"
ATTACH=Rocketometer.brd Rocketometer.sch Rocketometer.bom Rocketometer.docx Apid.ods

include ../libraries/Serial/Makefile
include ../libraries/Task/Makefile
include ../libraries/Wire/StateTwoWire/Makefile
include ../libraries/bmp180/Makefile
include ../libraries/ad799x/Makefile
include ../libraries/hmc5883/Makefile
include ../libraries/mpu60x0/Makefile
include ../libraries/gps/Makefile
include ../libraries/dump/Makefile
include ../libraries/packet/Makefile
include ../libraries/fat/Makefile
include ../libraries/FileCircular/Makefile
include ../libraries/System/Makefile

format:
	sudo umount -f /dev/sde1 ; sudo mkfs.vfat -v -s 128 -f 1 -F 32 -n RKTO3 /dev/sde1
	sudo fsck -v /dev/sde1

