#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

#include "loading.h"

/** This function loads a graph from a file. 
The file has to have two numbers in the first line specifying number of vertices
and number of edges.
Then there should be a list of numbers followed by a string where the number 
defines the number of a vertex and the string specifies the label of the vertex.
Then follows a list of edges given by startvertex, endvertex (ints) and edge label (string).
The label strings must not contain whitespaces.
 */
struct Graph* readSimpleFormat(char* filename, int undirected, struct GraphPool *p, int strspace) {
	int i;
	int v, w;
	FILE* file = NULL;
	struct VertexList *e, *f;
	struct Graph* g = getGraph(p);

	if ((file = fopen(filename, "r")) ){

		/* read info on the graph */
		if (fscanf(file, "%i %i \n", &(g->n), &(g->m)) == 2) {
			/* create vertex pointer array. The actual vertices are taken from vp*/

			if (!setVertexNumber(g, g->n)) {
				printf("Error allocating memory for vertex array.\n");
				fclose(file);
				free(g);
				return NULL;
			}

			/* read vertex info */
			for (i=0; i<g->n; ++i) {
				g->vertices[i] = getVertex(p->vertexPool);
				g->vertices[i]->label = malloc(strspace * sizeof(char));
				/* the vertex is the primary owner of its label string, other vertices may refer to its 
				string but must not free it, if deleted*/
				g->vertices[i]->isStringMaster = 1;
				if (fscanf(file, "%i %s\n", &(g->vertices[i] ->number), g->vertices[i] ->label) != 2) {
					printf("Error reading vertex %i\n", i);
					return NULL;
				}
			}

			/* read info on edges */
			for (i=0; i< g->m; ++i) { 
				e = getVertexList(p->listPool);
				e->label = malloc(strspace * sizeof(char));
				if (fscanf(file, "%i %i %s\n", &v, &w, e->label) == 3) {
					e->startPoint = g->vertices[v];
					e->endPoint = g->vertices[w];
					/* the edge is the primary owner of its label string, other edges may refer to its 
					string but must not free it, if deleted*/
					e->isStringMaster = 1;

					/* if graph is undirected, add two edges, if it is directed add one edge */
					addEdge(g->vertices[e->startPoint->number], e);
					if (undirected) {
						f = shallowCopyEdge(e, p->listPool);
						f->startPoint = e->endPoint;
						f->endPoint = e->startPoint;
						addEdge(g->vertices[f->startPoint->number], f);						
					}
				} else {
					printf("Error reading edge %i\n", i);
					fclose(file);
					return NULL;
				}
			}


		} else {
			printf("Error reading the file %s\n", filename);
			fclose(file);
			return NULL;
		}
	} else {
		printf("Error opening the file %s\n", filename);
		return NULL;
	}

	fclose(file);
	return g;
}



/***********************************************************************************
 ******************** The Input Functions ******************************************
 ***********************************************************************************/


/* global variables used by the file iterator */
FILE* FI_DATABASE = NULL;
struct GraphPool* FI_GP = NULL;
// long int STARTPOSITION = 0;

/* cache for current graph */
char** HEAD_PTR = NULL;
size_t* HEAD_SIZE = NULL;
char** VERTEX_PTR = NULL;
size_t* VERTEX_SIZE = NULL;
char** EDGE_PTR = NULL;
size_t* EDGE_SIZE = NULL;

/**
 * We allow 2048 bytes of cache for each cache that is used to read vertex labels, edge labels, and the header of a graph.
 */
void initCache() {
	HEAD_SIZE = malloc(sizeof(size_t));
	*HEAD_SIZE = 2048;
	HEAD_PTR = malloc(sizeof(char*));
	*HEAD_PTR = malloc(*HEAD_SIZE * sizeof(char));

	VERTEX_SIZE = malloc(sizeof(size_t));
	*VERTEX_SIZE = 2048;
	VERTEX_PTR = malloc(sizeof(char*));
	*VERTEX_PTR = malloc(*VERTEX_SIZE * sizeof(char));

	EDGE_SIZE = malloc(sizeof(size_t));
	*EDGE_SIZE = 2048;
	EDGE_PTR = malloc(sizeof(char*));
	*EDGE_PTR = malloc(*EDGE_SIZE * sizeof(char));
}

/* open a database file to stream graphs from */
void createFileIterator(char* filename, struct GraphPool* p) {

	if ((FI_DATABASE = fopen(filename, "r"))) {
		FI_GP = p;
		initCache();
	} else {
		printf("File %s not found\n", filename);
	}

}

/**
init stdin to be the stream to read graphs from
*/
void createStdinIterator(struct GraphPool* p) {
	FI_DATABASE = stdin;
	FI_GP = p;
	initCache();
}

/** close the datastream */
void destroyFileIterator() {
	fclose(FI_DATABASE);
	
	free(HEAD_SIZE);
	free(*HEAD_PTR);
	free(HEAD_PTR);
	HEAD_PTR = NULL;
	HEAD_SIZE = NULL;

	free(VERTEX_SIZE);
	free(*VERTEX_PTR);
	free(VERTEX_PTR);
	VERTEX_PTR = NULL;
	VERTEX_SIZE = NULL;

	free(EDGE_SIZE);
	free(*EDGE_PTR);
	free(EDGE_PTR);
	EDGE_PTR = NULL;
	EDGE_SIZE = NULL;
}


/**
A method to directly print a graph from the input stream to the specified output stream.
*/
void writeCurrentGraph(FILE* out) {
	fputs(*HEAD_PTR, out);
	fputs(*VERTEX_PTR, out);
	fputs(*EDGE_PTR, out);
}


/* from http://stackoverflow.com/questions/16826422/c-most-efficient-way-to-convert-string-to-int-faster-than-atoi 
fastAtoi also is an altered version of code that can be found there. */
static unsigned int digitValue (char c)
{
   return (unsigned int)(c - '0');
}


/**
Parse a positive integer from the string starting at *pos.
Move *pos to the first position in the string where there is no number.
Skip any number of initial white spaces, then terminate at first position that is not a digit.

Return -1 if nothing was read due to invalid input.
In this case, the value *pos is not changed
*/
static inline int fastAtoi( const char ** pos )
{
	const char *p = *pos;
	unsigned int d;
	unsigned int n=0;

	for ( ; isspace(*p); p++) {}
	const char* start = p;
	--p;
	while ((d = digitValue(*++p)) <= 9)
	{
		n = n * 10 + d;
	}

	if (start != p) {
		*pos = p;
		return n;
	} else {
		return -1;
	}
}

static int AGNOSTIC_ERROR = 1;

/**
Parse an integer from the string starting at *pos.
Move *pos to the first position in the string where there is no number.
Skip any number of initial white spaces, then terminate at first position that is not initial - or digit.

Cornercase: Interprets '-' as 0.

Set local variable AGNOSTIC_ERROR to 1 if nothing was read due to invalid input.
*/
static inline int fastAtoiAgnostic( const char ** pos )
{
   const char *p = *pos;



   for ( ; isspace(*p); p++) {}
   const char *start = p;
   int x;
   if (*p == '-') {
	   x = -1;
   } else {
	   x = 1;
	   --p;
   }

   int n = 0;
   unsigned int d;
   while ((d = digitValue(*++p)) <= 9)
   {
      n = n * 10 + d;
   }


   if (start != p) {
	   // move pointer
	   *pos = p;
	   AGNOSTIC_ERROR = 0;
	   return x*n;
   } else {
	   // return error code
	   AGNOSTIC_ERROR = 1;
	   return 0;
   }

}


static inline int parseHeader(int* id, int* activity, int* n, int* m) { 
	const char *current = *HEAD_PTR + 1;
	if ((*HEAD_PTR)[0] != '#') { 
		return -1; 
	}

	*id = fastAtoiAgnostic(&current);
	int agnostic_errors = AGNOSTIC_ERROR;
	*activity = fastAtoiAgnostic(&current);
	agnostic_errors += AGNOSTIC_ERROR;
	*n = fastAtoi(&current);
	*m = fastAtoi(&current);
	/* return number of correctly read items.
		See: C11(ISO/IEC 9899:201x) §6.5.8 Relational operators */
	return 2 - agnostic_errors + (*n != -1) + (*m != -1);
}


static inline void fastLabelLength(const char* pos, size_t* offset, size_t* labelSize) {
	*offset = 0;
	*labelSize = 0;

    // skip whitespaces
    for ( ; isspace(*pos); pos++) {
    	++(*offset);
    }

    // get labelSize
    for ( ; !isspace(*pos); pos++) {
		++(*labelSize);
    }
}

static inline int grabLabel(const char** currentPosition, char** label) {
	size_t offset;
	size_t labelSize;
	*label = NULL;
	fastLabelLength(*currentPosition, &offset, &labelSize);
	if (labelSize != 0) {
		if ((*label = malloc((labelSize + 1) * sizeof(char)))) {
			memcpy(*label, *currentPosition + offset, labelSize);
			(*label)[labelSize] = '\0';
			*currentPosition += offset + labelSize;	
			return labelSize;
		}
	} 
	return 0;
}


static inline int parseEdgeNew(const char** currentPosition, int* v, int* w, char** label) {
	*v = fastAtoi(currentPosition);
	*w = fastAtoi(currentPosition);
	grabLabel(currentPosition, label);
	return (*v != -1) + (*w != -1) + (*label != NULL);
}


/* stream a graph from a database file of the format described in the documentation */
struct Graph* iterateFile() {
	int i;
	struct Graph* g = getGraph(FI_GP);
	const char* currentPosition;

	if (!FI_DATABASE) {
		fprintf(stderr, "Could not access input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* copy header line to local variable
	dependent on GNU C */
	if (getline(HEAD_PTR, HEAD_SIZE, FI_DATABASE) == -1) {
		fprintf(stderr, "Could not read graph header from input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}
			
	/* parse header */	
	if (parseHeader(&(g->number), &(g->activity), &(g->n), &(g->m)) != 4) {
		/* if reading of header does not work anymore, check if we have reached the correct end of the stream */
		if (**HEAD_PTR != '$') {
			fprintf(stderr, "Invalid Graph header: %s\nparsing result: %i %i %i %i\n", *HEAD_PTR, g->number, g->activity, g->n, g->m);
		}
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* read vertices */
	if ((g->vertices = malloc(g->n * sizeof(struct Vertex*))) == NULL) {
		fprintf(stderr, "Error allocating vertices\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* copy vertex line to local variable
	dependent on GNU C */
	if (getline(VERTEX_PTR, VERTEX_SIZE, FI_DATABASE) == -1) {
		fprintf(stderr, "Could not read vertex line from input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* parse vertex info */
	currentPosition = *VERTEX_PTR;
	for (i=0; i<g->n; ++i) {
		char* label;
		if (grabLabel(&currentPosition, &label) != -1) {
			g->vertices[i] = getVertex(FI_GP->vertexPool);
			g->vertices[i]->label = label;
			g->vertices[i]->number = i;
			g->vertices[i]->isStringMaster = 1;
		} else {
			fprintf(stderr, "Error while parsing vertices\n");
			dumpGraph(FI_GP, g);
			return NULL;
		}
	}
					
	/* copy edge line to local variable
	dependent on GNU C */
	if (getline(EDGE_PTR, EDGE_SIZE, FI_DATABASE) == -1) {
		fprintf(stderr, "Could not read edge line from input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* parse edge info */
	currentPosition = *EDGE_PTR;
	for (i=0; i<g->m; ++i) {
		char* label;
		int v,w;

		if (parseEdgeNew(&currentPosition, &v, &w, &label) == 3) {
			
			struct VertexList* e = getVertexList(FI_GP->listPool);
			struct VertexList* f = getVertexList(FI_GP->listPool);

			/* edge */
			e->startPoint = g->vertices[v-1];
			e->endPoint = g->vertices[w-1];
			e->label = label;
			e->isStringMaster = 1;

			addEdge(e->startPoint, e);

			/* reverse edge*/
			f->startPoint = g->vertices[w-1];
			f->endPoint = g->vertices[v-1];
			f->label = e->label;

			addEdge(f->startPoint, f);
		} else {
			fprintf(stderr, "Error while parsing edges\n");
			dumpGraph(FI_GP, g);
			return NULL;
		}
	}
	return g;
}


/* stream a directed graph from a database file of the format described in the documentation */
struct Graph* iterateFileDirected() {
	int i;
	struct Graph* g = getGraph(FI_GP);
	const char* currentPosition;

	if (!FI_DATABASE) {
		fprintf(stderr, "Could not access input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* copy header line to local variable
	dependent on GNU C */
	if (getline(HEAD_PTR, HEAD_SIZE, FI_DATABASE) == -1) {
		fprintf(stderr, "Could not read graph header from input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* parse header */
	if (parseHeader(&(g->number), &(g->activity), &(g->n), &(g->m)) != 4) {
		/* if reading of header does not work anymore, check if we have reached the correct end of the stream */
		if (**HEAD_PTR != '$') {
			fprintf(stderr, "Invalid Graph header: %s\nparsing result: %i %i %i %i\n", *HEAD_PTR, g->number, g->activity, g->n, g->m);
		}
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* read vertices */
	if ((g->vertices = malloc(g->n * sizeof(struct Vertex*))) == NULL) {
		fprintf(stderr, "Error allocating vertices\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* copy vertex line to local variable
	dependent on GNU C */
	if (getline(VERTEX_PTR, VERTEX_SIZE, FI_DATABASE) == -1) {
		fprintf(stderr, "Could not read vertex line from input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* parse vertex info */
	currentPosition = *VERTEX_PTR;
	for (i=0; i<g->n; ++i) {
		char* label;
		if (grabLabel(&currentPosition, &label) != -1) {
			g->vertices[i] = getVertex(FI_GP->vertexPool);
			g->vertices[i]->label = label;
			g->vertices[i]->number = i;
			g->vertices[i]->isStringMaster = 1;
		} else {
			fprintf(stderr, "Error while parsing vertices\n");
			dumpGraph(FI_GP, g);
			return NULL;
		}
	}

	/* copy edge line to local variable
	dependent on GNU C */
	if (getline(EDGE_PTR, EDGE_SIZE, FI_DATABASE) == -1) {
		fprintf(stderr, "Could not read edge line from input stream.\n");
		dumpGraph(FI_GP, g);
		return NULL;
	}

	/* parse edge info */
	currentPosition = *EDGE_PTR;
	for (i=0; i<g->m; ++i) {
		char* label;
		int v,w;

		if (parseEdgeNew(&currentPosition, &v, &w, &label) == 3) {

			struct VertexList* e = getVertexList(FI_GP->listPool);

			/* edge */
			e->startPoint = g->vertices[v-1];
			e->endPoint = g->vertices[w-1];
			e->label = label;
			e->isStringMaster = 1;

			addEdge(e->startPoint, e);

			/* reverse edge is not added! */

		} else {
			fprintf(stderr, "Error while parsing edges\n");
			dumpGraph(FI_GP, g);
			return NULL;
		}
	}
	return g;
}



/* return the base 10 string representation of label in a correctly sized newly allocated buffer */
char* intLabel(const unsigned int label) {
	int bufferSize;
	char* representation;
	
	if (label>1) {
		bufferSize = (int)ceil(log10(label)) + 2;
	} else {
		bufferSize = 3;
	}

	representation = malloc(bufferSize * sizeof(char));
	if (representation) {
		sprintf(representation, "%i", label);
	}
	return representation;
}


/**
 * return the label strings corresponding to the numbers used in AIDS99.txt
 */
char* aids99EdgeLabel(const unsigned int label) {
	char* res = malloc(5*sizeof(char));
	sprintf(res, "%i", label);
	return res;
}


/**
 * return the label strings corresponding to the numbers used in AIDS99.txt
 */
char* aids99VertexLabel(const unsigned int label) {
	char* res = malloc(5*sizeof(char));

	switch (label) {
	case 1:
		sprintf(res, "%s", "H");
		break;
	case 2:
		sprintf(res, "%s", "C");
		break;
	case 3:
		sprintf(res, "%s", "O");
		break;
	case 4:
		sprintf(res, "%s", "CU");
		break;
	case 5:
		sprintf(res, "%s", "N");
		break;
	case 6:
		sprintf(res, "%s", "S");
		break;
	case 7:
		sprintf(res, "%s", "P");
		break;
	case 8:
		sprintf(res, "%s", "CL");
		break;
	case 9:
		sprintf(res, "%s", "ZN");
		break;
	case 10:
		sprintf(res, "%s", "B");
		break;
	case 11:
		sprintf(res, "%s", "BR");
		break;
	case 12:
		sprintf(res, "%s", "CO");
		break;
	case 13:
		sprintf(res, "%s", "MN");
		break;
	case 14:
		sprintf(res, "%s", "AS");
		break;
	case 15:
		sprintf(res, "%s", "AL");
		break;
	case 16:
		sprintf(res, "%s", "NI");
		break;
	case 17:
		sprintf(res, "%s", "SE");
		break;
	case 18:
		sprintf(res, "%s", "SI");
		break;
	case 19:
		sprintf(res, "%s", "V");
		break;
	case 20:
		sprintf(res, "%s", "SN");
		break;
	case 21:
		sprintf(res, "%s", "I");
		break;
	case 22:
		sprintf(res, "%s", "F");
		break;
	case 23:
		sprintf(res, "%s", "LI");
		break;
	case 24:
		sprintf(res, "%s", "SB");
		break;
	case 25:
		sprintf(res, "%s", "FE");
		break;
	case 26:
		sprintf(res, "%s", "PD");
		break;
	case 27:
		sprintf(res, "%s", "HG");
		break;
	case 28:
		sprintf(res, "%s", "BI");
		break;
	case 29:
		sprintf(res, "%s", "NA");
		break;
	case 30:
		sprintf(res, "%s", "CA");
		break;
	case 31:
		sprintf(res, "%s", "TI");
		break;
	case 32:
		sprintf(res, "%s", "ZR");
		break;
	case 33:
		sprintf(res, "%s", "HO");
		break;
	case 34:
		sprintf(res, "%s", "GE");
		break;
	case 35:
		sprintf(res, "%s", "PT");
		break;
	case 36:
		sprintf(res, "%s", "RU");
		break;
	case 37:
		sprintf(res, "%s", "RH");
		break;
	case 38:
		sprintf(res, "%s", "CR");
		break;
	case 39:
		sprintf(res, "%s", "GA");
		break;
	case 40:
		sprintf(res, "%s", "K");
		break;
	case 41:
		sprintf(res, "%s", "AG");
		break;
	case 42:
		sprintf(res, "%s", "AU");
		break;
	case 43:
		sprintf(res, "%s", "TB");
		break;
	case 44:
		sprintf(res, "%s", "IR");
		break;
	case 45:
		sprintf(res, "%s", "TE");
		break;
	case 46:
		sprintf(res, "%s", "MG");
		break;
	case 47:
		sprintf(res, "%s", "PB");
		break;
	case 48:
		sprintf(res, "%s", "W");
		break;
	case 49:
		sprintf(res, "%s", "CS");
		break;
	case 50:
		sprintf(res, "%s", "MO");
		break;
	case 51:
		sprintf(res, "%s", "RE");
		break;
	case 52:
		sprintf(res, "%s", "CD");
		break;
	case 53:
		sprintf(res, "%s", "OS");
		break;
	case 54:
		sprintf(res, "%s", "PR");
		break;
	case 55:
		sprintf(res, "%s", "ND");
		break;
	case 56:
		sprintf(res, "%s", "SM");
		break;
	case 57:
		sprintf(res, "%s", "GD");
		break;
	case 58:
		sprintf(res, "%s", "YB");
		break;
	case 59:
		sprintf(res, "%s", "ER");
		break;
	case 60:
		sprintf(res, "%s", "U");
		break;
	case 61:
		sprintf(res, "%s", "TL");
		break;
	case 62:
		sprintf(res, "%s", "NB");
		break;
	case 63:
		sprintf(res, "%s", "AC");
		break;
	default:
		sprintf(res, "ERR");
		break;
	}
	return res;
}

/**
 * return an array of label strings corresponding to the numbers used in AIDS99.txt.
 * that is: label[i] contains the string that is encoded in the file using the integer i
 */
char** aids99VertexLabelArray() {
	char** label = malloc(64*sizeof(char*));
	int i;
	for (i=0; i<64; ++i) {
		label[i] = malloc(5*sizeof(char));
	}

	label[0] = "ERR";
	label[1] = "H";
	label[2] = "C";
	label[3] = "O";
	label[4] = "CU";
	label[5] = "N";
	label[6] = "S";
	label[7] = "P";
	label[8] = "CL";
	label[9] = "ZN";
	label[10] = "B";
	label[11] = "BR";
	label[12] = "CO";
	label[13] = "MN";
	label[14] = "AS";
	label[15] = "AL";
	label[16] = "NI";
	label[17] = "SE";
	label[18] = "SI";
	label[19] = "V";
	label[20] = "SN";
	label[21] = "I";
	label[22] = "F";
	label[23] = "LI";
	label[24] = "SB";
	label[25] = "FE";
	label[26] = "PD";
	label[27] = "HG";
	label[28] = "BI";
	label[29] = "NA";
	label[30] = "CA";
	label[31] = "TI";
	label[32] = "ZR";
	label[33] = "HO";
	label[34] = "GE";
	label[35] = "PT";
	label[36] = "RU";
	label[37] = "RH";
	label[38] = "CR";
	label[39] = "GA";
	label[40] = "K";
	label[41] = "AG";
	label[42] = "AU";
	label[43] = "TB";
	label[44] = "IR";
	label[45] = "TE";
	label[46] = "MG";
	label[47] = "PB";
	label[48] = "W";
	label[49] = "CS";
	label[50] = "MO";
	label[51] = "RE";
	label[52] = "CD";
	label[53] = "OS";
	label[54] = "PR";
	label[55] = "ND";
	label[56] = "SM";
	label[57] = "GD";
	label[58] = "YB";
	label[59] = "ER";
	label[60] = "U";
	label[61] = "TL";
	label[62] = "NB";
	label[63] = "AC";

	return label;
}
