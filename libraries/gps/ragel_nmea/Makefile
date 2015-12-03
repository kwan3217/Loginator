include ../libraries/float/Makefile
LIBMAKE+=../libraries/gps/ragel_nmea/Makefile
EXTRAINCDIRS +=../libraries/gps/ragel_nmea/
ATTACH+=../libraries/gps/ragel_nmea/fixdot.pl
RAGEL += ../libraries/gps/ragel_nmea/nmea.rl
RAGEL_MODE = G0

%.CPP : %.rl
	ragel -$(RAGEL_MODE) -L -o $@ $<
	ragel -V -o $(<:.rl=.unfix.dot) $<
	perl ../libraries/gps/ragel_nmea/fixdot.pl < $(<:.rl=.unfix.dot) > $(<:.rl=.dot)
	dot -Tsvg $(<:.rl=.dot) > $(<:.rl=.svg)


