TPKNAME = tpk
LWMNAME = lwm
MAPNAME = map
CSCNAME = csc
CPPFLAGS = -g -Wall -pedantic -W -ggdb -O3
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.c,%.o,$(wildcard ./executables/*.c))
TPKOBJECTS = $(filter-out ./executables/levelwiseMain.o ./executables/map2gaston.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
LWMOBJECTS = $(filter-out ./executables/main.o ./executables/map2gaston.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
MAPOBJECTS = $(filter-out ./executables/main.o ./executables/levelwiseMain.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
CSCOBJECTS = $(filter-out ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))

main: map
	

all: main
	

tpk: $(TPKOBJECTS)
	gcc -o $(TPKNAME) $(TPKOBJECTS) $(CPPFLAGS)

lwm: $(LWMOBJECTS)
	gcc -o $(LWMNAME) $(LWMOBJECTS) $(CPPFLAGS)

map: $(MAPOBJECTS)
	gcc -o $(MAPNAME) $(MAPOBJECTS) $(CPPFLAGS)

csc: $(CSCOBJECTS)
	gcc -o $(CSCNAME) $(CSCOBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm ./executables/*.o
