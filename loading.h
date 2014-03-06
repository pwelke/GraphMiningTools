#ifndef LOADING_H
#define LOADING_H

#include "graph.h"

struct Graph* readSimpleFormat(char* filename, int undirected, struct GraphPool *p, int strspace);

char* aids99VertexLabel(int label);
char** aids99VertexLabelArray();
char* aids99EdgeLabel(int label);
struct Graph* iterateFile(char*(*getVertexLabel)(int), char*(*getEdgeLabel)(int));
void createFileIterator(char* filename, struct GraphPool* p);
void destroyFileIterator();


#endif /* LOADING_H */
