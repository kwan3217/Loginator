MEDIA=/media/Arisa

# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp)
CPPSRC = main.cpp
CPPSRC += gps.cpp
CPPSRC += uart.cpp
CPPSRC += adc.cpp
CPPSRC += sdbuf.cpp
CPPSRC += conparse.cpp
CPPSRC += load.cpp
CPPSRC += setup.cpp
CPPSRC += loop.cpp
CPPSRC += command.cpp
CPPSRC += stringex.cpp
CPPSRC += circular.cpp
CPPSRC += sirfwrite.cpp
CPPSRC += nmeawrite.cpp
CPPSRC += pktwrite.cpp
CPPSRC += flash.cpp
CPPSRC += i2c_int.cpp
CPPSRC += spi.cpp
CPPSRC += compass.cpp
CPPSRC += gyro.cpp
CPPSRC += acc.cpp
CPPSRC += serial.cpp
CPPSRC += tictoc.cpp
CPPSRC += float.cpp
CPPSRC += matrix.cpp
CPPSRC += kalman.cpp
CPPSRC += imu.cpp
CPPSRC += script.cpp
CPPSRC += pinger.cpp
CPPSRC += sensor.cpp
CPPSRC += control.cpp
CPPSRC += motor.cpp
CPPSRC += guidance.cpp

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension cpp for C++-files (use extension .cpp)
#CPPSRCARM = $(TARGET).cpp
CPPSRCARM = 

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = 

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM = 
#ASRCARM = crt.S
#ASRCARM += $(WINARM_COMMON)\Common_WinARM/src/swi_handler.S

#We do this so that setup.c is compiled fresh each time. It has the build timestamp.
PHONY_TGT = setup.o FW.SFE
CDEFS = 
sinclude ../LPCduino/Makefile

