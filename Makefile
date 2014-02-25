CFLAGS=-g -std=gnu99 -Wall -I. -DUSE_TIMEPRO
CXXFLAGS=-g -Wall -I. -DUSE_TIMEPRO
LDFLAGS=-g -lpthread -lrt -ldl -lstdc++ -Wl,--export-dynamic

all: test_timepro test_instrument test_cplusplus

test_timepro: test_timepro.o libtimepro.a
	$(CC) $(CFLAGS) -o $@ -L. test_timepro.o -ltimepro $(LDFLAGS)

libtimepro.a: timepro.o platform.o instrument.o timeprocpp.o
	$(AR) rcs libtimepro.a timepro.o timeprocpp.o platform.o instrument.o

test_instrument: test_instrument.c libtimepro.a
	$(CC) $(CFLAGS) -finstrument-functions -o $@ -L. test_instrument.c -ltimepro $(LDFLAGS)

test_cplusplus: test_cplusplus.cc timepro.hpp
	g++ $(CXXFLAGS) -finstrument-functions -o $@ -L. test_cplusplus.cc -ltimepro $(LDFLAGS)

test_timepro.o: test_timepro.c

timepro.o: timepro.h timepro_private.h timepro.c

timeprocpp.o: timepro.hpp timeprocpp.cpp timepro.h

platform.o: platform.c platform.h

instrument.o: instrument.c timepro.h timepro_private.h

clean:
	rm -rvf test_timepro test_cplusplus test_instrument *.o libtimepro.a
