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
LWGNAME = lwg
GENNAME = ggen

CPPFLAGS = -g -Wall -pedantic -W -ggdb -O3 -std=gnu99 -lm
EVERYTHING = $(wildcard *.c) $(wildcard ./executables/*.c)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
HELPFILES = $(patsubst ./executables/%.txt, ./executables/%.help, $(wildcard ./executables/*.txt))

TPKOBJECTS = $(OBJECTS) ./executables/main.o
TPKHELP = 
LWMOBJECTS = $(OBJECTS) ./executables/levelwiseTreesetMiningMain.o
LWMHELP = ./executables/levelwiseTreesetMiningMainHelp.help
LWGOBJECTS = $(OBJECTS) ./executables/levelwiseGraphMining.o
LWGHELP =
MTGOBJECTS = $(OBJECTS) ./executables/mapTrees2gaston.o
MTGHELP =
MGGOBJECTS = $(OBJECTS) ./executables/mapAIDS2gaston.o
MGGHELP =
CPKOBJECTS = $(OBJECTS) ./executables/cpkMain.o
CPKHELP =
STSOBJECTS = $(OBJECTS) ./executables/treeSamplingMain.o
STSHELP = ./executables/treeSamplingHelp.help
CCDOBJECTS = $(OBJECTS) ./executables/countCycleDegree.o
CCDHELP =
GFOBJECTS = $(OBJECTS) ./executables/filter.o
GFHELP = ./executables/filterHelp.help
CSTROBJECTS = $(OBJECTS) ./executables/cstring.o
CSTRHELP = ./executables/cstringHelp.help
TCIOBJECTS = $(OBJECTS) ./executables/cactus.o
TCIHELP =
PERFOBJECTS = $(OBJECTS) ./executables/tciPerf.o
PERFHELP =
GENOBJECTS = $(OBJECTS) ./executables/generator.o
GENHELP = ./executables/generatorHelp.help

# visualize the include dependencies between the source files.
# for this, .c and .h files with the same name are interpreted as one entity
dependencies.png: $(EVERYTHING)
	executables/cinclude2dot | sed 's/\.c/\.h/' | dot -Tpng -Gsize=40,40 > $@
	eog $@


main: $(GFNAME)
	
all: $(TPKNAME) $(LWMNAME) $(MTGNAME) $(MGGNAME) $(CPKNAME) $(STSNAME) $(CCDNAME) $(TCINAME) $(PERFNAME) $(GFNAME) $(CSTRNAME) $(LWGNAME)

%.help: %.txt
	@xxd -i $< > $@

help: $(HELPFILES)

$(GFNAME): $(GFHELP) $(GFOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(CSTRNAME): $(CSTRHELP) $(CSTROBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(CPKNAME): $(CPKHELP) $(CPKOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(STSNAME): $(STSHELP) $(STSOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(TPKNAME): $(TPKHELP) $(TPKOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(LWMNAME): $(LWMHELP) $(LWMOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(MTGNAME): $(MTGHELP) $(MTGOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(MGGNAME): $(MGGHELP) $(MGGOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(CCDNAME): $(CCDHELP) $(CCDOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(TCINAME): $(TCIHELP) $(TCIOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(PERFNAME): $(PERFHELP) $(PERFOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(LWGNAME): $(LWGHELP) $(LWGOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(GENNAME): $(GENHELP) $(GENOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

%.o : %.c %.h
	@gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm ./executables/*.o
	rm ./executables/*.help

print-%:
	@echo $*=$($*)
	