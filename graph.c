#include <malloc.h>
#include <stdio.h>
#include "graph.h"


/********************************************************************************************************
******************************* Methods that deal with VertexList structs *****************************
********************************************************************************************************/


/**
Clones an edge. start- and endPoint of the new edge are pointers to the !same! vertex structs
as those of the original edge. The label field also refers to the !same! string as the original.
Therefore, the new edge is no stringMaster.
*/
struct VertexList* shallowCopyEdge(struct VertexList* e, struct ListPool* p) {
	struct VertexList* f = getVertexList(p);
	f->startPoint = e->startPoint;
	f->endPoint = e->endPoint;
	f->label = e->label;
	/* f does not manage its string but refers to a different lists string, thus it is no stringMaster */
	return f;
}


/**
Clones an edge. start- and endPoint of the new edge are pointers to the !same! vertex structs
as those of the original edge but !inversed!. The label field also refers to the !same! string as the original.
Therefore, the new edge is no stringMaster.
*/
struct VertexList* inverseEdge(struct VertexList* e, struct ListPool* p) {
	struct VertexList* f = getVertexList(p);
	f->startPoint = e->endPoint;
	f->endPoint = e->startPoint;
	f->label = e->label;
	/* f does not manage its string but refers to a different lists string, thus it is no stringMaster */
	return f;
}


/**
The Method takes the second argument as new head of the list and appends the first argument to its tail.
Caution: atm any list element dangling at the second argument will be lost.
Caution: The Position of the first element of the list changes. 
Caution: e has to be nonNULL, list can be a NULL pointer
*/
struct VertexList* push(struct VertexList* list, struct VertexList* e) {
	e->next = list;
	return e;
}



/********************************************************************************************************
******************************* Methods that deal with Vertices ***************************************
********************************************************************************************************/



/** 
This method clones a vertex in the sense that the returned vertex has the same number
and label. The other fields remain initialized with zero/null values
*/
struct Vertex* shallowCopyVertex(struct Vertex *v, struct VertexPool *p) {
	struct Vertex *new = getVertex(p);
	new->number = v->number;
	new->label = v->label;
	return new;
}

int degree(struct Vertex* v) {
	struct VertexList* e;
	int deg = 0;
	for (e=v->neighborhood; e!=NULL; e=e->next) {
		++deg;
	}
	return deg;
}

char isLeaf(struct Vertex* v) {
	/* check if v has a neigbor at all */
	if (v->neighborhood) {
		/* check if v has exactly one neighbor, thus is a leaf */
		if (v->neighborhood->next == NULL) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}


/**
 * Given two vertices v, w this method returns the number of common neighbors.
 * For this, it uses the ->visited member of the vertices, so don't use this method
 * in a context where this member is used to store other information
 *
 * Assertions:
 * input: ->visited = 0 for all vertices in the graph
 * output: ->visited = 0 for all vertices in the graph if the input assertion holds.
 */
int commonNeighborCount(struct Vertex* v, struct Vertex* w) {
	struct VertexList* e;
	int count = 0;

	for (e=v->neighborhood; e; e=e->next) {
		e->endPoint->visited = 1;
	}
	for (e=w->neighborhood; e; e=e->next) {
		/* if the endpoint is visited, then there is a triangle
		 * if there are more than two triangles, break */
		if (e->endPoint->visited) {
			++count;
		}
	}
	for (e=v->neighborhood; e; e=e->next) {
		e->endPoint->visited = 0;
	}
	return count;
}


/**
 * Check if the vertex w is in the adjacency list of v.
 * As we consider undirected graphs, the name of the method fits.
 */
char isIncident(struct Vertex* v, struct Vertex* w) {
	struct VertexList* e;

	for (e=v->neighborhood; e; e=e->next) {
		if (e->endPoint == w) {
			return 1;
		}
	}

	return 0;
}


/**
 * Check if the vertex has degree 2 in O(1).
 */
char isDegreeTwoVertex(struct Vertex* v) {
	struct VertexList* idx;
	int delta = 0;

	/* check if the current vertex is of degree 2. */
	for (idx=v->neighborhood; idx; idx=idx->next) {
		if (delta < 3) {
			++delta;
		} else {
			break;
		}
	}

	return (delta == 2);
}


/**
Add edge to the adjacency list of a specified vertex. e and v have to be nonNULL.
*/
void addEdge(struct Vertex* v, struct VertexList* e) {
	v->neighborhood = push(v->neighborhood, e);
}


/**
 * Removes vertex w from the adjacency list of vertex v
 */
void removeEdge(struct Vertex* v, struct Vertex* w, struct ListPool* p) {
	struct VertexList* idx, *tmp;

	if (v->neighborhood->endPoint == w) {
		tmp = v->neighborhood;
		v->neighborhood = v->neighborhood->next;
		dumpVertexList(p, tmp);
		return;
	}

	tmp = v->neighborhood;
	for (idx=tmp->next; idx; idx=idx->next, tmp=tmp->next) {
		if (idx->endPoint == w) {
			tmp->next = idx->next;
			dumpVertexList(p, idx);
			return;
		}
	}
}



/********************************************************************************************************
******************************* Methods that deal with Graphs ****************************************
********************************************************************************************************/


/** Delete the directed edge between vertices v and w in g and return it */
struct VertexList* deleteEdge(struct Graph* g, int v, int w) {
	struct VertexList* e = g->vertices[v]->neighborhood;
	struct VertexList* f = e->next;

	if (e->endPoint->number == w) {
		g->vertices[v]->neighborhood = e->next;
		e->next = NULL;
		return e;
	}

	for ( ; f!=NULL; e=f, f=f->next) {
		if (f->endPoint->number == w) {
			e->next = f->next;
			f->next = NULL;
			return f;
		}
	}
	/* returns NULL, if there is no edge between v and w */
	return NULL;
}

void deleteEdgeBetweenVertices(struct Graph* g, struct VertexList* idx, struct GraphPool* gp) {
	struct VertexList* tmp = deleteEdge(g, idx->startPoint->number, idx->endPoint->number);
	dumpVertexList(gp->listPool, tmp);
	tmp = deleteEdge(g, idx->endPoint->number, idx->startPoint->number);
	if (tmp != NULL) {
		--(g->m);
	}
	dumpVertexList(gp->listPool, tmp);
}

/**
Delete the edges in list from g. Edges are considered to be undirected. That means,
if e=(v,w) is present, this method tries to remove (v,w) and (w,v) from g.
*/
void deleteEdges(struct Graph* g, struct ShallowGraph* list, struct GraphPool* gp) {
	struct VertexList* idx;

	for (idx=list->edges; idx!=NULL; idx=idx->next) {
		deleteEdgeBetweenVertices(g, idx, gp);
	}
}


/**
 * Creates a hard copy of g. Vertices and edges have the same numbers and
 * labels but are different objects in memory. Thus altering (deleting, adding etc.)
 * anything in the copy does not have any impact on g.
 *
 * Deleting a vertex or an edge in g, however, may result in a memory leak, as vertices
 * and edges of the copy reference the label strings of the original structures.
 *
 * TODO a similar method that can handle induced subgraphs properly.
 */
struct Graph* cloneGraph(struct Graph* g, struct GraphPool* gp) {
	struct Graph* copy = getGraph(gp);
	int i;

	copy->activity = g->activity;
	copy->m = g->m;
	copy->n = g->n;
	copy->number = g->number;

	copy->vertices = malloc(g->n * sizeof(struct Vertex*));

	/* copy vertices */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			copy->vertices[i] = shallowCopyVertex(g->vertices[i], gp->vertexPool);
		}
	}

	/* copy edges */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			struct VertexList* e;
			for (e=g->vertices[i]->neighborhood; e; e=e->next) {
				struct VertexList* tmp = getVertexList(gp->listPool);
				tmp->endPoint = copy->vertices[e->endPoint->number];
				tmp->startPoint = copy->vertices[e->startPoint->number];
				tmp->label = e->label;

				/* add the shallow copy to the new graph */
				tmp->next = copy->vertices[e->startPoint->number]->neighborhood;
				copy->vertices[e->startPoint->number]->neighborhood = tmp;
			}
		}
	}
	return copy;
}


/**
 * Due to implementation, there can be some graphs that contain less vertices
 * than positions in their ->vertices array.
 *
 * This method creates a hard copy of the part of g that is actually there.
 * Vertices may get different numbers, but the induced subgraph of g that is
 * actually there and the copy are isomorphic as labeled graphs.
 * Vertices and edges of the copy are different objects in memory.
 * Thus altering (deleting, adding etc.) anything in the copy does not
 * have any impact on g.
 *
 * Deleting a vertex or an edge in g, however, results in a memory leak, as vertices
 * and edges of the copy reference the label strings of the original structures.
 *
 *
 */
struct Graph* cloneInducedGraph(struct Graph* g, struct GraphPool* gp) {
	struct Graph* copy = getGraph(gp);
	struct Vertex** tmp;
	int i, j;
	int actualVertices = 0;

	copy->activity = g->activity;
	copy->m = g->m;
	copy->number = g->number;

	setVertexNumber(copy, g->n);


	/* copy vertices */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			copy->vertices[i] = shallowCopyVertex(g->vertices[i], gp->vertexPool);
			++actualVertices;
		}
	}

	/* copy edges */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			struct VertexList* e;
			for (e=g->vertices[i]->neighborhood; e; e=e->next) {
				struct VertexList* tmp = getVertexList(gp->listPool);
				tmp->endPoint = copy->vertices[e->endPoint->number];
				tmp->startPoint = copy->vertices[e->startPoint->number];
				tmp->label = e->label;

				/* add the shallow copy to the new graph */
				tmp->next = copy->vertices[e->startPoint->number]->neighborhood;
				copy->vertices[e->startPoint->number]->neighborhood = tmp;
			}
		}
	}

	/* up to this point, there may be elements of copy->vertices, that are NULL */
	tmp = copy->vertices;

	/* set the number of vertices correctly */
	setVertexNumber(copy, actualVertices);

	/* shift the vertices that are there into the new array and
	 * assign new numbers */
	j = 0;
	for (i=0; i<g->n; ++i) {
		if (tmp[i]) {
			copy->vertices[j] = tmp[i];
			copy->vertices[j]->number = j;
			++j;
		}
	}

	free(tmp);

	return copy;
}


/**
 * create a graph that is isomorphic to the induced subgraph defined by edgeList
 */
struct Graph* shallowGraphToGraph(struct ShallowGraph* edgeList, struct GraphPool* gp) {
	struct Graph* g = getGraph(gp);
	struct VertexList* e;
	int n = 0;
	int i;

	/* clear all ->lowPoint s */
	for (e=edgeList->edges; e; e=e->next) {
		e->startPoint->lowPoint = 0;
		e->endPoint->lowPoint = 0;
	}

	/* count number of distinct vertices
	 * and number vertices accordingly*/
	for (e=edgeList->edges; e; e=e->next) {
		if (e->startPoint->lowPoint == 0) {
			++n;
			e->startPoint->lowPoint = n;
		}
		if (e->endPoint->lowPoint == 0) {
			++n;
			e->endPoint->lowPoint = n;
		}
	}

	/* set vertex number of new Graph to n, initialize stuff*/
	setVertexNumber(g, n);
	g->m = edgeList->m;

	for (i=0; i<n; ++i) {
		g->vertices[i] = getVertex(gp->vertexPool);
		g->vertices[i]->number = i;
	}

	/* add copies of edges and labels of vertices */
	for (e=edgeList->edges; e; e=e->next) {
		struct VertexList* f = getVertexList(gp->listPool);
		f->startPoint = g->vertices[e->startPoint->lowPoint - 1];
		f->endPoint = g->vertices[e->endPoint->lowPoint - 1];
		f->label = e->label;
		f->startPoint->label = e->startPoint->label;
		f->endPoint->label = e->endPoint->label;

		addEdge(g->vertices[e->startPoint->lowPoint - 1], f);
		addEdge(g->vertices[e->endPoint->lowPoint - 1], inverseEdge(f, gp->listPool));
	}

	return g;
} 


char isIncident(struct Graph* g, int v, int w) {
	struct VertexList* e;
	if ((g->vertices[v]) && (g->vertices[v]->neighborhood)) {
		for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->number == w) {
				return 1;
			}
		}
	}
	/* if there is no vertex, or no neighborhood at all, or w does not occur
	as endpoint of any edge incident to v, return false */
	return 0;
}


/**
 * Allocates an array of n pointers to vertices and sets the number of vertices of
 * g to n. The array is initialized to NULL.
 */
struct Vertex** setVertexNumber(struct Graph* g, int n) {
	int i;
	
	if (n<0) {
		printf("Error allocating memory for array of negative size %i\n", n);
		return NULL;
	}
	
	g->n = n;
	g->vertices = malloc(n * sizeof(struct Vertex*));

	for (i=0; i<n; ++i) {
		g->vertices[i] = NULL;
	}

	return g->vertices;
}


/**
 * Given a graph, this method returns a ShallowGraph containing one copy of each edge.
 * Due to implementation, edges in the output are s.t. startPoint->number < endPoint->number
 * and are sorted by startPoint->number. Note that the order of endPoints for some fixed startPoint
 * depends on the order of startPoint->neighborhood. I.e. the order of the output is not necessarily
 * sorted lexicographically.
 */
struct ShallowGraph* getGraphEdges(struct Graph *g, struct ShallowGraphPool* sgp) {
	int i;
	struct ShallowGraph* edges = getShallowGraph(sgp);
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			struct VertexList *idx;
			for (idx = g->vertices[i]->neighborhood; idx; idx = idx->next) {
				if (idx->endPoint->number > i) {
					pushEdge(edges, shallowCopyEdge(idx, sgp->listPool));
				}
			}
		}
	}

	return edges;
}

/**
Add an undirected edge between vertex v and vertex w in g with label label
*/
void addEdgeBetweenVertices(int v, int w, char* label, struct Graph* g, struct GraphPool* gp) {
	struct VertexList* e = getVertexList(gp->listPool);
	e->startPoint = g->vertices[v];
	e->endPoint = g->vertices[w];
	e->label = label;

	addEdge(g->vertices[v], e);
	addEdge(g->vertices[w], inverseEdge(e, gp->listPool));

	++(g->m);
}

void addEdges(struct Graph* g, struct ShallowGraph* list, struct GraphPool* gp) {
	struct VertexList* e;

	for (e=list->edges; e!=NULL; e=e->next) {
		addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, g, gp);
	}
}

/**
Create a graph without edges that has the same vertex set as g.
The vertices in the new graph are independent copies of the vertices
of g and share only label and number.
*/
struct Graph* emptyGraph(struct Graph* g, struct GraphPool* gp) {
	struct Graph* empty = getGraph(gp);
	int v;
	setVertexNumber(empty, g->n);

	for (v=0; v<g->n; ++v) {
		empty->vertices[v] = shallowCopyVertex(g->vertices[v], gp->vertexPool);
	}
	return empty;
}


/**
Create a graph without edges that has n vertices.
*/
struct Graph* createGraph(int n, struct GraphPool* gp) {
	struct Graph* empty = getGraph(gp);
	int v;
	setVertexNumber(empty, n);

	for (v=0; v<n; ++v) {
		empty->vertices[v] = getVertex(gp->vertexPool);
		empty->vertices[v]->number = v;
	}
	return empty;
}



/********************************************************************************************************
******************************* Methods that deal with ShallowGraphs ********************************
********************************************************************************************************/

/**
Given a list of ShallowGraphs that have edges where the start- and endPoints
are pointers to some graph, reset these pointers to point to the vertices of
newBase with the same number. i.e. 
e->startPoint = newBase->vertices[e->startPoint->number];
Expects list to be a list, not a cycle.
*/
void rebaseShallowGraph(struct ShallowGraph* list, struct Graph* newBase) {
	struct ShallowGraph* idx;
	struct VertexList* e;

	for (idx=list; idx!=NULL; idx=idx->next) {
		for (e=idx->edges; e!=NULL; e=e->next) {
			e->startPoint = newBase->vertices[e->startPoint->number];
			e->endPoint = newBase->vertices[e->endPoint->number];
		}
	}
}


/** 
This method should take two ShallowGraph Cycles and return a cycle that is some concatenation
of the two input cycles
*/
struct ShallowGraph* addComponent(struct ShallowGraph* g, struct ShallowGraph* h) {
	/* if g and h are both cycles, concatenate them */
	if (g && h) {	
		struct ShallowGraph* tmp = g->prev;
	
		g->prev->next = h;
		g->prev = h->prev;
		h->prev->next = g;
		h->prev = tmp;
		return g;
	} else {
		/* if one of the two inputs is NULL, return the other without change */
		return (g) ? g : h;
	}
}

struct ShallowGraph* cloneShallowGraph(struct ShallowGraph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* copy = getShallowGraph(sgp);
	struct VertexList* edgeList = NULL;
	struct VertexList* e;
	int m = 0;
	for (e=g->edges; e!=NULL; e=e->next) {
		if (edgeList == NULL) {
			edgeList = shallowCopyEdge(e, sgp->listPool);
			copy->edges = edgeList;
		} else {
			edgeList->next = shallowCopyEdge(e, sgp->listPool);
			edgeList = edgeList->next;
		}
		++m;
	}
	copy->lastEdge = edgeList;
	copy->m = m;

	return copy;
}


/**
 * Adds an edge at the beginning of the edges list of g. Sets the pointer to the last
 * element of this list to e if this was NULL before. And increments the number of edges
 * in the shallow graph.
 *
 * TODO: Usage of this method is safe only if edges and lastEdge are set correctly
 */
void pushEdge(struct ShallowGraph *g, struct VertexList *e) {

	g->edges = push(g->edges, e);
	if (g->lastEdge == NULL) {
		g->lastEdge = e;
	}
	++g->m;
}


/**
 * Returns the first edge in g and updates g accordingly
 * If you try to pop from an empty ShallowGraph, NULL is
 * returned
 */
struct VertexList* popEdge(struct ShallowGraph* g) {
	struct VertexList* e = g->edges;
	if (g->edges) {
		g->edges = g->edges->next;
		--g->m;
		if (g->edges == NULL) {
			g->lastEdge = NULL;
		}
	}
	if (e != NULL) {
		e->next = NULL;
	}
	return e;
}


/**
 * Adds the edge at the end of the list. Only this edge is added, its next pointer is set to NULL
 */
void appendEdge(struct ShallowGraph *g, struct VertexList *e) {

	if (g->lastEdge == NULL) {
		g->edges = push(g->edges, e);
		g->lastEdge = e;
	} else {
		g->lastEdge->next = e;
		e->next = NULL;
		g->lastEdge = e;
	}
	++g->m;
}


/*
 * Ensures, that the lastEdge Pointer in g points to the last element in its edge list.
 * g may be empty, but not NULL.
 */
struct VertexList* assertLastPointer(struct ShallowGraph* g) {
	struct VertexList* idx;

	/* if lastedge is not initialized, set it to the first edge. */
	if (g->lastEdge == NULL) {
		g->lastEdge = g->edges;
	}

	/* move lastedge pointer to the end of the list */
	for (idx=g->lastEdge; idx; idx=idx->next) {
			g->lastEdge = idx;
	}

	return g->lastEdge;
}


/**
 * Returns a copy of the given list of edges, where the order is reversed and start- and
 * endpoints of each edge are switched.
 */
struct ShallowGraph* inverseCycle(struct ShallowGraph* cycle, struct ShallowGraphPool *sgp) {
	struct VertexList *index;
	struct ShallowGraph* inverse = getShallowGraph(sgp);


	for (index=cycle->edges; index; index=index->next) {
		struct VertexList* e = inverseEdge(index, sgp->listPool);
		pushEdge(inverse, e);

		/* set lastEdge pointer to the correct value */
		if (index == cycle->edges) {
			inverse->lastEdge = e;
		}
	}

	return inverse;
}
