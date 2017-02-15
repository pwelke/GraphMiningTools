#include <stdio.h>

#include "graph.h"
#include "bipartiteMatching.h"


/**
set edge flow to flow (either 0 or 1) and residual edge flow
to 1-used */
void setFlow(struct VertexList* e, int flow) {
	e->used = flow;
	((struct VertexList*)e->label)->used = 1 - flow;
}

/**
Add flow to edge flow and subtract flow from residual edge flow.
Somebody else is responsible of checking whether this operation is valid.
*/
void addFlow(struct VertexList* e, int flow) {
	e->used += flow;
	((struct VertexList*)e->label)->used -= flow;
}


/**
dfs that searches for a path from s to t and augments it, 
if found.
returns 1 if there is a path or 0 otherwise.
*/
char augment(struct Vertex* s, struct Vertex* t) {
	struct VertexList* e;

	if (s == t) {
		return 1;
	}
	s->visited = 1;
	for (e=s->neighborhood; e!=NULL; e=e->next) {
		if ((e->used == 0) && (e->endPoint->visited == 0)) {
			char found = augment(e->endPoint, t);
			if (found) {
				setFlow(e, 1);
				s->visited = 0;
				return 1;
			}
		}
	}
	s->visited = 0;
	return 0;
}


/**
dfs that searches for a path from s to t and augments it by 1,
if found.
returns 1 if there is a path or 0 otherwise.
*/
char augmentWithCapacity(struct Vertex* s, struct Vertex* t) {
	if (s == t) {
		return 1;
	}

	s->visited = 1;
	for (struct VertexList* e=s->neighborhood; e!=NULL; e=e->next) {
		if ((e->used < e->flag) && (e->endPoint->visited == 0)) {
			char found = augmentWithCapacity(e->endPoint, t);
			if (found) {
				addFlow(e, 1);
				s->visited = 0;
				return 1;
			}
		}
	}
	s->visited = 0;
	return 0;
}


/** 
Return the matched vertex of a, or NULL, if a is not
matched. a needs to be in the set A of the bipartition.
*/
struct Vertex* getMatchedVertex(struct Vertex* a) {
	struct VertexList* e;
	for (e=a->neighborhood; e!=NULL; e=e->next) {
		if (e->used == 1) {
			return e->endPoint;
		}
	}
	return NULL;
}

/**
Return true if a in A is covered by the current matching in B or 0 otherwise.
*/
char isMatched(struct Vertex* a) {
	return (getMatchedVertex(a) != NULL);
}


/**
Method for constructing the residual graph. 
Creates an edge e between v and w and its residual edge f
between w and v.
e->used = 0, f->used = 1
e->label = f, f->label = e (for constant time augmenting)
*/
void addResidualEdges(struct Vertex* v, struct Vertex* w, struct ListPool* lp) {
	struct VertexList* f1;
	struct VertexList* f2;

	f1 = getVertexList(lp);
	f1->startPoint = v;
	f1->endPoint = w;
	f2 = inverseEdge(f1, lp);

	f1->used = 0;
	f2->used = 1;
	f1->label = (char*)f2;
	f2->label = (char*)f1;

	addEdge(v, f1);
	addEdge(w, f2);
}


/**
Method for constructing the residual graph. 
Creates an edge e between v and w and its residual edge f
between w and v.
e->used = 0, f->used = capacity
e->flag = f->flag = capacity
e->label = f, f->label = e (for constant time augmenting)
*/
void addResidualEdgesWithCapacity(struct Vertex* v, struct Vertex* w, int capacity, struct ListPool* lp) {
	struct VertexList* f1;
	struct VertexList* f2;

	f1 = getVertexList(lp);
	f1->startPoint = v;
	f1->endPoint = w;
	f2 = inverseEdge(f1, lp);

	f1->used = 0;
	f2->used = capacity;
	f1->flag = capacity;
	f2->flag = capacity;
	f1->label = (char*)f2;
	f2->label = (char*)f1;

	addEdge(v, f1);
	addEdge(w, f2);
}


/**
Safety utility function. Given a bipartite graph, the
matching algorithm needs some strange modifications to 
work. If the bipartite graph should be used somewhere else
lateron, it is better to create a local copy. 
*/
struct Graph* __cloneStrangeBipartite(struct Graph* g, struct GraphPool* gp) {
	int v; 
	struct VertexList* e;
	struct Graph* h = createGraph(g->n, gp);
	h->number = g->number;
	h->m = g->m;

	for (v=0; v<g->number; ++v) {
		for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {		
			addResidualEdges(h->vertices[v], h->vertices[e->endPoint->number], gp->listPool);
		}
	}
	return h;
}


/**
Init the bipartite graph such that edges have ->used 0, residual
edges have ->used 1.
*/
void initBipartite(struct Graph* B) {
	int i;
	struct VertexList* e;
	for (i=0; i<B->number; ++i) {
		for (e=B->vertices[i]->neighborhood; e!=NULL; e=e->next) {
			setFlow(e, 0);
		}
	}
}


/**
Removes the vertices s and t that were added to B by bipartiteMatchingFastAndDirty().

This method should run in O(|V(B)|), as the residual edge from each vertex v in B to s or t
should be the first edge in the neighborhood of v, as it was added last.
*/
void removeSandT(struct Graph* B, struct Vertex* s, struct Vertex* t, struct GraphPool* gp) {	
	int w;

	for (w=0; w<B->number; ++w) {
		removeEdge(B->vertices[w], s, gp->listPool);
	}

	for (w=B->number; w<B->n; ++w) {
		removeEdge(B->vertices[w], t, gp->listPool);
	}

	dumpVertexListRecursively(gp->listPool, s->neighborhood);
	dumpVertexListRecursively(gp->listPool, t->neighborhood);

	dumpVertex(gp->vertexPool, s);
	dumpVertex(gp->vertexPool, t);
}


/**
Return a maximum matching of the bipartite graph g.

Input is a bipartite graph g. That is: V(g) = A \dot{\cup} B,
g->number = |A| and vertices 0 to |A|-1 belong to A.

Furthermore, there are only edges {a, b} with a \in A and 
b \in B which are undirected (hence, (a,b) and (b,a) are present in B). 
The ->label of edge (a,b) points to the edge (b,a).
That is, the cast ((struct VertexList*)e->label) is valid.

The residual capacity of those edges is expected to be 
0 for (a,b) and
1 for (b,a)
and needs to be encoded in the ->used member of each edge.
->visited needs to be initialized to 0 for each vertex.

The algorithm changes the ->used values of edges in g
*/
int bipartiteMatchingFastAndDirty(struct Graph* g, struct GraphPool* gp) {
	int v;
	int matchingSize = 0;

	struct Vertex* s = getVertex(gp->vertexPool);
	struct Vertex* t = getVertex(gp->vertexPool);
	s->number = -1;
	t->number = -2;

	/* Add s, t and edges from s to A and from B to t.
	Also, set residual capacities for these edges correctly */
	for (v=0; v<g->number; ++v) {
		addResidualEdges(s, g->vertices[v], gp->listPool);
	}

	for (v=g->number; v<g->n; ++v) {
		addResidualEdges(g->vertices[v], t, gp->listPool);
	}

	while (augment(s, t)) {
		++matchingSize;
	}

	removeSandT(g, s, t, gp);

	return matchingSize;
}


/**
dfs that searches for a path from s to t and augments it,
if found.
returns 1 if there is a path or 0 otherwise.
*/
static char augment_B_rec(struct Graph* B, struct Vertex* v) {
	struct VertexList* e;

	if ((v->number >= B->number) && (v->d == 0)) {
		v->d = 1;
		return 1;
	}
	v->visited = 1;
	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if ((e->used == 0) && (e->endPoint->visited == 0)) {
			char found = augment_B_rec(B, e->endPoint);
			if (found) {
				setFlow(e, 1);
				v->visited = 0;
				return 1;
			}
		}
	}
	v->visited = 0;
	return 0;
}

/**
dfs that searches for a path from s to t and augments it,
if found.
returns 1 if there is a path or 0 otherwise.
*/
static char augment_B(struct Graph* B) {
	for (int v=B->activity; v<B->number; ++v) {
		if (B->vertices[v]->d == 0) {
			char found = augment_B_rec(B, B->vertices[v]);
			if (found) {
				B->activity = v+1;
				B->vertices[v]->d = 1;
				return 1;
			}
		}
	}
	B->activity = B->number;
	return 0;
}


/**
Return a maximum matching of the bipartite graph g.

Input is a bipartite graph g. That is: V(g) = A \dot{\cup} B,
g->number = |A| and vertices 0 to |A|-1 belong to A.

Furthermore, there are only edges {a, b} with a \in A and
b \in B which are undirected (hence, (a,b) and (b,a) are present in B).
The ->label of edge (a,b) points to the edge (b,a).
That is, the cast ((struct VertexList*)e->label) is valid.

The residual capacity of those edges is expected to be
0 for (a,b) and
1 for (b,a)
and needs to be encoded in the ->used member of each edge.
->visited needs to be initialized to 0 for each vertex.

The algorithm changes the ->used values of edges in g and the ->d values of vertices.

Difference to bipartiteMatchingFastAndDirty() is that this method does not
explicitly add source and sink vertices but stores coverage of vertices by matching in the ->d flag of vertices.

Furthermore, it uses the fact that the way how the matching is constructed, vertices that one test for an augmenting
path starting at each vertex is sufficient.
*/
int bipartiteMatchingEvenMoreDirty(struct Graph* g) {
	int matchingSize = 0;
	int activity = g->activity;
	while (augment_B(g)) {
		++matchingSize;
	}
	g->activity = activity;
	return matchingSize;
}

/**
Return 1 if there is a matching that covers A, or 0 otherwise.

Input is a bipartite graph g. That is: V(g) = A \dot{\cup} B,
g->number = |A| and vertices 0 to |A|-1 belong to A.

Furthermore, there are only edges {a, b} with a \in A and
b \in B which are undirected (hence, (a,b) and (b,a) are present in B).
The ->label of edge (a,b) points to the edge (b,a).
That is, the cast ((struct VertexList*)e->label) is valid.

The residual capacity of those edges is expected to be
0 for (a,b) and
1 for (b,a)
and needs to be encoded in the ->used member of each edge.
->visited needs to be initialized to 0 for each vertex.

The algorithm changes the ->used values of edges in g and the ->d values of vertices.

Due to the claim mentioned above, we can stop searching for a matching that covers A the
instant we have found a vertex where no augmenting path starts. Hence, in this situation
the algorithm terminates early.

*/
char bipartiteMatchingTerminateEarly(struct Graph* B) {

	for (int v=0; v<B->number; ++v) {
		if (B->vertices[v]->d == 0) {
			char found = augment_B_rec(B, B->vertices[v]);
			if (found) {
				B->vertices[v]->d = 1;
			} else {
				return 0;
			}
		}
	}
	return 1;
}


/**
Returns ShallowGraph containing a copy of each edge from A to B that has ->used == 1.
These edges form a matching, if bipartiteMatchingFastAndDirty was invoked on g
*/
struct ShallowGraph* getMatching(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* matching = getShallowGraph(sgp);
	struct VertexList* e;
	int v;
	
	for (v=0; v<g->number; ++v) {
		for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->used == 1) {
				struct VertexList* f = shallowCopyEdge(e, sgp->listPool);
				f->label = NULL;
				appendEdge(matching, f);
			}
		}
	}
	return matching;
}


/**
Return a maximum matching of the bipartite graph h.

Input is a bipartite graph h. That is: V(h) = A \dot{\cup} B,
h->number = |A| and vertices 0 to |A|-1 belong to A.
Furthermore, there are only edges (a, b) with a \in A and 
b \in B which are undirected.

*/
struct ShallowGraph* bipartiteMatching(struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* matching;
	struct Graph* g = __cloneStrangeBipartite(h, gp);

	bipartiteMatchingFastAndDirty(g, gp);
	matching = getMatching(g, sgp);
	rebaseShallowGraph(matching, h); 

	/* garbage collection */
	dumpGraph(gp, g);

	return matching;
}
