#ifndef GRAPH_H
#define GRAPH_H


/******** Graph data structures ******************************/

/**
The vertices of a graph can store certain information
*/
struct Vertex{
	char* label;
	struct VertexList* neighborhood;
	struct Vertex* next;
	int number;
	int visited;
	int lowPoint; 
	int isStringMaster;
	int d;
};

/**
This is used to model edges
*/
struct VertexList{
	char* label;
	struct VertexList* next;
	struct Vertex* startPoint;
	struct Vertex* endPoint;
	int used;
	/* not used atm */
	int flag;
	int isStringMaster;
};

/**
A graph has n vertices and m edges and a !constant! number of vertices.
*/
struct Graph{
	int n;
	int m;
	int number;
	int activity;
	struct Vertex** vertices;
	struct Graph* next;
};

/**
A ShallowGraph is a container for a list of edges representing a subgraph of some Graph
*/
struct ShallowGraph{
	struct VertexList* edges;
	struct VertexList* lastEdge;
	int m;
	struct ShallowGraph* next;
	struct ShallowGraph* prev;
};


/******** Object pools *****************************************/
struct ShallowGraphPool{
	struct ShallowGraph* unused;
	struct ShallowGraph* tmp;
	struct ListPool* listPool;
};

struct GraphPool{
	struct Graph* unused;
	struct Graph* tmp;
	struct VertexPool* vertexPool;
	struct ListPool* listPool;
};

struct ListPool{
	struct VertexList* unused;
	struct VertexList* tmp;
};

struct VertexPool{
	struct Vertex* unused;
	struct Vertex* tmp;
};


/******* VertexList ******************************************/
struct VertexList* push(struct VertexList* list, struct VertexList* e);
struct VertexList* shallowCopyEdge(struct VertexList* e, struct ListPool* p);
struct VertexList* inverseEdge(struct VertexList* e, struct ListPool* p);


/******* Vertex ***********************************************/
struct Vertex* shallowCopyVertex(struct Vertex *v, struct VertexPool *p);

void removeEdge(struct Vertex* v, struct Vertex* w, struct ListPool* p);
void addEdge(struct Vertex* v, struct VertexList* e);

int degree(struct Vertex* v);
char isLeaf(struct Vertex* v);
int commonNeighborCount(struct Vertex* v, struct Vertex* w);
char isIncident(struct Vertex* v, struct Vertex* w);
char isDegreeTwoVertex(struct Vertex* v);


/******* ShallowGraph ******************************************/
struct ShallowGraph* cloneShallowGraph(struct ShallowGraph* g, struct ShallowGraphPool* sgp);
void rebaseShallowGraph(struct ShallowGraph* list, struct Graph* newBase);

void pushEdge(struct ShallowGraph *g, struct VertexList *e);
void appendEdge(struct ShallowGraph *g, struct VertexList *e);
struct VertexList* popEdge(struct ShallowGraph* g);
struct VertexList* assertLastPointer(struct ShallowGraph* g);

struct ShallowGraph* inverseCycle(struct ShallowGraph* cycle, struct ShallowGraphPool *sgp);
struct ShallowGraph* addComponent(struct ShallowGraph* g, struct ShallowGraph* h);


/******* Graph *************************************************/
struct Graph* cloneGraph(struct Graph* g, struct GraphPool* gp);
struct Graph* cloneInducedGraph(struct Graph* g, struct GraphPool* gp);

struct Vertex** setVertexNumber(struct Graph* g, int n);
struct Graph* emptyGraph(struct Graph* g, struct GraphPool* gp);
struct Graph* createGraph(int n, struct GraphPool* gp);
void addEdgeBetweenVertices(int v, int w, char* label, struct Graph* g, struct GraphPool* gp);
void addEdges(struct Graph* g, struct ShallowGraph* list, struct GraphPool* gp);
struct VertexList* deleteEdge(struct Graph* g, int v, int w);
void deleteEdges(struct Graph* g, struct ShallowGraph* list, struct GraphPool* gp);
void deleteEdgeBetweenVertices(struct Graph* g, struct VertexList* idx, struct GraphPool* gp);
char isNeighbor(struct Graph* g, int v, int w);

struct ShallowGraph* getGraphEdges(struct Graph *g, struct ShallowGraphPool* sgp);
struct Graph* shallowGraphToGraph(struct ShallowGraph* edgeList, struct GraphPool* gp);

#include "memoryManagement.h"

#endif /* GRAPH_H */
