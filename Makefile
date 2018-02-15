
# your choice of compiler 
CC = gcc
# CC = clang

# debug compiler flags
# CPPLINKFLAGS = -g -Wall -Wextra -Wshadow -Wformat=2 -pedantic -W -ggdb -std=gnu99 -lm -fsanitize=undefined
# CPPFLAGS = -g -Wall -Wextra -Wshadow -Wformat=2 -pedantic -W -ggdb -std=gnu99 -fsanitize=undefined

# optimized compiler flags
CPPLINKFLAGS = -g -Wall -Wextra -Wunreachable-code -Wshadow -Wformat=2 -pedantic -W -std=gnu99 -lm -O2 -D NDEBUG -Wl,-O1
CPPFLAGS = -g -Wall -Wextra -Wunreachable-code -Wshadow -Wformat=2 -pedantic -W -std=gnu99 -O2 -D NDEBUG

# technicalities
EVERYTHING = $(wildcard *.c) $(wildcard ./executables/*.c)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
HELPFILES = $(patsubst ./executables/%.txt, ./executables/%.help, $(wildcard ./executables/*.txt))


# specify executable names, objects, helpfiles, and compilation recipes
TPKNAME = tpk
TPKOBJECTS = $(OBJECTS) ./executables/main.o
TPKHELP = 
$(TPKNAME): $(TPKHELP) $(TPKOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

LWGNAME = lwg
LWGOBJECTS = $(OBJECTS) ./executables/levelwiseGraphMiningMain.o
LWGHELP = ./executables/levelwiseGraphMiningHelp.help
$(LWGNAME): $(LWGHELP) $(LWGOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

CPKNAME = cpk
CPKOBJECTS = $(OBJECTS) ./executables/cpkMain.o
CPKHELP =
$(CPKNAME): $(CPKHELP) $(CPKOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

STSNAME = ts
STSOBJECTS = $(OBJECTS) ./executables/treeSamplingMain.o
STSHELP = ./executables/treeSamplingHelp.help
$(STSNAME): $(STSHELP) $(STSOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

CCDNAME = ccd
CCDOBJECTS = $(OBJECTS) ./executables/countCycleDegree.o
CCDHELP =
$(CCDNAME): $(CCDHELP) $(CCDOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

GFNAME = gf
GFOBJECTS = $(OBJECTS) ./executables/filter.o
GFHELP = ./executables/filterHelp.help
$(GFNAME): $(GFHELP) $(GFOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

CSTRNAME = cstring
CSTROBJECTS = $(OBJECTS) ./executables/cstring.o
CSTRHELP = ./executables/cstringHelp.help
$(CSTRNAME): $(CSTRHELP) $(CSTROBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

TCINAME = tci
TCIOBJECTS = $(OBJECTS) ./executables/cactus.o
TCIHELP =
$(TCINAME): $(TCIHELP) $(TCIOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

PERFNAME = perf
PERFOBJECTS = $(OBJECTS) ./executables/tciPerf.o
PERFHELP =
$(PERFNAME): $(PERFHELP) $(PERFOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

GENNAME = ggen
GENOBJECTS = $(OBJECTS) ./executables/generator.o
GENHELP = ./executables/generatorHelp.help
$(GENNAME): $(GENHELP) $(GENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)
	
CGENNAME = cgen
CGENOBJECTS = $(OBJECTS) ./executables/chainGenerator.o
CGENHELP = ./executables/chainGeneratorHelp.help
$(CGENNAME): $(CGENHELP) $(CGENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

NGENNAME = ngen
NGENOBJECTS = $(OBJECTS) ./executables/neighborhoodGenerator.o
NGENHELP = ./executables/neighborhoodGeneratorHelp.help
$(NGENNAME): $(NGENHELP) $(NGENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

WLNAME = wl
WLOBJECTS = $(OBJECTS) ./executables/weisfeilerLehmanMain.o
WLHELP = ./executables/weisfeilerLehmanMainHelp.help
$(WLNAME): $(WLHELP) $(WLOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

PENAME = gpe
PEOBJECTS = $(OBJECTS) ./executables/patternExtractor.o
PEHELP = ./executables/patternExtractorHelp.help
$(PENAME): $(PEHELP) $(PEOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

TESTNAME = test
TESTOBJECTS = $(OBJECTS) ./tests/testsuite.o
$(TESTNAME): $(TESTOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)
	@./test

GFCNAME = gfc
GFCOBJECTS = $(OBJECTS) ./executables/formatConverter.o 
GFCHELP = ./executables/formatConverterHelp.help
$(GFCNAME): $(GFCHELP) $(GFCOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

L2UNAME = labeled2unlabeled
L2UOBJECTS = $(OBJECTS) ./executables/labeled2unlabeledMain.o 
L2UHELP = ./executables/labeled2unlabeledMainHelp.help
$(L2UNAME): $(L2UHELP) $(L2UOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

ALLTARGETS = ${CGENNAME} $(L2UNAME) $(TPKNAME) $(MTGNAME) $(MGGNAME) $(CPKNAME) $(STSNAME) $(CCDNAME) $(TCINAME) $(PERFNAME) $(GFNAME) $(CSTRNAME) $(LWGNAME) $(GENNAME) $(NGENNAME) $(WLNAME) $(PENAME) $(GFCNAME)

# visualize the include dependencies between the source files.
# for this, .c and .h files with the same name are interpreted as one entity
dependencies.png: $(EVERYTHING)
	executables/cinclude2dot | sed 's/\.c/\.h/' | dot -Tpng -Gsize=40,40 > $@
	eog $@

move: $(ALLTARGETS)
	mv $(ALLTARGETS) ../experiments/bin

main: $(GFNAME)
	
all: $(ALLTARGETS)

%.help: %.txt
	@xxd -i $< > $@

help: $(HELPFILES)

%.o : %.c %.h
	@$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	@rm -f *.o
	@rm -f ./executables/*.o
	@rm -f ./executables/*.help
	@rm -f $(ALLTARGETS)

print-%:
	@echo $*=$($*)
	