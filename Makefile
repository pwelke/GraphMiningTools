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

CPPFLAGS = -g -Wall -pedantic -W -ggdb -O3 -std=gnu99 -lm
EVERYTHING = $(wildcard *.c) $(wildcard ./executables/*.c)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
TPKOBJECTS = $(OBJECTS) ./executables/main.o
LWMOBJECTS = $(OBJECTS) ./executables/levelwiseTreesetMiningMain.o
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
	

all: tpk lwm csc cpk ts mtg mgg sts gf cstr

$(GFNAME): $(GFOBJECTS)
	@echo "\nLink Graph Filter executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(CSTRNAME): $(CSTROBJECTS)
	@echo "\nLink Canonical String executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(CPKNAME): $(CPKOBJECTS)
	@echo "\nLink Cyclic Pattern Kernel executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(STSNAME): $(STSOBJECTS)
	@echo "Link Spanning Tree Sampling executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(TPKNAME): $(TPKOBJECTS)
	@echo "\nLink Tree Pattern Kernel executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(LWMNAME): $(LWMOBJECTS)
	@echo "\nLink Levelwise Treeset Mining executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(MTGNAME): $(MTGOBJECTS)
	@echo "\nLink Data Transformer executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(MGGNAME): $(MGGOBJECTS)
	@echo "\nLink Data Transformer executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(CSCNAME): $(CSCOBJECTS)
	@echo "\nLink Count Graph Properties executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(CCDNAME): $(CCDOBJECTS)
	@echo "\nLink Count Graph Properties executable:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(TCINAME): $(TCIOBJECTS)
	@echo "\nLink Cactree Subgraph Isomorphism:"
	@gcc -o $@ $^ $(CPPFLAGS)

$(PERFNAME): $(PERFOBJECTS)
	@echo "\nLink Cactree Subgraph Isomorphism Performance Test:"
	@gcc -o $@ $^ $(CPPFLAGS)

%.o : %.c %.h
	@gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm ./executables/*.o
