CC=g++ -Wall -Wextra -O3
CFLAGS= -Wl,--no-as-needed,-lTMVA
LDFLAGS=`root-config --glibs --cflags`
SOURCES= fillnanogen.cc ../analysis/src/systematicTools.cc ../eventselection/src/eventSelectionsParticleLevel.cc ../eventselection/src/eventFlatteningParticleLevel.cc ../../codeLibrary.o 
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=fillnanogen

all: 
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(EXECUTABLE)
        
clean:
	rm -rf *o $(EXECUTABLE)
