PROJECTNAME = opk
CPPFLAGS = -g  -O3 -Wall -pedantic -W -ggdb
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

main: $(OBJECTS)
	gcc -o $(PROJECTNAME) $(OBJECTS) $(CPPFLAGS)

%.o : %.c %.h
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm *.o
	
new: clean main

all: main
