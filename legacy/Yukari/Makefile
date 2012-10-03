MEDIA=/media/Arisa

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
SRC = main.c
SRC += gps.c
SRC += uart.c
SRC += adc.c
SRC += sdbuf.c
SRC += conparse.c
SRC += load.c
SRC += setup.c
SRC += loop.c
SRC += command.c
SRC += stringex.c
SRC += circular.c
SRC += sirfwrite.c
SRC += nmeawrite.c
SRC += pktwrite.c
SRC += flash.c
SRC += i2c.c
SRC += spi.c
SRC += compass.c
SRC += gyro.c
SRC += acc.c
SRC += serial.c 
SRC += tictoc.c
SRC += float.c
SRC += matrix.c
SRC += kalman.c
SRC += IMU.c
SRC += script.c
SRC += pinger.c
SRC += sensor.c
SRC += control.c
SRC += motor.c
SRC += guidance.c

#Needed most of the time for basic functions
SRC += $(LIBPATH)syscalls.c 

#Functions for SD interaction
SRC += $(LIBPATH)rootdir.c 
SRC += $(LIBPATH)sd_raw.c 
SRC += $(LIBPATH)fat16.c 
SRC += $(LIBPATH)partition.c 
SRC += $(LIBPATH)itoa.c

# List C source files here which must be compiled in ARM-Mode.
# use file-extension c for "c-only"-files
SRCARM = $(LIBPATH)irq.c

# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp)
CPPSRC = 

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
