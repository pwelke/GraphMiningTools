TPKNAME = tpk
LWMNAME = lwm
MAPNAME = map
CPPFLAGS = -g -Wall -pedantic -W -ggdb -O3
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
TPKOBJECTS = $(filter-out levelwiseMain.o map2gaston.o, $(OBJECTS))
LWMOBJECTS = $(filter-out main.o map2gaston.o, $(OBJECTS))
MAPOBJECTS = $(filter-out main.o levelwiseMain.o, $(OBJECTS))

main: map
	

all: main
	

tpk: $(TPKOBJECTS)
	gcc -o $(TPKNAME) $(TPKOBJECTS) $(CPPFLAGS)

lwm: $(LWMOBJECTS)
	gcc -o $(LWMNAME) $(LWMOBJECTS) $(CPPFLAGS)

map: $(MAPOBJECTS)
	gcc -o $(MAPNAME) $(MAPOBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
