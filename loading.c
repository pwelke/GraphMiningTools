#include <stdio.h>
#include <malloc.h>
#include <string.h>
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

void initCache() {
	HEAD_SIZE = malloc(sizeof(size_t));
	*HEAD_SIZE = 20;
	HEAD_PTR = malloc(sizeof(char*));
	*HEAD_PTR = malloc(*HEAD_SIZE * sizeof(char));

	VERTEX_SIZE = malloc(sizeof(size_t));
	*VERTEX_SIZE = 20;
	VERTEX_PTR = malloc(sizeof(char*));
	*VERTEX_PTR = malloc(*VERTEX_SIZE * sizeof(char));

	EDGE_SIZE = malloc(sizeof(size_t));
	*EDGE_SIZE = 20;
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

/* stream a graph from a database file of the format described in the documentation */
struct Graph* iterateFile(char*(*getVertexLabel)(int), char*(*getEdgeLabel)(int)) {
	if (FI_DATABASE) {
		int i;
		int error = 0;
		struct Graph* g = getGraph(FI_GP);

		// dependent on GNU C
		getline(HEAD_PTR, HEAD_SIZE, FI_DATABASE);

		if (sscanf(*HEAD_PTR, " # %i %i %i %i", &(g->number), &(g->activity), &(g->n), &(g->m)) == 4) {

			if ((g->vertices = malloc(g->n * sizeof(struct Graph*)))) {

				if (error == 0) {
					char* currentPosition;
					// dependent on GNU C
					getline(VERTEX_PTR, VERTEX_SIZE, FI_DATABASE);
					currentPosition = *VERTEX_PTR;

					/* read vertex info */
					for (i=0; i<g->n; ++i) {
						int label = -1;
						int charsRead;
						if (sscanf(currentPosition, " %i%n", &label, &charsRead) == 1) {
							currentPosition += charsRead;

							g->vertices[i] = getVertex(FI_GP->vertexPool);
							g->vertices[i]->label = getVertexLabel(label);
							g->vertices[i]->number = i;
							g->vertices[i]->isStringMaster = 1;
						} else {
							fprintf(stderr, "Error vertex\n");
							error = 1;
							free(g->vertices);
							break;
						}
					}
				}

				if (error == 0) {
					char* currentPosition;
					// dependent on GNU C
					getline(EDGE_PTR, EDGE_SIZE, FI_DATABASE);
					currentPosition = *EDGE_PTR;
					/* read edge info */
					for (i=0; i<g->m; ++i) {
						int label = -1;
						int v,w;
						int charsRead;

						if (sscanf(currentPosition, " %i %i %i%n", &v, &w, &label, &charsRead) == 3) {
							
							struct VertexList* e = getVertexList(FI_GP->listPool);
							struct VertexList* f = getVertexList(FI_GP->listPool);
							
							currentPosition += charsRead;

							/* edge */
							e->startPoint = g->vertices[v-1];
							e->endPoint = g->vertices[w-1];
							e->label = getEdgeLabel(label);
							e->isStringMaster = 1;

							addEdge(e->startPoint, e);

							/* reverse edge*/
							f->startPoint = g->vertices[w-1];
							f->endPoint = g->vertices[v-1];
							f->label = e->label;

							addEdge(f->startPoint, f);
						} else {
							fprintf(stderr, "Error edge\n");
							error = 1;
							free(g->vertices);
							break;
						}
					}
				}
			} else {
				error = 1;
				fprintf(stderr, "Error allocating vertices\n");
			}

			/* if everything worked, return graph, otherwise return graph initialized to -1 */
			if (error == 0) {
				return g;
			} else {
				char c;
				fpos_t previous;

				/* go to the next occurrence of # */
				if (fgetpos(FI_DATABASE, &previous) != 0)
					fprintf(stderr, "Error obtaining file stream position\n");
				if (fscanf(FI_DATABASE, "%c", &c) != 1)
					fprintf(stderr, "Error skipping invalid graph\n");
				while (c != '#') {
					if (fgetpos(FI_DATABASE, &previous) != 0)
						fprintf(stderr, "Error obtaining file stream position\n");
					if (fscanf(FI_DATABASE, "%c", &c) != 1)
						fprintf(stderr, "Error skipping invalid graph\n");
				}

				/* go to the previous position s.t. the next char in the file stream is # */
				if (fsetpos(FI_DATABASE, &previous) != 0)
					fprintf(stderr, "Error positioning file stream pointer");

				g->n = g->m = g->number = g->activity = -1;
				return g;
			}

		} else {
			/* if reading of header does not work anymore, there is no graph left */
			dumpGraph(FI_GP, g);
			if (**HEAD_PTR != '$') {
				fprintf(stderr, "Invalid Graph header: %s\n", *HEAD_PTR);
			}
			return NULL;
		}
	} else {
		return NULL;
	}
}


/**
 * return the label strings corresponding to the numbers used in AIDS99.txt
 */
char* aids99EdgeLabel(int label) {
	char* res = malloc(5*sizeof(char));
	sprintf(res, "%i", label);
	return res;
}


/**
 * return the label strings corresponding to the numbers used in AIDS99.txt
 */
char* aids99VertexLabel(int label) {
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
