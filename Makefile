TPKNAME = tpk
LWMNAME = lwm
MAPNAME = map
CSCNAME = csc
CPKNAME = cpk
CCDNAME = ccd
CPPFLAGS = -g -Wall -pedantic -W -ggdb -O3
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.c,%.o,$(wildcard ./executables/*.c))
TPKOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/levelwiseMain.o ./executables/map2gaston.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
LWMOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/main.o ./executables/map2gaston.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
MAPOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/main.o ./executables/levelwiseMain.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
CSCOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))
CPKOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/countSpanningTreeClasses.o ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))
CCDOBJECTS = $(filter-out ./executables/cpkMain.o  ./executables/countSpanningTreeClasses.o ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))

main: map
	

all: tpk lwm map csc cpk
	
cpk: $(CPKOBJECTS)
	echo "Link Cyclic Pattern Kernel executable:"
	gcc -o $(CPKNAME) $(CPKOBJECTS) $(CPPFLAGS)

tpk: $(TPKOBJECTS)
	echo "Link Tree Pattern Kernel executable:"
	gcc -o $(TPKNAME) $(TPKOBJECTS) $(CPPFLAGS)

lwm: $(LWMOBJECTS)
	echo "Link Levelwise Mining executable:"
	gcc -o $(LWMNAME) $(LWMOBJECTS) $(CPPFLAGS)

map: $(MAPOBJECTS)
	echo "Link Data Transformer executable:"
	gcc -o $(MAPNAME) $(MAPOBJECTS) $(CPPFLAGS)

csc: $(CSCOBJECTS)
	echo "Link Count Graph Properties executable:"
	gcc -o $(CSCNAME) $(CSCOBJECTS) $(CPPFLAGS)

ccd: $(CCDOBJECTS)
	echo "Link Count Graph Properties executable:"
	gcc -o $(CCDNAME) $(CCDOBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm ./executables/*.o
