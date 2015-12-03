#Simulated Robot files
CPPSRC+=sim/robot.cpp
#Robot code used directly
CPPSRC+=main.cpp navigate.cpp guide.cpp control.cpp 
#Sim replacements for library code
CPPSRC+=sim/sim_Serial.cpp sim/sim_gpio.cpp sim/sim_LPC214x.cpp sim/sim_pwm.cpp sim/sim_LPCduino.cpp sim/sim_Time.cpp
CPPSRC+=sim/sim_hmc5883.cpp sim/sim_l3g4200d.cpp 
CPPSRC+=sim/sim_HardSPI.cpp sim/sim_sdhc.cpp
CPPSRC+=sim/sim_StateTwoWire.cpp sim/sim_Wire.cpp
#Library code used directly
CPPSRC+=../libraries/packet/packet.cpp
CPPSRC+=../libraries/Partition/Partition.cpp ../libraries/fat/cluster.cpp ../libraries/fat/file.cpp ../libraries/fat/direntry.cpp
CPPSRC+=../libraries/Print/Print.cpp
CPPSRC+=../libraries/Circular/Circular.cpp
CPPSRC+=../libraries/gps/ragel_nmea/nmea.cpp
CPPSRC+=../libraries/System/Stringex.cpp
CPPSRC+=../libraries/quaternion/Quaternion.cpp
CPPSRC+=../libraries/FileCircular/FileCircular.cpp
CPPSRC+=../libraries/dump/dump.cpp
CPPSRC+=../libraries/config/readconfig.cpp
CPPSRC+=../libraries/l3g4200d/l3g4200d_common.cpp

INCLUDE=sim
INCLUDE+=.
INCLUDE+=../libraries/Serial
INCLUDE+=../libraries/Print 
INCLUDE+=../libraries/fat
INCLUDE+=../libraries/Partition
INCLUDE+=../libraries/packet
INCLUDE+=../libraries/Circular
INCLUDE+=../libraries/FileCircular
INCLUDE+=../libraries/dump
INCLUDE+=../libraries/tarball
INCLUDE+=../libraries/config
INCLUDE+=../libraries/gps/ragel_nmea
INCLUDE+=../libraries/quaternion
INCLUDE+=../libraries/PWM
INCLUDE+=../libraries/float
INCLUDE+=../libraries/LPCduino
INCLUDE+=../libraries/gpio
INCLUDE+=../libraries/Wire
INCLUDE+=../libraries/Wire/StateTwoWire
INCLUDE+=../libraries/hmc5883
INCLUDE+=../libraries/l3g4200d
INCLUDE+=../libraries/spi
INCLUDE+=../libraries/System
INCLUDE+=../libraries/sdhc

OBJEXT=o64
OBJ=$(CPPSRC:.cpp=.$(OBJEXT))
BFDB=i386:x86-64
BFDO=elf64-x86-64
OBJCOPY=objcopy
OBJDUMP=objdump

include ../libraries/tarball/Makefile
include ../libraries/packet/Makefile
include ../libraries/gps/ragel_nmea/Makefile

main.cpp: extract_str.inc 

sim/playback.cpp: extract_str.inc

../libraries/gps/ragel_nmea/nmea.o64: ../libraries/gps/ragel_nmea/nmea.CPP
	clang++ -g -O0 -std=c++1y -c -o $@ $< $(addprefix -I ,$(INCLUDE)) -Wno-ignored-attributes

%.o64: %.cpp
	clang++ -g -O0 -std=c++1y -c -o $@ $< $(addprefix -I ,$(INCLUDE)) -Wno-ignored-attributes

source.o64:
	make source.o64

simhost: sim/sim.o64 $(OBJ) source.o64
	clang++ -g -O0 -o $@ -lm $^ -L/usr/lib/gcc/x86_64-linux-gnu/4.8.2/ -Wno-ignored-attributes

playback: sim/playback.o64 $(OBJ) source.o64
	clang++ -g -O0 -o $@ -lm $^ -L/usr/lib/gcc/x86_64-linux-gnu/4.8.2/ -Wno-ignored-attributes

test_nmea: sim/test_nmea.o64 ../libraries/gps/ragel_nmea/nmea.o64 ../libraries/System/Stringex.o64 ../libraries/float/float.o64
	clang++ -g -O0 -o $@ -lm $^ -L/usr/lib/gcc/x86_64-linux-gnu/4.8.2/ -Wno-ignored-attributes

clean:
	$(RM) $(OBJ) simhost playback source.o64 sim/sim.064 sim/playback.o64 sim/test_nmea.o64 ../libraries/gps/ragel_nmea/nmea.CPP *~

sim/sdcard:
	dd if=/dev/zero of=$@ bs=1M count=4096
	mkfs.vfat -v -s 128 -f 1 -F 32 -n YukariSim $@


