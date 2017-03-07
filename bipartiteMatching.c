#include <stdio.h>
#include <limits.h>
#include <malloc.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "vertexQueue.h"
#include "intMath.h"


/**
 * Bipartite matching algorithms and network flow algorithms.
 *
 * Somewhat unintuitively, the following conventions are used:
 *
 * We are dealing with directed networks. Each edge e has a capacity, some flow on it, and a residual edge e'
 * which are stored as follows
 *
 * - e->label points to e' and e'->label points to e
 * - e->used = e'->used = capacity
 * - e->flag = capacity - e'->flag = flow on e
 *
 */



/**
set edge to flag (either 0 or 1) and residual edge
to 1-flag */
void setFlag(struct VertexList* e, int flag) {
	e->flag = flag;
	((struct VertexList*)e->label)->flag = 1 - flag;
}


/**
Add flow to edge flow and subtract flow from residual edge flow.
Somebody else is responsible of checking whether this operation is valid.
*/
void addFlow(struct VertexList* e, int flow) {
	e->flag += flow;
	((struct VertexList*)e->label)->flag -= flow;
}


static int pr_vertexBecomesActive(struct Vertex* v, struct Vertex* s, struct Vertex* t) {
	int excessIsZero = v->visited == 0;
	return excessIsZero && (v != s) && (v != t);
}


static void pr_push(struct VertexList* e, struct Vertex* s, struct Vertex* t, struct ShallowGraph** activeVertices, struct ShallowGraphPool* sgp) {
	// compute pushable flow
	int flow = min(e->startPoint->visited, e->used - e->flag);

	// add endPoint to list of active vertices, if it becomes active now
	if (pr_vertexBecomesActive(e->endPoint, s, t)) {
		addToVertexQueue(e->endPoint, activeVertices[e->endPoint->d], sgp);
	}

	// augment preflow by flow
	addFlow(e, flow);
	e->startPoint->visited -= flow;
	e->endPoint->visited += flow;
}


static int pr_relabel(struct Vertex* v) {
	int newDistance = INT_MAX;
	for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
		if ((e->endPoint->d < newDistance) && (e->used - e->flag > 0)) {
			newDistance = e->endPoint->d + 1;
		}
	}
	v->d = newDistance;
	return newDistance;
}


static struct ShallowGraph** pr_init(struct Graph* g, struct Vertex* s, struct ShallowGraphPool* sgp) {

	struct ShallowGraph** activeVertices = malloc((2 * g->n - 1) * sizeof(struct ShallowGraph*));
	for (int i=0; i<2 * g->n - 1; ++i) {
		activeVertices[i] = getShallowGraph(sgp);
	}

	// set distance label to n
	s->d = g->n;

	for (struct VertexList* e=s->neighborhood; e!=NULL; e=e->next) {
		// set preflow of out edges to max
		addFlow(e, e->used);
		// add excess to s and endPoint
		e->endPoint->visited += e->flag;
		s->visited -= e->flag;
		addToVertexQueue(e->endPoint, activeVertices[0], sgp);
	}
	return activeVertices;
}


static int pr_isAllowedEdge(struct VertexList* e) {
	int isForward = e->startPoint->d == e->endPoint->d + 1;
	int notSaturated = e->used - e->flag > 0;
	return isForward && notSaturated;
}


/**
 * Return the largest index such that the list of active vertices with that distance label is not empty.
 * If there are no active vertices left with distance label >= 1, the return value will be 0 and point
 * either to a full or an empty list. This results in correct behavior.
 */
static int pr_findSmallestActiveIndex(struct ShallowGraph** activeVertices, int i) {
	int j = i-1;
	for ( ; j>0; --j) {
		if (peekFromVertexQueue(activeVertices[j]) != NULL) {
			break;
		}
	}
	return j;
}

int pr_sanityCheck(struct Graph* g, struct Vertex* s, struct Vertex* t, FILE* out) {
	int sOuttIn = s->visited + t->visited;

	int excessViolated = 0;
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[vi];
		if ((v != s) && (v != t)) {
			if (v->visited != 0) {
				++excessViolated;
			}
		}
	}

	if (out) {
		fprintf(out, "excess s: %i, excess t:%i, violations in between: %i\n", s->visited, t->visited, excessViolated);
	}

	return !sOuttIn && !excessViolated;
}

static int pr_checkActiveArray(struct ShallowGraph** activeVertices, int nLists, FILE* out) {
	int forgottenVertices = 0;
	for (int i=0; i<nLists; ++i) {
		if (peekFromVertexQueue(activeVertices[i])) {
			forgottenVertices += activeVertices[i]->m;
			if (out) {
				fprintf(out, "Level %i: Vertex %i\n", i, peekFromVertexQueue(activeVertices[i])->number);
			}
		}
	}
	if (out) {
		fprintf(out, "total of forgotten vertices: %i\n", forgottenVertices);
	}
	return forgottenVertices;
}



/**
 * Goldberg and Tarjans Push-Relabel Algorithm.
 * Implementation follows Korte, Vygen: Combinatorial Optimization, Chapter 8.5
 */
void pushRelabel(struct Graph* g, struct Vertex* s, struct Vertex* t, struct ShallowGraphPool* sgp) {

	struct ShallowGraph** activeVertices = pr_init(g, s, sgp);

//	int debug = 0;
//	char filename [50];

	int i = 0;
	for (struct Vertex* active=popFromVertexQueue(activeVertices[i], sgp); active!=NULL; active=popFromVertexQueue(activeVertices[i], sgp)) {

//		int activeId = active->number;
//		if (activeId == 80) {
//			printf("!");
//		}

		// look for allowed edges and push as much flow as possible
		char foundAllowedEdge = 0;
		for (struct VertexList* e=active->neighborhood; e!=NULL; e=e->next) {
			if (pr_isAllowedEdge(e)) {
				pr_push(e, s, t, activeVertices, sgp);
				foundAllowedEdge = 1;

				// if vertex lost its 'active' state, stop looping
				if (active->visited == 0) {
					break;
				}
			}
		}

		// if there are no allowed edges (left), relabel v
		if (!foundAllowedEdge) {
			i = pr_relabel(active);
			addToVertexQueue(active, activeVertices[i], sgp);
		} else {
			if (active->visited != 0) {
				addToVertexQueue(active, activeVertices[i], sgp);
			}
		}

//		sprintf(filename, "flowInstance_step%i.dot", debug);
//		FILE* f = fopen(filename, "w");
//		printFlowInstanceDotFormat(g, f);
//		fclose(f);
//		++debug;

		if (peekFromVertexQueue(activeVertices[i]) == NULL) {
			i = pr_findSmallestActiveIndex(activeVertices, i);
		}

	}

//	pr_checkActiveArray(activeVertices, 2 * g->n - 1, stderr);

	// garbage collection
	for (int i=0; i<2 * g->n - 1; ++i) {
		dumpShallowGraph(sgp, activeVertices[i]);
	}
	free(activeVertices);
}


/**
dfs that searches for a path from s to t in a network where every edge has capacity 1
and augments it, if found.
returns 1 if there is a path or 0 otherwise.
*/
char augment(struct Vertex* s, struct Vertex* t) {
	struct VertexList* e;

	if (s == t) {
		return 1;
	}
	s->visited = 1;
	for (e=s->neighborhood; e!=NULL; e=e->next) {
		if ((e->flag == 0) && (e->endPoint->visited == 0)) {
			char found = augment(e->endPoint, t);
			if (found) {
				setFlag(e, 1);
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
if found. Not restricted to capacity 1 networks (however, maximum
capacities should be polynomial in graph size, o/w there might be
superpolynomial many augmentation steps).
returns 1 if there is a path or 0 otherwise.
*/
char augmentWithCapacity(struct Vertex* s, struct Vertex* t) {
	if (s == t) {
		return 1;
	}

	s->visited = 1;
	for (struct VertexList* e=s->neighborhood; e!=NULL; e=e->next) {
		if ((e->flag < e->used) && (e->endPoint->visited == 0)) {
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
		if (e->flag == 1) {
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
e->flag = 0, f->flag = 1
e->label = f, f->label = e (for constant time augmenting)
*/
void addResidualEdges(struct Vertex* v, struct Vertex* w, struct ListPool* lp) {
	struct VertexList* f1;
	struct VertexList* f2;

	f1 = getVertexList(lp);
	f1->startPoint = v;
	f1->endPoint = w;
	f2 = inverseEdge(f1, lp);

	f1->flag = 0;
	f2->flag = 1;
	f1->label = (char*)f2;
	f2->label = (char*)f1;

	addEdge(v, f1);
	addEdge(w, f2);
}


/**
Method for constructing the residual graph. 
Creates an edge e between v and w and its residual edge f
between w and v.
e->used = capacity, f->used = capacity
e->flag = 0. f->flag = capacity
e->label = f, f->label = e (for constant time augmenting)
*/
void addResidualEdgesWithCapacity(struct Vertex* v, struct Vertex* w, int capacity, struct ListPool* lp) {
	struct VertexList* f1;
	struct VertexList* f2;

	f1 = getVertexList(lp);
	f1->startPoint = v;
	f1->endPoint = w;
	f2 = inverseEdge(f1, lp);

	f1->used = capacity;
	f2->used = capacity;
	f1->flag = 0;
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
later on, it is better to create a local copy.
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
Init the bipartite graph such that edges have flag 0, residual 
edges have flag 1.
*/
void initBipartite(struct Graph* B) {
	int i;
	struct VertexList* e;
	for (i=0; i<B->number; ++i) {
		for (e=B->vertices[i]->neighborhood; e!=NULL; e=e->next) {
			setFlag(e, 0);
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
and needs to be encoded in the ->flag member of each edge.
->visited needs to be initialized to 0 for each vertex.

The algorithm changes the ->flag values of edges in g
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
		if ((e->flag == 0) && (e->endPoint->visited == 0)) {
			char found = augment_B_rec(B, e->endPoint);
			if (found) {
				setFlag(e, 1);
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
and needs to be encoded in the ->flag member of each edge.
->visited needs to be initialized to 0 for each vertex.

The algorithm changes the ->flag values of edges in g and the ->d values of vertices.

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
and needs to be encoded in the ->flag member of each edge.
->visited needs to be initialized to 0 for each vertex.

The algorithm changes the ->flag values of edges in g and the ->d values of vertices.

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
Returns ShallowGraph containing a copy of each edge from A to B that has ->flag == 1.
These edges form a matching, if bipartiteMatchingFastAndDirty was invoked on g
*/
struct ShallowGraph* getMatching(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* matching = getShallowGraph(sgp);
	struct VertexList* e;
	int v;
	
	for (v=0; v<g->number; ++v) {
		for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->flag == 1) {
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
