#ifndef CANONICALSTRING_H
#define CANONICALSTRING_H

/**
 * Used to store results of mergeSearchTree
 */
struct compInfo{
	int id;
	int count;
	int depth;
};
struct compInfo* getResultVector(int n);
int compInfoComparison(const void* e1, const void* e2);

int lexComp(const void* e1, const void* e2);
int lexicographicComparison(const struct ShallowGraph *g1, const struct ShallowGraph *g2);
int compareVertexLists(const struct VertexList* e1, const struct VertexList* e2);


struct Vertex* buildSearchTree(struct ShallowGraph* strings, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char addStringToSearchTree(struct Vertex* root, struct VertexList* edge, struct GraphPool* p);
void dumpSearchTree(struct GraphPool* p, struct Vertex* root);
void printSearchTree(struct Vertex* root, int level);
void printStringsInSearchTree(struct Vertex* root, FILE* stream, struct ShallowGraphPool* sgp);

/* should be private */
struct VertexList* getTerminatorEdge(struct ListPool *p);
struct VertexList* getInitialisatorEdge(struct ListPool *p);

struct ShallowGraph* getTreePatterns(struct Graph* forest, struct ShallowGraphPool *sgp);
struct ShallowGraph* canonicalStringOfRootedTree(struct Vertex* vertex, struct Vertex* parent, struct ShallowGraphPool *p);
struct ShallowGraph* canonicalStringOfRootedLevelTree(struct Vertex* vertex, struct Vertex* parent, int maxDepth, struct ShallowGraphPool *p);
struct ShallowGraph* canonicalStringOfLevelTree(struct ShallowGraph* vertexList, int maxDepth, struct ShallowGraphPool* sgp);

void mergeSearchTrees(struct Vertex* globalTree, struct Vertex* localTree, int divisor, struct compInfo* results, int* pos, struct Vertex* trueRoot, int depth, struct GraphPool* p);
void shallowMergeSearchTrees(struct Vertex* globalTree, struct Vertex* localTree, int divisor, struct compInfo* results, int* pos, struct Vertex* trueRoot, int depth, struct GraphPool* p);

struct ShallowGraph* permutateCycle(struct ShallowGraph* g);
struct ShallowGraph* getCyclePatterns(struct ShallowGraph* cycles, struct ShallowGraphPool* sgp);

/* should be private */
void markConnectedComponents(struct Vertex *v, int component);

struct ShallowGraph* getCanonicalStringOfOuterplanarBlock(struct ShallowGraph* hamiltonianCycle, struct ShallowGraph* diagonals, struct ShallowGraphPool* sgp);

void printCanonicalString(struct ShallowGraph *s, FILE* stream);
void printCanonicalStrings(struct ShallowGraph *s, FILE* stream);

char* canonicalStringToChar(struct ShallowGraph* string);

#endif /* CANONICALSTRING_H */
