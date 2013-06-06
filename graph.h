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

/******* stuff ***********************************************/
char* copyString(char* string);


/******* VertexList ******************************************/
struct ListPool* createListPool(unsigned int initNumberOfElements);
void freeListPool(struct ListPool *p);

struct VertexList* getVertexList(struct ListPool* pool);
void dumpVertexList(struct ListPool* p, struct VertexList* l);
void dumpVertexListRecursively(struct ListPool* p, struct VertexList* e);

struct VertexList* push(struct VertexList* list, struct VertexList* e);
struct VertexList* shallowCopyEdge(struct VertexList* e, struct ListPool* p);
struct VertexList* inverseEdge(struct VertexList* e, struct ListPool* p);
void printVertexList(struct VertexList *f);


/******* Vertex ***********************************************/
struct VertexPool* createVertexPool(unsigned int initNumberOfElements);
void freeVertexPool(struct VertexPool* p);

struct Vertex* getVertex(struct VertexPool* p);
void dumpVertex(struct VertexPool* p, struct Vertex* v);

struct Vertex* shallowCopyVertex(struct Vertex *v, struct VertexPool *p);

void removeEdge(struct Vertex* v, struct Vertex* w, struct ListPool* p);
void addEdge(struct Vertex* v, struct VertexList* e);

int commonNeighborCount(struct Vertex* v, struct Vertex* w);
char isIncident(struct Vertex* v, struct Vertex* w);
char isDeg2Vertex(struct Vertex* v);


/******* ShallowGraph ******************************************/
struct ShallowGraphPool* createShallowGraphPool(unsigned int initNumberOfElements, struct ListPool* lp);
void freeShallowGraphPool(struct ShallowGraphPool *p);

struct ShallowGraph* getShallowGraph(struct ShallowGraphPool *p);
void dumpShallowGraph(struct ShallowGraphPool *p, struct ShallowGraph* g);
void dumpShallowGraphCycle(struct ShallowGraphPool *p, struct ShallowGraph* g);

void appendEdge(struct ShallowGraph *g, struct VertexList *e);
void pushEdge(struct ShallowGraph *g, struct VertexList *e);
struct VertexList* popEdge(struct ShallowGraph* g);
struct VertexList* assertLastPointer(struct ShallowGraph* g);

struct ShallowGraph* inverseCycle(struct ShallowGraph* cycle, struct ShallowGraphPool *sgp);

struct ShallowGraph* addComponent(struct ShallowGraph* g, struct ShallowGraph* h);
void printShallowGraph(struct ShallowGraph* g);
int printShallowGraphCount(struct ShallowGraph* g, char silent);


/******* Graph *************************************************/
struct GraphPool* createGraphPool(unsigned int initNumberOfElements, struct VertexPool* vp, struct ListPool* lp);
void freeGraphPool(struct GraphPool* p);

struct Graph* getGraph(struct GraphPool* p);
struct Graph* cloneGraph(struct Graph* g, struct GraphPool* gp);
struct Graph* cloneInducedGraph(struct Graph* g, struct GraphPool* gp);
void dumpGraph(struct GraphPool* p, struct Graph *g);

struct Vertex** setVertexNumber(struct Graph* g, int n);

struct ShallowGraph* getGraphEdges(struct Graph *g, struct ShallowGraphPool* sgp);
struct Graph* shallowGraphToGraph(struct ShallowGraph* edgeList, struct GraphPool* gp);

void printGraph(struct Graph* g);
void printGraphEdges(struct Graph *g);


#endif /* SLAVE_H */
