
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
OBJECTFOLDER = ./o
XOBJECTFOLDER = ./o/executables
COMPILEDHELPFOLDER = ./o/help
EVERYTHING = $(wildcard *.c) $(wildcard ./executables/*.c)
OBJECTS = $(patsubst %.c,$(OBJECTFOLDER)/%.o,$(wildcard *.c))
HELPFILES = $(patsubst ./executables/%.txt, $(COMPILEDHELPFOLDER)/%.help, $(wildcard ./executables/*.txt))

# specify executable names, objects, helpfiles, and compilation recipes
TPKNAME = tpk
TPKOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/main.o
TPKHELP = 
$(TPKNAME): $(TPKHELP) $(TPKOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

LWGNAME = lwg
LWGOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/levelwiseGraphMiningMain.o
LWGHELP = $(COMPILEDHELPFOLDER)/levelwiseGraphMiningHelp.help
$(LWGNAME): $(LWGHELP) $(LWGOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

LWGRNAME = lwgr
LWGROBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/levelwiseGraphMiningRootedMain.o
LWGRHELP = $(COMPILEDHELPFOLDER)/levelwiseGraphMiningRootedHelp.help
$(LWGRNAME): $(LWGRHELP) $(LWGROBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)


CPKNAME = cpk
CPKOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/cpkMain.o
CPKHELP =
$(CPKNAME): $(CPKHELP) $(CPKOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

STSNAME = ts
STSOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/treeSamplingMain.o
STSHELP = $(COMPILEDHELPFOLDER)/treeSamplingHelp.help
$(STSNAME): $(STSHELP) $(STSOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

CCDNAME = ccd
CCDOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/countCycleDegree.o
CCDHELP =
$(CCDNAME): $(CCDHELP) $(CCDOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

GFNAME = gf
GFOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/filter.o
GFHELP = $(COMPILEDHELPFOLDER)/filterHelp.help
$(GFNAME): $(GFHELP) $(GFOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

CSTRNAME = cstring
CSTROBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/cstring.o
CSTRHELP = $(COMPILEDHELPFOLDER)/cstringHelp.help
$(CSTRNAME): $(CSTRHELP) $(CSTROBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

TCINAME = tci
TCIOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/cactus.o
TCIHELP =
$(TCINAME): $(TCIHELP) $(TCIOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

#PERFNAME = perf
#PERFOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/tciPerf.o
#PERFHELP =
#$(PERFNAME): $(PERFHELP) $(PERFOBJECTS)
#	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

GENNAME = ggen
GENOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/generator.o
GENHELP = $(COMPILEDHELPFOLDER)/generatorHelp.help
$(GENNAME): $(GENHELP) $(GENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)
	
CGENNAME = cgen
CGENOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/chainGenerator.o
CGENHELP = $(COMPILEDHELPFOLDER)/chainGeneratorHelp.help
$(CGENNAME): $(CGENHELP) $(CGENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

NGENNAME = ngen
NGENOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/neighborhoodGenerator.o
NGENHELP = $(COMPILEDHELPFOLDER)/neighborhoodGeneratorHelp.help
$(NGENNAME): $(NGENHELP) $(NGENOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

WLNAME = wl
WLOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/weisfeilerLehmanMain.o
WLHELP = $(COMPILEDHELPFOLDER)/weisfeilerLehmanMainHelp.help
$(WLNAME): $(WLHELP) $(WLOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

PENAME = gpe
PEOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/patternExtractor.o
PEHELP = $(COMPILEDHELPFOLDER)/patternExtractorHelp.help
$(PENAME): $(PEHELP) $(PEOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

TESTNAME = test
TESTOBJECTS = $(OBJECTS) ./tests/testsuite.o
$(TESTNAME): $(TESTOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)
	@./test

GFCNAME = gfc
GFCOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/formatConverter.o 
GFCHELP = $(COMPILEDHELPFOLDER)/formatConverterHelp.help
$(GFCNAME): $(GFCHELP) $(GFCOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

L2UNAME = labeled2unlabeled
L2UOBJECTS = $(OBJECTS) $(XOBJECTFOLDER)/labeled2unlabeledMain.o 
L2UHELP = $(COMPILEDHELPFOLDER)/labeled2unlabeledMainHelp.help
$(L2UNAME): $(L2UHELP) $(L2UOBJECTS)
	@$(CC) -o $@ $(filter-out %.help, $^) $(CPPLINKFLAGS)

ALLTARGETS = ${CGENNAME} $(L2UNAME) $(TPKNAME) $(MTGNAME) $(MGGNAME) $(CPKNAME) $(STSNAME) $(CCDNAME) $(TCINAME) $(GFNAME) $(CSTRNAME) $(LWGNAME) $(LWGRNAME) $(GENNAME) $(NGENNAME) $(WLNAME) $(PENAME) $(GFCNAME)
# $(PERFNAME)

# visualize the include dependencies between the source files.
# for this, .c and .h files with the same name are interpreted as one entity
dependencies.png: $(EVERYTHING)
	executables/cinclude2dot | sed 's/\.c/\.h/' | dot -Tpng -Gsize=40,40 > $@
	eog $@

deploy: $(ALLTARGETS)
	mv $(ALLTARGETS) ../experiments/bin

main: $(GFNAME)
	
all: $(ALLTARGETS)

$(COMPILEDHELPFOLDER)/%.help: ./executables/%.txt
	@xxd -i $< > $@

help: $(HELPFILES)

$(OBJECTFOLDER)/%.o : %.c %.h
	@$(CC) $(CPPFLAGS) -c $< -o $@

$(XOBJECTFOLDER)/%.o : ./executables/%.c ./executables/%.h
	@$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	@rm -f *.o
	@rm -f $(OBJECTFOLDER)/*.o
	@rm -f ./executables/*.o
	@rm -f ./executables/*.help
	@rm -f $(COMPILEDHELPFOLDER)/*.help
	@rm -f $(ALLTARGETS) test

print-%:
	@echo $*=$($*)
	