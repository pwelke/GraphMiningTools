TPKNAME = tpk
LWMNAME = lwm
GFCNAME = gfc
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
TESTNAME = test
L2UNAME = labeled2unlabeled
ALLTARGETS = $(L2UNAME) $(TPKNAME) $(LWMNAME) $(MTGNAME) $(MGGNAME) $(CPKNAME) $(STSNAME) $(CCDNAME) $(TCINAME) $(PERFNAME) $(GFNAME) $(CSTRNAME) $(LWGNAME) $(GENNAME) $(NGENNAME) $(WLNAME) $(TESTNAME) $(PENAME) $(GFCNAME)

CC = gcc
# CPPFLAGS = -g -Wall -Wextra -pedantic -W -ggdb -std=gnu99 -lm
CPPFLAGS = -g -Wall -Wextra -pedantic -W -std=gnu99 -lm -O2 -D NDEBUG -Wl,-O1
EVERYTHING = $(wildcard *.c) $(wildcard ./executables/*.c)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
HELPFILES = $(patsubst ./executables/%.txt, ./executables/%.help, $(wildcard ./executables/*.txt))

TPKOBJECTS = $(OBJECTS) ./executables/main.o
TPKHELP = 
LWMOBJECTS = $(OBJECTS) ./executables/levelwiseTreesetMiningMain.o
LWMHELP = ./executables/levelwiseTreesetMiningMainHelp.help
LWGOBJECTS = $(OBJECTS) ./executables/levelwiseGraphMiningMain.o
LWGHELP = ./executables/levelwiseGraphMiningHelp.help
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
TESTOBJECTS = $(OBJECTS) ./executables/lowPointTest.o
GFCOBJECTS = $(OBJECTS) ./executables/formatConverter.o 
GFCHELP = ./executables/formatConverterHelp.help
L2UOBJECTS = $(OBJECTS) ./executables/labeled2unlabeledMain.o 
L2UHELP = ./executables/labeled2unlabeledMainHelp.help

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
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(CSTRNAME): $(CSTRHELP) $(CSTROBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(CPKNAME): $(CPKHELP) $(CPKOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(STSNAME): $(STSHELP) $(STSOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(TPKNAME): $(TPKHELP) $(TPKOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(LWMNAME): $(LWMHELP) $(LWMOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(CCDNAME): $(CCDHELP) $(CCDOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(TCINAME): $(TCIHELP) $(TCIOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(PERFNAME): $(PERFHELP) $(PERFOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(LWGNAME): $(LWGHELP) $(LWGOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(GENNAME): $(GENHELP) $(GENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(NGENNAME): $(NGENHELP) $(NGENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(WLNAME): $(WLHELP) $(WLOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)
	
$(PENAME): $(PEHELP) $(PEOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)
	
$(GFCNAME): $(GFCHELP) $(GFCOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

$(TESTNAME): $(TESTOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)
	
$(L2UNAME): $(L2UHELP) $(L2UOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPFLAGS)

%.o : %.c %.h
	@$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	@rm -f *.o
	@rm -f ./executables/*.o
	@rm -f ./executables/*.help
	@rm -f $(ALLTARGETS)

print-%:
	@echo $*=$($*)
	