#include <string.h>
#include <malloc.h>

#include "cs_Parsing.h"



/* internalized strings for initializator and terminator edges */
char* initString = "(";
char* termString = ")";
char getTerminatorSymbol() { return ')'; }
char getInitialisatorSymbol() { return '('; }

/**
Returns a VertexList object that represents the end of a canonical String.
Currently the ) sign is used for that.
 */
struct VertexList* getTerminatorEdge(struct ListPool *p) {
	struct VertexList* e = getVertexList(p);
	e->label = termString;
	return e;
}

/**
Returns a VertexList object that represents the beginning of a canonical String.
Currently the ( sign is used for that.
 */
struct VertexList* getInitialisatorEdge(struct ListPool *p) {
	struct VertexList* e = getVertexList(p);
	e->label = initString;
	return e;
}


/*****************************************************************************************
 **************************************  Output  *****************************************
 *****************************************************************************************/


/**
 * Conversion method to convert a string stored as a labeled path to a char array string
 */
char* canonicalStringToChar(struct ShallowGraph* string) {
	char* out;
	struct VertexList* e;
	int size = 0;
	int i;

	/* count the space needed for the string (+1 for a whitespace separating two labels) */
	for (e=string->edges; e; e=e->next) {
		size += strlen(e->label) + 1;
	}
	out = malloc((size + 1) * sizeof(char));


	for (e=string->edges, i=0; e; e=e->next) {
		sprintf(&(out[i]), "%s ", e->label);
		i += strlen(e->label) + 1;
	}

	return out;
}


/**
Print the canonical string, represented by the ShallowGraph s to 
the screen.
 */
void printCanonicalString(struct ShallowGraph* s, FILE* stream) {
	struct VertexList *i;
	for (i=s->edges; i; i = i->next) {	
		char* j;
		for (j=i->label; *j != '\0'; ++j) {
			fputc(*j, stream);
		}
		fputc(' ', stream);
	}
	fputc('\n', stream);
}


/**
Print the canonical strings represented by the list of
ShallowGraphs s 
 */
void printCanonicalStrings(struct ShallowGraph *s, FILE* stream) {
	struct ShallowGraph* i;
	for (i=s; i; i=i->next) {
		printCanonicalString(i, stream);
	}
}



/*****************************************************************************************
 **************************************  Input  ******************************************
 *****************************************************************************************/


/**
We are not done reading a canonical string from stream, if there is a non-space
character before the next line break or the end of file (whichever comes first)
*/
char __notDone(FILE* stream) {
	int c;
    for (c = fgetc(stream); c == ' '; c = fgetc(stream));
    ungetc(c, stream);
	return (c != '\0') && (c != '\n');
}


/**
Write chars in stream to buffer until a space is encountered.
buffer needs to be large enough to hold the number of chars
written. __readLabel consumes the space, but replaces its occurrence in
buffer with '\0'. this is actually quite good for the use case in parseCString,
as there, all labels are terminated by a space due to the printCanonicalString() 
implementation.
*/
void __readLabel(FILE* stream, char* buffer) {
	int i = 0;
	for (buffer[i] = fgetc(stream); buffer[i] != ' '; buffer[++i] = fgetc(stream));
	buffer[i] = '\0';
}


/**
parses a canonical string from stream that was written using printCanonicalString().
*/
struct ShallowGraph* parseCString(FILE* stream, char* buffer, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* string = getShallowGraph(sgp);
	struct VertexList* e;
	
	while (__notDone(stream)) {
		__readLabel(stream, buffer);
		e = getVertexList(sgp->listPool);
		if (strcmp(buffer, termString) == 0) {
			e->label = termString;
		} else {
			if (strcmp(buffer, initString) == 0) {
				e->label = initString;
			} else {
				int length = strlen(buffer) + 1;
				char* label = malloc(length * sizeof(char));
				strcpy(label, buffer);
				e->label = label;
				e->isStringMaster = 1;
			}
		}
		appendEdge(string, e);
	}
	return string;
}

static FILE* str2stream(char* str) {
	// extremely dangerous and extremely nasty
	FILE* f = fopen("/tmp/nasty", "w");
	fprintf(f, "%s", str);
	fclose(f);
	return fopen("/tmp/nasty", "r");
}

/**
 * create a canonical string struct given its representation as a char array string
 * Not thread safe, of course. Also, extremely inefficient, I guess. Should only be used for debugging.
 */
struct ShallowGraph* string2cstring(char* str, struct ShallowGraphPool* sgp) {
	FILE* f = str2stream(str);
	char x[20];
	struct ShallowGraph* cstr = parseCString(f, x, sgp);
	fclose(f);
	return cstr;
}
