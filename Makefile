TPKNAME = tpk
LWMNAME = lwm
MTGNAME = mtg
MGGNAME = mgg
CSCNAME = csc
CPKNAME = cpk
STSNAME = ts
CCDNAME = ccd
TCINAME = tci
PERFNAME = perf
GFNAME = gf
CSTRNAME = cstring
TCIPERFNAME = tciperf

CPPFLAGS = -g -Wall -pedantic -W -ggdb -O3 -std=gnu99 -lm
EVERYTHING = $(wildcard *.c) $(wildcard ./executables/*.c)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
TPKOBJECTS = $(OBJECTS) ./executables/main.o
LWMOBJECTS = $(OBJECTS) ./executables/levelwiseMain.o
MTGOBJECTS = $(OBJECTS) ./executables/mapTrees2gaston.o
MGGOBJECTS = $(OBJECTS) ./executables/mapAIDS2gaston.o
CSCOBJECTS = $(OBJECTS) ./executables/countSpanningTreeClasses.o
CPKOBJECTS = $(OBJECTS) ./executables/cpkMain.o
STSOBJECTS = $(OBJECTS) ./executables/spanningTreeSamplingMain.o
CCDOBJECTS = $(OBJECTS) ./executables/countCycleDegree.o
GFOBJECTS = $(OBJECTS) ./executables/filter.o
CSTROBJECTS = $(OBJECTS) ./executables/cstring.o
TCIOBJECTS = $(OBJECTS) ./executables/cactus.o
PERFOBJECTS= $(OBJECTS) ./executables/tciPerf.o

# visualize the include dependencies between the source files.
# for this, .c and .h files with the same name are interpreted as one entity

dependencies.png: $(EVERYTHING)
	executables/cinclude2dot | sed 's/\.c/\.h/' | dot -Tpng -Gsize=40,40 > $@
	eog $@


main: sts
	

all: tpk lwm csc cpk sts mtg mgg stt gf cstr

gf: $(GFOBJECTS)
	@echo "\nLink Graph Filter executable:"
	gcc -o $(GFNAME) $(GFOBJECTS) $(CPPFLAGS)

cstr: $(CSTROBJECTS)
	@echo "\nLink Graph Filter executable:"
	gcc -o $(CSTRNAME) $(CSTROBJECTS) $(CPPFLAGS)

cpk: $(CPKOBJECTS)
	@echo "\nLink Cyclic Pattern Kernel executable:"
	gcc -o $(CPKNAME) $(CPKOBJECTS) $(CPPFLAGS)

sts: $(STSOBJECTS)
	@echo "Link Spanning Tree Sampling executable:"
	gcc -o $(STSNAME) $(STSOBJECTS) $(CPPFLAGS)

tpk: $(TPKOBJECTS)
	@echo "\nLink Tree Pattern Kernel executable:"
	gcc -o $(TPKNAME) $(TPKOBJECTS) $(CPPFLAGS)

lwm: $(LWMOBJECTS)
	@echo "\nLink Levelwise Mining executable:"
	gcc -o $(LWMNAME) $(LWMOBJECTS) $(CPPFLAGS)

mtg: $(MTGOBJECTS)
	@echo "\nLink Data Transformer executable:"
	gcc -o $(MTGNAME) $(MTGOBJECTS) $(CPPFLAGS)

mgg: $(MGGOBJECTS)
	@echo "\nLink Data Transformer executable:"
	gcc -o $(MGGNAME) $(MGGOBJECTS) $(CPPFLAGS)

csc: $(CSCOBJECTS)
	@echo "\nLink Count Graph Properties executable:"
	gcc -o $(CSCNAME) $(CSCOBJECTS) $(CPPFLAGS)

ccd: $(CCDOBJECTS)
	@echo "\nLink Count Graph Properties executable:"
	gcc -o $(CCDNAME) $(CCDOBJECTS) $(CPPFLAGS)

tci: $(TCIOBJECTS)
	@echo "\nLink Cactree Subgraph Isomorphism:"
	gcc -o $(TCINAME) $(TCIOBJECTS) $(CPPFLAGS)

perf: $(PERFOBJECTS)
	@echo "\nLink Cactree Subgraph Isomorphism Performance Test:"
	gcc -o $(PERFNAME) $(PERFOBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm ./executables/*.o
