#ifndef CONNECTED_COMPONENTS_H_
#define CONNECTED_COMPONENTS_H_

struct ShallowGraph* getConnectedComponents(struct Graph* g, struct ShallowGraphPool* sgp);
struct ShallowGraph* getRepresentativeVertices(struct Graph* g, struct ShallowGraphPool* sgp);
char isConnected(struct Graph* g);
void markComp(struct Vertex* v, int compNumber);

#endif
