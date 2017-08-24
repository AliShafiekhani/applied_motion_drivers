CC             = gcc
CPP            = g++
MAKE           = make
MAKEFILE       = Makefile

CFLAGS         = -O0 -Wall
EFILE          = driver_test
INCLS          = -I/usr/local/include -I/usr/local/include -Iinclude
LIBS           = -L/usr/local/lib -lserial

$(EFILE): main.o applied_motion_drivers.o $(MAKEFILE)
	$(CPP) $(CFLAGS) -o $(EFILE) main.o applied_motion_drivers.o $(LIBS)

main.o: src/main.cpp src/applied_motion_drivers.cpp $(MAKEFILE)
	$(CPP) $(CFLAGS) $(INCLS) -c src/main.cpp

applied_motion_drivers.o: src/applied_motion_drivers.cpp $(MAKEFILE)
	$(CPP) $(CFLAGS) $(INCLS) -c src/applied_motion_drivers.cpp

clean:
	\rm -f *.o *~ $(EFILE)
