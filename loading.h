#ifndef LOADING_H
#define LOADING_H

#include "graph.h"

struct Graph* readSimpleFormat(char* filename, int undirected, struct GraphPool *p, int strspace);

char* aids99VertexLabel(const unsigned int label);
char** aids99VertexLabelArray();
char* aids99EdgeLabel(const unsigned int label);
char* intLabel(const unsigned int label);
struct Graph* iterateFile(char*(*getVertexLabel)(const unsigned int), char*(*getEdgeLabel)(const unsigned int));
void createFileIterator(char* filename, struct GraphPool* p);
void createStdinIterator(struct GraphPool* p);
void destroyFileIterator();
void writeCurrentGraph(FILE* out);


#endif /* LOADING_H */
