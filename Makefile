TPKNAME = tpk
LWMNAME = lwm
MTGNAME = tree2gaston
MGGNAME = aids2gaston
CPKNAME = cpk
STSNAME = ts
CCDNAME = ccd
TCINAME = tci
PERFNAME = perf
GFNAME = gf
CSTRNAME = cstring
LWGNAME = lwg
GENNAME = ggen
NGENNAME = ngen
WLNAME = wl
PENAME = gpe
TESTNAME = iterativeSubtreeTest 

ALLTARGETS = $(TPKNAME) $(LWMNAME) $(MTGNAME) $(MGGNAME) $(CPKNAME) $(STSNAME) $(CCDNAME) $(TCINAME) $(PERFNAME) $(GFNAME) $(CSTRNAME) $(LWGNAME) $(GENNAME) $(NGENNAME) $(WLNAME) $(TESTNAME) $(PENAME)

CPPFLAGS = -g -Wall -Wextra -pedantic -W -ggdb -std=gnu99 -lm
#CPPFLAGS = -g -Wall -Wextra -pedantic -W -std=gnu99 -lm -O2 -D NDEBUG -Wl,-O1
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
NGENOBJECTS = $(OBJECTS) ./executables/neighborhoodGenerator.o
NGENHELP = ./executables/neighborhoodGeneratorHelp.help
WLOBJECTS = $(OBJECTS) ./executables/weisfeilerLehmanMain.o
WLHELP = ./executables/weisfeilerLehmanMainHelp.help
PEOBJECTS = $(OBJECTS) ./executables/patternExtractor.o
PEHELP = ./executables/patternExtractorHelp.help
TESTOBJECTS = $(OBJECTS) ./executables/iterativeSubtreeTest.o

# visualize the include dependencies between the source files.
# for this, .c and .h files with the same name are interpreted as one entity
dependencies.png: $(EVERYTHING)
	executables/cinclude2dot | sed 's/\.c/\.h/' | dot -Tpng -Gsize=40,40 > $@
	eog $@


main: $(GFNAME)
	
all: $(ALLTARGETS)

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

$(NGENNAME): $(NGENHELP) $(NGENOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(WLNAME): $(WLHELP) $(WLOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)
	
$(PENAME): $(PEHELP) $(PEOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(TESTNAME): $(TESTOBJECTS)
	@gcc -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

%.o : %.c %.h
	@gcc $(CPPFLAGS) -c $< -o $@

clean:
	@rm -f *.o
	@rm -f ./executables/*.o
	@rm -f ./executables/*.help
	@rm -f $(ALLTARGETS)

print-%:
	@echo $*=$($*)
	