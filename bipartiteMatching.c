#include "graph.h"
#include "stdio.h"

void setFlag(struct VertexList* e, int flag) {
	e->flag = flag;
	((struct VertexList*)e->label)->flag = 1 - flag;
}

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

struct Graph* cloneStrangeBipartite(struct Graph* g, struct GraphPool* gp) {
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


void removeSandT(struct Graph* B, struct Vertex* s, struct Vertex* t, struct GraphPool* gp) {
	struct VertexList* temp = NULL;
	struct VertexList* e;
	struct VertexList* f;
	struct VertexList* g;	
	int w;


	/* mark edges that will be removed */
	for (e=s->neighborhood; e!=NULL; e=e->next) {
		e->used = 1;
		((struct VertexList*)e->label)->used = 1;
	}
	for (e=t->neighborhood; e!=NULL; e=e->next) {
		e->used = 1;
		((struct VertexList*)e->label)->used = 1;
	}

	/* remove edges from s and t */
	for (e=s->neighborhood; e!=NULL; e=f) {
		f = e->next;
		e->next = temp;
		temp = e;
	}
	for (e=t->neighborhood; e!=NULL; e=f) {
		f = e->next;
		e->next = temp;
		temp = e;
	}

	/* remove residual edges */
	for (w=0; w<B->n; ++w) {
		f = NULL;
		g = NULL;
		/* partition edges */
		for (e=B->vertices[w]->neighborhood; e!=NULL; e=B->vertices[w]->neighborhood) {
			B->vertices[w]->neighborhood = e->next;
			if (e->used == 1) {
				e->next = f;
				f = e;
			} else {
				e->next = g;
				g = e;
			}
		}
		/* set neighborhood to unused, append used to temp */
		B->vertices[w]->neighborhood = g;
		while (f!=NULL) {
			e = f;
			f = f->next;
			e->next = temp;
			temp = e;
		}
	}
	dumpVertexListRecursively(gp->listPool, temp);
	dumpVertex(gp->vertexPool, s);
	dumpVertex(gp->vertexPool, t);
}

/**
Return a maximum matching of the biapartite graph g.

Input is a bipartite graph g. That is: V(g) = A \dot{\cup} B,
g->number = |A| and vertices 0 to |A|-1 belong to A.
Furthermore, there are only edges (a, b) with a \in A and 
b \in B which are undirected.
Furthermore, the ->label of edge (a,b) points to the edge (b,a).
That is, the cast ((struct VertexList*)e->label) is valid

The algorithm changes the ->flag values of edges in g
*/
int bipartiteMatchingFastAndDirty(struct Graph* g, struct GraphPool* gp) {
	struct Vertex* s = getVertex(gp->vertexPool);
	s->number = -1;
	struct Vertex* t = getVertex(gp->vertexPool);
	t->number = -2;

	int v;
	int matchingSize = 0;

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

	// debug
	printf("m size %i\n", matchingSize);

	return matchingSize;
}


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
Return a maximum matching of the biapartite graph h.

Input is a bipartite graph h. That is: V(h) = A \dot{\cup} B,
h->number = |A| and vertices 0 to |A|-1 belong to A.
Furthermore, there are only edges (a, b) with a \in A and 
b \in B which are undirected.

*/
struct ShallowGraph* bipartiteMatching(struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* matching;
	struct Graph* g = cloneStrangeBipartite(h, gp);

	bipartiteMatchingFastAndDirty(g, gp);
	matching = getMatching(g, sgp);
	rebaseShallowGraph(matching, h); 

	/* garbage collection */
	dumpGraph(gp, g);

	return matching;
}