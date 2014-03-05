#include <stdio.h>
#include "intMath.h"
#include "listComponents.h"


/**
Traverses a graph and marks all vertices reachable from v with the number given
by the argument component.
@refactor: to dfs.c
 */
void markConnectedComponents(struct Vertex *v, int component) {
	struct VertexList *index;

	/* mark vertex as visited */
	v->visited = component;

	/*recursive call for all neighbors that are not visited so far */
	for (index = v->neighborhood; index; index = index->next) {
		if (!(index->endPoint->visited)) {
			markConnectedComponents(index->endPoint, component);
		}
	}
}



/************************* Find Biconnected Components **************************/


/**
Finds the biconnected components of a graph. 
The stack argument has to contain an artificial vertex which will be interpreted as the head
of a list that is used as a stack. This is necessary to not change the pointer to the first element of the list.

The output of this function will be a doubly connected cycle of ShallowGraph structs which each contain a
list of cloned edges of the graph edges and each represent a biconnected component of the graph.

The ShallowGraphs contain a field m which specifies the number of contained edges. Iff this number is larger
than 1, the component is 2-connected and each edge is contained in a cycle.
 */
struct ShallowGraph* __tarjanFBC(struct Vertex *v, struct Vertex* w, int i, struct VertexList* stack, struct ListPool* lp, struct ShallowGraphPool* gp) {
	struct VertexList *index, *stackEdge;

	struct ShallowGraph* graphList = NULL;
	struct ShallowGraph* tmp;

	/* set the lowest occurrence of a vertex in the current cycle to the current vertex */
	v->lowPoint = i + 1;
	/* assign dfs-numbers */
	v->visited = i + 1;

	for (index = v->neighborhood; index; index = index->next) {
		if (!(index->endPoint->visited)) {
			/* add copy of the current edge to stack of edges, head of the stack remains the head! */
			stack->next = push(stack->next, shallowCopyEdge(index, lp));

			/* recursive call */
			tmp = __tarjanFBC(index->endPoint, v, i + 1, stack, lp, gp);

			/* add output to component list */
			graphList = addComponent(graphList, tmp);

			/* TODO comment */
			v->lowPoint = min(v->lowPoint, index->endPoint->lowPoint);

			if (index->endPoint->lowPoint >= v->visited) {

				/* start new biconnected component */
				struct ShallowGraph* newBiconnectedComponent = getShallowGraph(gp);
				for (stackEdge = stack->next; stackEdge->startPoint->visited >= index->endPoint->visited; stackEdge = stack->next) {
					/* pop stackEdge and add it to the newBiconnectedComponent */
					stack->next = stackEdge->next;
					newBiconnectedComponent->edges = push(newBiconnectedComponent->edges, stackEdge);
					++(newBiconnectedComponent->m);
				}

				/* pop index edge */
				stack->next = stackEdge->next;
				newBiconnectedComponent->edges = push(newBiconnectedComponent->edges, stackEdge);
				++(newBiconnectedComponent->m);

				/* set the prev and next pointers s.t. newBiconnectedComponent is "a cycle of length 1" */
				newBiconnectedComponent->next = newBiconnectedComponent;
				newBiconnectedComponent->prev = newBiconnectedComponent;

				/* add new biconnected component to the list of components */
				graphList = addComponent(graphList, newBiconnectedComponent);
			}
		} else {
			/* if the edge leads to a vertex that was already visited and is not the edge on which we 
			entered the vertex, we have found a cycle and add the edge to the stack. */
			if ((index->endPoint->visited < v->visited) && (index->endPoint->number != w->number)) {
				/* add index to the edge stack */
				stack->next = push(stack->next, shallowCopyEdge(index, lp));
				v->lowPoint = min(v->lowPoint, index->endPoint->visited);
			} 
		}
	}
	return graphList;
}


/**
Convenience method to call Tarjans algorithm for finding all biconnected components of an undirected graph.
Input:
- A graph
- A ListPool struct to manage VertexList structs
- A GraphPool struct to manage Shallowgraph structs

tarjans algorithm to find biconnected components is applied to each connected component of the graph once,
yielding a runtime of O(m+n).
 */
struct ShallowGraph* listBiconnectedComponents(struct Graph* g, struct ShallowGraphPool *gp) {
	int i;

	/* __tarjanFBC needs a static (artificial) head of stack */
	struct VertexList *headOfStack = getVertexList(gp->listPool);

	/* this is where the magic happens. For all vertices: if the vertex is not visited yet,
	 * start find biconnected components for its connected component. This yields a list of
	 * all biconnected components in the graph. */
	struct ShallowGraph *allComponents = NULL;

	/* init ->visited to zero */
	for (i=0; i<g->n; ++i) {
		g->vertices[i]->visited = 0;
	}

	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]->visited == 0) {
			struct ShallowGraph* currentComponents = __tarjanFBC(g->vertices[i], g->vertices[i], 0, headOfStack, gp->listPool, gp);
			allComponents = addComponent(allComponents, currentComponents);
		}
	}


	/* __tarjanFBC returns a cycle of ShallowGraphs. Convert it to a list */
	if (allComponents) {
		allComponents->prev->next = NULL;
		allComponents->prev = NULL;
	} else {
		/* this case should not occur */
		printf("Error: No cycle of shallow graphs after __tarjanFBC\n");
	}

	/* garbage collection */
	dumpVertexList(gp->listPool, headOfStack);

	/*return results */
	return allComponents;
}


/**
Assumes that vertices are numbered from 0 to n-1 and ordered accordingly in original->vertices
returns a struct Graph which represents the forest obtained from original by removing the union of all cycles in original.
the next field of that struct points to a second graph struct which is exactly this union.

runtime: O(2n + 2m + k) where k is the number of ShallowGraph structs. We have k<=m

This function consumes h completely. Do not attempt to dump or free list after calling this method.
 */
struct Graph* partitionIntoForestAndCycles(struct ShallowGraph* list, struct Graph *original, struct GraphPool* gp, struct ShallowGraphPool *sgp) {

	struct Graph *forest = getGraph(gp);
	struct Graph *cycles = NULL;
	int i;
	int componentNumber = 0;

	/* create create empty forest with same vertex set as original */
	if (!(setVertexNumber(forest, original->n))) {
		printf("Error allocating vertex arrays for forest or cycles in graph %i\n", original->number);
		dumpGraph(gp, forest);
		dumpGraph(gp, cycles);
		return NULL;
	}

	for (i=0; i<original->n; ++i) {
		forest->vertices[i] = shallowCopyVertex(original->vertices[i], gp->vertexPool);
	}


	while (1) {
		if (list->m == 1) {
			/* add the edge contained in this graph to the forest */
			struct VertexList* rev;

			/* update start and endPoint to the new copies of these vertices */
			list->edges->startPoint = forest->vertices[list->edges->startPoint->number];
			list->edges->endPoint = forest->vertices[list->edges->endPoint->number];

			/* add edge to incidence list of its new startpoint */
			addEdge(list->edges->startPoint, list->edges);

			/* clone edge and reverse it */
			rev = shallowCopyEdge(list->edges, gp->listPool);
			rev->startPoint = list->edges->endPoint;
			rev->endPoint = list->edges->startPoint;
			addEdge(rev->startPoint, rev);

			/* pop edge */
			list->edges = NULL;

			/* update edge count of forest */
			forest->m += 1;

		} else {
			struct Graph* newCycle = getGraph(gp);

			/* TODO bad thing: o(n) for each biconnected component */
			setVertexNumber(newCycle, original->n);

			/* append the graph to the list of cycle unions, encode the biconnected component
			 * some edge belongs to in ->used */
			while (list->edges) {
				struct VertexList *rev; 
				struct VertexList *curr = list->edges;

				/* add vertices on the fly */
				if(newCycle->vertices[curr->startPoint->number] == NULL) {
					newCycle->vertices[curr->startPoint->number] = shallowCopyVertex(curr->startPoint, gp->vertexPool);
				}
				if(newCycle->vertices[curr->endPoint->number] == NULL) {
					newCycle->vertices[curr->endPoint->number] = shallowCopyVertex(curr->endPoint, gp->vertexPool);
				}

				/* update start and endPoint to the new copies of these vertices */
				curr->startPoint = newCycle->vertices[list->edges->startPoint->number];
				curr->endPoint = newCycle->vertices[list->edges->endPoint->number];

				/* pop curr (has to be done before appending it to its new list) */
				list->edges = curr->next;

				/* set indicator of biconnected component */
				curr->used = componentNumber;

				/* add edge to incidence list of its new startpoint */
				addEdge(curr->startPoint, curr);

				/* clone edge and reverse it */
				rev = shallowCopyEdge(curr, gp->listPool);
				rev->startPoint = curr->endPoint;
				rev->endPoint = curr->startPoint;
				addEdge(rev->startPoint, rev);

				/* update edge count of cycles */
				newCycle->m += 1;
			}
			++componentNumber;

			/* add the new cyclic component to the list of biconnected components */
			newCycle->next = cycles;
			cycles = newCycle;

		}

		/* breaking condition and iteration of list pointer, dumping of sucked out ShallowGraphs */
		if (list->next) {
			list = list->next;
			dumpShallowGraph(sgp, list->prev);
		} else {
			/* if there is no next element, the current element has to be dumped and the loop has to end */
			dumpShallowGraph(sgp, list);
			break;
		}
	}

	/* it may happen, that cycles == NULL, if the input graph is a forest, nevertheless
	 * forest != NULL in any case */
	forest->next = cycles;
	return forest;
}
