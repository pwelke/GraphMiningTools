PROJECTNAME = tpk
CPPFLAGS = -g -Wall -pedantic -W -ggdb
# -O3
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

main: $(OBJECTS)
	gcc -o $(PROJECTNAME) $(OBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	
new: clean main

all: main
