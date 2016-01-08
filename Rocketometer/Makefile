MEDIA="/media/MICROSD 8GB"
ATTACH=Rocketometer.brd Rocketometer.sch Rocketometer.bom Rocketometer.docx Apid.ods

include ../libraries/peripherals/Serial/Makefile
include ../libraries/Task/Makefile
include ../libraries/peripherals/Wire/StateTwoWire/Makefile
include ../libraries/sensors/bmp180/Makefile
include ../libraries/sensors/ad799x/Makefile
include ../libraries/sensors/hmc5883/Makefile
include ../libraries/sensors/mpu60x0/Makefile
include ../libraries/gps/nmea/Makefile
include ../libraries/dump/Makefile
include ../libraries/packet/Makefile
include ../libraries/file/sdhc/Makefile
include ../libraries/file/FileCircular/Makefile
include ../libraries/System/Makefile

format:
	sudo umount -f /dev/sde1 ; sudo mkfs.vfat -v -s 128 -f 1 -F 32 -n RKTO3 /dev/sde1
	sudo fsck -v /dev/sde1

