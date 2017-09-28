#ifndef GRAPH_PRINTING_H
#define GRAPH_PRINTING_H

void printGraphAidsFormat(struct Graph* g, FILE* out);
void printGraphEdgesOfTwoGraphs(char* name, struct Graph *g, struct Graph* h);
char diffGraphs(char* name, struct Graph *g, struct Graph* h);
void printVertexList(struct VertexList *f);
void printEdge(struct VertexList *e);
void printGraphEdges(struct Graph *g);
void printGraph(struct Graph* g);
int printShallowGraphCount(struct ShallowGraph* g, char silent);
void printShallowGraph(struct ShallowGraph* g);
void printLabelledShallowGraph(struct ShallowGraph* g);
void printOverlapGraphDotFormat(struct Graph* g, FILE* out);
void printGraphDotFormat(struct Graph* g, FILE* out);

#endif /* GRAPH_PRINTING_H */
