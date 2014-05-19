TPKNAME = tpk
LWMNAME = lwm
MAPNAME = map
CSCNAME = csc
CPKNAME = cpk
STSNAME = cpke
CCDNAME = ccd
CPPFLAGS = -g -Wall -pedantic -W -ggdb -O3
# OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.c,%.o,$(wildcard ./executables/*.c))
# TPKOBJECTS = $(filter-out ./executables/cpkExtendedMain.o ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/levelwiseMain.o ./executables/map2gaston.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
# LWMOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/main.o ./executables/map2gaston.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
# MAPOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/main.o ./executables/levelwiseMain.o ./executables/countSpanningTreeClasses.o, $(OBJECTS))
# CSCOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/cpkMain.o ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))
# CPKOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/countSpanningTreeClasses.o ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))
# CPKEOBJECTS = $(filter-out ./executables/countCycleDegree.o ./executables/countSpanningTreeClasses.o ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))
# CCDOBJECTS = $(filter-out ./executables/cpkMain.o  ./executables/countSpanningTreeClasses.o ./executables/main.o ./executables/map2gaston.o ./executables/levelwiseMain.o, $(OBJECTS))
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
TPKOBJECTS = $(OBJECTS) ./executables/main.o
LWMOBJECTS = $(OBJECTS) ./executables/levelwiseMain.o
MAPOBJECTS = $(OBJECTS) ./executables/map2gaston.o
CSCOBJECTS = $(OBJECTS) ./executables/countSpanningTreeClasses.o
CPKOBJECTS = $(OBJECTS) ./executables/cpkMain.o
STSOBJECTS = $(OBJECTS) ./executables/spanningTreeSamplingMain.o
CCDOBJECTS = $(OBJECTS) ./executables/countCycleDegree.o

main: cpk
	

all: tpk lwm map csc cpk sts
	
cpk: $(CPKOBJECTS)
	@echo "\nLink Cyclic Pattern Kernel executable:"
	gcc -o $(CPKNAME) $(CPKOBJECTS) $(CPPFLAGS)

sts: $(STSOBJECTS)
	echo "Link Spanning Tree Sampling executable:"
	gcc -o $(STSNAME) $(STSOBJECTS) $(CPPFLAGS)

tpk: $(TPKOBJECTS)
	@echo "\nLink Tree Pattern Kernel executable:"
	gcc -o $(TPKNAME) $(TPKOBJECTS) $(CPPFLAGS)

lwm: $(LWMOBJECTS)
	@echo "\nLink Levelwise Mining executable:"
	gcc -o $(LWMNAME) $(LWMOBJECTS) $(CPPFLAGS)

map: $(MAPOBJECTS)
	@echo "\nLink Data Transformer executable:"
	gcc -o $(MAPNAME) $(MAPOBJECTS) $(CPPFLAGS)

csc: $(CSCOBJECTS)
	@echo "\nLink Count Graph Properties executable:"
	gcc -o $(CSCNAME) $(CSCOBJECTS) $(CPPFLAGS)

ccd: $(CCDOBJECTS)
	@echo "\nLink Count Graph Properties executable:"
	gcc -o $(CCDNAME) $(CCDOBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm ./executables/*.o
