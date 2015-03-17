#ifndef CACTUS_TREE_H
#define CACTUS_TREE_H

struct Characteristics{
    int masterID;
    int size;
    struct TreeList ***treeIDs;
    struct Characteristics *next;
};

struct TreeList{
    int treeID;
    struct TreeList *next;
};

 int cactusTreeSubIso(struct Graph * graph, struct Graph *pattern, struct GraphPool *gPool, struct ShallowGraphPool *sgPool);
int bfs(struct Graph *graph);
struct Graph *computeComponentTree(struct Graph *graph, struct ShallowGraph *bComponents, struct GraphPool *gPool, struct ShallowGraphPool *sgPool);
struct Characteristics *processRootTree(struct Vertex *root, struct Graph *graph, struct ShallowGraph **rootedComponents, struct Graph *pattern, struct GraphPool *gPool, struct ShallowGraphPool *sgPool);
struct Characteristics *processComponentTree(struct Graph *graph, struct Vertex *root, struct Vertex  *componentRoot, int treeNumber, struct ShallowGraph **rootedComponents, struct Characteristics *oldChars, struct Characteristics *newChars, struct Graph *pattern,  struct GraphPool *gPool, struct ShallowGraphPool *sgPool);
struct Characteristics *characteristics(struct Graph *graph, struct Vertex *w, const int treeID, const int wTreeID, struct Characteristics *oldCharacteristics, struct Characteristics *newCharacteristics, struct Vertex *patternVertex, int patternSize, struct GraphPool *gPool, struct ShallowGraphPool *sgPool);

struct Characteristics *allocateCharacteristics(int size, int master);
void freeCharacteristics(struct Characteristics *characteristics);
struct Characteristics *mergeCharacteristics(struct Characteristics *one, struct Characteristics *two);
struct Characteristics *insertCharacteristic(struct Characteristics * characteristics, int size, int masterID, int patternRoot, int patternSubRoot, int treeID);
char checkCharacteristic(struct Characteristics *characteristics, const int masterID, const int patternRoot, const int patternSubRoot, const int treeID);
char hasSolution(struct Characteristics *characteristic);

struct TreeList ***mergeTreeArrays(struct TreeList ***one, struct TreeList ***two, int size);
struct TreeList *mergeTreeLists(struct TreeList *one, struct TreeList *two);
struct TreeList* insertTree(struct TreeList *trees, int treeID);
char checkTreeList(struct TreeList *treeList, const int treeID);

void initSpanningTree( struct ShallowGraph *rootedComponent, struct VertexList **deletedEdges, int mask);
char nextSpanningTree(struct ShallowGraph *rootedComponent, struct VertexList **deletedEdges, int mask);

void markEdge(struct VertexList *e, int mask);
void unmarkEdge(struct VertexList *e, int mask);
void markEdgeBetweenVertices(struct Vertex *start, struct Vertex *end, int mask);
void unmarkEdgeBetweenVertices(struct Vertex *start, struct Vertex *end, int mask);

void freeAllPools(struct GraphPool *gPool, struct ShallowGraphPool *sgPool);

#endif
