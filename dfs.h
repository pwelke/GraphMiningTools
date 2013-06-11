#ifndef DFS_H_
#define DFS_H_


struct ShallowGraph* findBiconnectedComponents(struct Graph* g, struct ShallowGraphPool* gp);
struct ShallowGraph* tarjanFBC(struct Vertex* v, struct Vertex* w, int i, struct VertexList* stack, struct ListPool* lp, struct ShallowGraphPool* gp);
int min(int a, int b);

struct Graph* partitionIntoForestAndCycles(struct ShallowGraph* list, struct Graph* original, struct GraphPool* p, struct ShallowGraphPool* gp);

struct ShallowGraph* readTarjanListAllCycles(struct Graph *g, struct ShallowGraphPool *sgp);
struct ShallowGraph* backtrack(struct Graph* g, struct Vertex* v, struct Vertex* parent, struct Vertex* s, int allowance, struct ShallowGraph* currentPath, struct ShallowGraphPool* sgp);
char DFS(struct Vertex* v, struct Vertex* parent, struct Vertex* target, int allowance);
char findPath(struct Vertex* v, struct Vertex* parent, struct Vertex* target, int allowance, struct ShallowGraph* path, struct ShallowGraphPool *sgp);

#endif /* DFS_H_ */
