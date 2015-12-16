#Robot code used directly
CPPSRC+=main.cpp navigate.cpp guide.cpp control.cpp config.cpp
#Library code used directly
CPPSRC+=../libraries/packet/ccsds/ccsds.cpp
CPPSRC+=../libraries/Circular/Circular.cpp
CPPSRC+=../libraries/gps/ragel_nmea/nmea.cpp
CPPSRC+=../libraries/System/Stringex.cpp
CPPSRC+=../libraries/float/quaternion/Quaternion.cpp
CPPSRC+=../libraries/config/readconfig.cpp
CPPSRC+=../libraries/peripherals/gpio/gpio.cpp
CPPSRC+=../libraries/peripherals/Serial/Serial.cpp
CPPSRC+=../libraries/peripherals/spi/HardSPI.cpp
CPPSRC+=../libraries/PWM/pwm.cpp
CPPSRC+=../libraries/LPCduino/LPCduino.cpp
#Put these first so that sim files can replace embedded files when needed
INCLUDE=sim
INCLUDE+=../libraries/sim
INCLUDE+=.
INCLUDE+=../libraries/Print 
INCLUDE+=../libraries/file/fat
INCLUDE+=../libraries/file/Partition
INCLUDE+=../libraries/file/sdhc
INCLUDE+=../libraries/file/FileCircular
INCLUDE+=../libraries/packet
INCLUDE+=../libraries/packet/ccsds
INCLUDE+=../libraries/Circular
INCLUDE+=../libraries/dump
INCLUDE+=../libraries/tarball
INCLUDE+=../libraries/config
INCLUDE+=../libraries/gps/ragel_nmea
INCLUDE+=../libraries/PWM
INCLUDE+=../libraries/float
INCLUDE+=../libraries/float/vector
INCLUDE+=../libraries/float/quaternion
INCLUDE+=../libraries/LPCduino
INCLUDE+=../libraries/peripherals/gpio
INCLUDE+=../libraries/peripherals/Serial
INCLUDE+=../libraries/peripherals/spi
INCLUDE+=../libraries/peripherals/Wire
INCLUDE+=../libraries/peripherals/Wire/StateTwoWire
INCLUDE+=../libraries/sensors/hmc5883
INCLUDE+=../libraries/sensors/l3g4200d
INCLUDE+=../libraries/System

include ../libraries/tarball/Makefile
include ../libraries/packet/Makefile
include ../libraries/gps/ragel_nmea/Makefile

