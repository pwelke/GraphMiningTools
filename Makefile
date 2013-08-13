TPKNAME = tpk
LWMNAME = lwm
CPPFLAGS = -g -Wall -pedantic -W -ggdb
# -O3
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
TPKOBJECTS = $(filter-out levelwiseMain.o, $(OBJECTS))
LWMOBJECTS = $(filter-out main.o, $(OBJECTS))

main: tpk
	

all: main
	

tpk: $(TPKOBJECTS)
	gcc -o $(TPKNAME) $(TPKOBJECTS) $(CPPFLAGS)

lwm: $(LWMOBJECTS)
	gcc -o $(LWMNAME) $(LWMOBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	rm tpk
