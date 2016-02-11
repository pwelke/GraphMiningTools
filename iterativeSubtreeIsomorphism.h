#ifndef ITERATIVE_SUBTREEISO_H_
#define ITERATIVE_SUBTREEISO_H_

#include "newCube.h"

int computeCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v, struct GraphPool* gp);

/* vertices of g have their ->visited values set to the postorder. Thus, 
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
struct Graph* makeBipartiteInstanceFromVertices(struct SubtreeIsoDataStore data, struct Vertex* removalVertex, struct Vertex* u, struct Vertex* v, struct GraphPool* gp);
int* getParentsFromPostorder(struct Graph* g, int* postorder) ;
/* Return an array holding the indices of the parents of each vertex in g with root root.
the parent of root does not exist, which is indicated by index -1 */
int* getParents(struct Graph* g, int root);


// SUBTREE ISOMORPHISM

/**
Iterative Labeled Subtree Isomorphism Check. 

Implements the labeled subtree isomorphism algorithm of
Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism in an iterative version:

Input:
	a text    tree g
	a pattern tree h
	the cube that was computed for some subtree h-e and g, where e is an edge to a leaf of h
	(object pool data structures)

Output:
	yes, if h is subgraph isomorphic to g, no otherwise
	the cube for h and g

*/
void iterativeSubtreeCheck_intern(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* currentPointer, struct GraphPool* gp);
struct SubtreeIsoDataStore iterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct Graph* h, struct GraphPool* gp);


// INITIALIZATORS

struct SubtreeIsoDataStore initG(struct Graph* g);
/** create the set of characteristics for a single edge pattern graph */
struct SubtreeIsoDataStore initIterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct VertexList* patternEdge, struct GraphPool* gp);

#endif
