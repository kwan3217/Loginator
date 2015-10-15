LIBMAKE+=../libraries/float/Makefile
CPPSRC+=../libraries/float/float.cpp
EXTRAINCDIRS +=../libraries/float/

float.o64: float.cpp
	g++ -std=c++1y -O3 -o float.o64 -c float.cpp

testFloat.o64: testFloat.cpp
	g++ -std=c++1y -O3 -o testFloat.o64 -c testFloat.cpp

testFloat: float.o64 testFloat.o64
	g++ -o testFloat float.o64 testFloat.o64

clean64:
	rm float.o64 testFloat.o64 testFloat

