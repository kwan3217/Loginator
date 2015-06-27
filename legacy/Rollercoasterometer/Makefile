sinclude ../LPCduino/Makefile
IN2_COMMON = ..
LIBPATH = ../lib/
SCREENPATH = ../screens/
USBPATH = ../LPCUSB/

FLASH_TOOL = LPC12
#FLASH_TOOL = UVISION
#FLASH_TOOL = OPENOCD

# MCU name and submodel
MCU      = arm7tdmi-s
SUBMDL   = LPC2148
#THUMB    = -mthumb

#This is needed to run IAP function call stuff
THUMB_IW = -mthumb-interwork

## Create ROM-Image (final)
RUN_MODE=ROM_RUN
## Create RAM-Image (debugging) - not used in this example
#RUN_MODE=RAM_RUN

# not supported in this example:
## Exception-Vector placement only supported for "ROM_RUN"
## (placement settings ignored when using "RAM_RUN")
## - Exception vectors in ROM:
##VECTOR_LOCATION=VECTORS_IN_ROM
## - Exception vectors in RAM:
##VECTOR_LOCATION=VECTORS_IN_RAM

# Target file name (without extension).
TARGET = main

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
SRC += debug.c
SRC += armVIC.c
SRC += display.c
SRC += flash.c

#Needed most of the time for basic functions
SRC += $(LIBPATH)syscalls.c 
SRC += $(LIBPATH)serial.c 

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
ASRCARM = Startup.S
#ASRCARM = crt.S
#ASRCARM += $(WINARM_COMMON)\Common_WinARM/src/swi_handler.S


