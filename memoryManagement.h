#ifndef MEMORY_MANAGEMENT_H_
#define MEMORY_MANAGEMENT_H_

/** do not include this header somewhere directly. Always just include graph.h */

struct ListPool* createListPool(unsigned int initNumberOfElements);
void freeListPool(struct ListPool *p);
struct VertexList* getVertexList(struct ListPool* pool);
void dumpVertexList(struct ListPool* p, struct VertexList* l);
void dumpVertexListRecursively(struct ListPool* p, struct VertexList* e);

struct VertexPool* createVertexPool(unsigned int initNumberOfElements);
void freeVertexPool(struct VertexPool* p);
struct Vertex* getVertex(struct VertexPool* p);
void dumpVertex(struct VertexPool* p, struct Vertex* v);

struct ShallowGraphPool* createShallowGraphPool(unsigned int initNumberOfElements, struct ListPool* lp);
void freeShallowGraphPool(struct ShallowGraphPool *p);
struct ShallowGraph* getShallowGraph(struct ShallowGraphPool *p);
void dumpShallowGraph(struct ShallowGraphPool *p, struct ShallowGraph* g);
void dumpShallowGraphCycle(struct ShallowGraphPool *p, struct ShallowGraph* g);

struct GraphPool* createGraphPool(unsigned int initNumberOfElements, struct VertexPool* vp, struct ListPool* lp);
void freeGraphPool(struct GraphPool* p);
struct Graph* getGraph(struct GraphPool* p);
void dumpGraph(struct GraphPool* p, struct Graph *g);

/******* stuff ***********************************************/
char* copyString(char* string);


#endif