PROJECTNAME = tpk
GCCFLAGS = -g -Wall -pedantic -W -ggdb
# -O3
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

TPKOBJECTS = $(filter-out levelwiseMain.o, $(OBJECTS))
LWMOBJECTS = $(filter-out main.o, $(OBJECTS))

main: tpk

tpk: $(TPKOBJECTS)
	gcc -o $(PROJECTNAME) $(TPKOBJECTS) $(GCCFLAGS)

lwm: $(LWMOBJECTS)
	gcc -o lwm $(LWMOBJECTS) $(GCCFLAGS)

%.o : %.c %.h
	gcc $(GCCFLAGS) -c $< -o $@

clean:
	rm *.o
	rm tpk
	
new: clean main

all: main
