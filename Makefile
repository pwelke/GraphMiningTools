TPKNAME = tpk
LWMNAME = lwm
MTGNAME = mtg
MGGNAME = mgg
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
CPKOBJECTS = $(OBJECTS) ./executables/cpkMain.o
STSOBJECTS = $(OBJECTS) ./executables/treeSamplingMain.o
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


main: $(GFNAME)
	
all: $(TPKNAME) $(LWMNAME) $(MTGNAME) $(MGGNAME) $(CPKNAME) $(STSNAME) $(CCDNAME) $(TCINAME) $(PERFNAME) $(GFNAME) $(CSTRNAME) 

$(GFNAME): $(GFOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(CSTRNAME): $(CSTROBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(CPKNAME): $(CPKOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(STSNAME): $(STSOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(TPKNAME): $(TPKOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(LWMNAME): $(LWMOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(MTGNAME): $(MTGOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(MGGNAME): $(MGGOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(CCDNAME): $(CCDOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(TCINAME): $(TCIOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

$(PERFNAME): $(PERFOBJECTS)
	@gcc -o $@ $^ $(CPPFLAGS)

%.o : %.c %.h
	@gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm ./executables/*.o

print-%:
	@echo $*=$($*)