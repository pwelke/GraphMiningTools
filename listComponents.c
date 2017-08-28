#include <stdio.h>
#include <malloc.h>
#include <limits.h>

#include "intMath.h"
#include "listComponents.h"


/**
Compute the criticality of every vertex. 
I.e. the number of connected components of G-v for each vertex v.

The algorithm uses that there is exactly one connected component in G-v 
for every biconnected component of G that contains v.
*/
int* computeCriticality(struct ShallowGraph* biconnectedComponents, int n) {

	/* store for each vertex if the current bic.comp was already counted */
	int* occurrences = malloc(n * sizeof(int));
	/* store the cycle degrees of each vertex in g */
	int* cycleDegrees = malloc(n * sizeof(int));

	int v;
	struct ShallowGraph* comp;
	int compNumber = 0;

	for (v=0; v<n; ++v) {
		occurrences[v] = -1;
		cycleDegrees[v] = 0;
	}


	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		struct VertexList* e;
		for (e=comp->edges; e!=NULL; e=e->next) {
			if (occurrences[e->startPoint->number] < compNumber) {
				occurrences[e->startPoint->number] = compNumber;
				++cycleDegrees[e->startPoint->number];
			}
			if (occurrences[e->endPoint->number] < compNumber) {
				occurrences[e->endPoint->number] = compNumber;
				++cycleDegrees[e->endPoint->number];
			}
		}
		++compNumber;				
	}
	free(occurrences);
	return cycleDegrees;
}


int* computeCycleDegrees(struct ShallowGraph* biconnectedComponents, int n) {

	/* store for each vertex if the current bic.comp was already counted */
	int* occurrences = malloc(n * sizeof(int));
	/* store the cycle degrees of each vertex in g */
	int* cycleDegrees = malloc(n * sizeof(int));

	int v;
	struct ShallowGraph* comp;
	int compNumber = 0;

	for (v=0; v<n; ++v) {
		occurrences[v] = -1;
		cycleDegrees[v] = 0;
	}

	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			struct VertexList* e;
			for (e=comp->edges; e!=NULL; e=e->next) {
				if (occurrences[e->startPoint->number] < compNumber) {
					occurrences[e->startPoint->number] = compNumber;
					++cycleDegrees[e->startPoint->number];
				}
				if (occurrences[e->endPoint->number] < compNumber) {
					occurrences[e->endPoint->number] = compNumber;
					++cycleDegrees[e->endPoint->number];
				}
			}
			++compNumber;
		}			
	}
	free(occurrences);
	return cycleDegrees;
}


int getMaxCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp) {
	int maxDegree = -1;
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	int* cycleDegrees = computeCycleDegrees(biconnectedComponents, g->n);

	int v;
	for (v=0; v<g->n; ++v) {
		if (maxDegree < cycleDegrees[v]) {
			maxDegree = cycleDegrees[v];
		}
	}

	free(cycleDegrees);
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return maxDegree;
}


int getMinCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp) {
	int minDegree = INT_MAX;
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	int* cycleDegrees = computeCycleDegrees(biconnectedComponents, g->n);

	int v;
	for (v=0; v<g->n; ++v) {
		if (minDegree > cycleDegrees[v]) {
			minDegree = cycleDegrees[v];
		}
	}

	free(cycleDegrees);
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return minDegree;
}


/**
Count the number of biconnected components that are not a bridge.
*/
int getNumberOfBiconnectedComponents(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	int compNumber = 0;
	for (struct ShallowGraph* comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
			++compNumber;
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return compNumber;
}


/**
Count the number of biconnected components that are not a bridge.
*/
int getNumberOfBlocks(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	int compNumber = 0;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			++compNumber;
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return compNumber;
}


/**
Count the number of edges in the graph that are bridges. 
I.e. count the number of biconnected components with only one edge.
*/
int getNumberOfBridges(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	int bridgeNumber = 0;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m == 1) {
			++bridgeNumber;
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return bridgeNumber;
}


/**
Count the number of connected components in the graph obtained from g by removing
all block edges (i.e. all edges that are not bridges)
*/
int getNumberOfBridgeTrees(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	int nConnectedComponents = getAndMarkConnectedComponents(forest);
	dumpGraphList(gp, forest);
	return nConnectedComponents;
}



/**
Traverses a graph and marks all vertices reachable from v with the number given
by the argument component. Return the number of edges that visited. Note, that this
is twice the number of edges in the connected component, due to standard reasons.

Careful: To avoid infinite runtime, the method tests if a visited vertex has ->visited == component.
Thus, either initialize ->visited's  with some value < 0 or start component counting with 1.
 */
int markAndStoreConnectedComponent(struct Vertex *v, struct Graph* copy, int component) {
	struct VertexList *index;
	int m = 0;

	/* mark vertex as visited */
	v->visited = component;
	// store pointer to vertex in copy graph
	copy->vertices[v->number] = v;

	/*recursive call for all neighbors that are not visited so far */
	for (index = v->neighborhood; index; index = index->next) {
		++m;
		if (index->endPoint->visited != component) {
			m += markAndStoreConnectedComponent(index->endPoint, copy, component);
		}
	}
	return m;
}


/** Return a list of all connected components as struct Graph*'s.

Attention: Bad Runtime: O(n * c) where c is number of components!
Could be implemented in O(n).
*/
struct Graph* listConnectedComponents(struct Graph* g, struct GraphPool* gp) {
	struct Graph* components = NULL;
	int v;
	int componentNumber = 0;

	struct Graph* copy = getGraph(gp);
	setVertexNumber(copy, g->n);

	for (v=0; v<g->n; ++v) {
		g->vertices[v]->visited = -1;
	}
	for (v=0; v<g->n; ++v) {
		if (g->vertices[v]->visited == -1) {
			int m = -1;
			int w;
			struct Graph* currentComponent;

			m = markAndStoreConnectedComponent(g->vertices[v], copy, componentNumber);
			currentComponent = cloneInducedGraph(copy, gp);
			currentComponent->m = m / 2;
			currentComponent->next = components;
			components = currentComponent;
			
			for (w=0; w<copy->n; ++w) {
				copy->vertices[w] = NULL;
			}

			++componentNumber;
		}
	}
	dumpGraph(gp, copy);
	return components;
}


/**
Traverses a graph and marks all vertices reachable from v with the number given
by the argument component.

Careful: To avoid infinite runtime, the method tests if a visited vertex has ->visited == component.
Thus, either initialize ->visited's  with some value < 0 or start component counting with 1.
 */
void markConnectedComponent(struct Vertex *v, int component) {
	struct VertexList *index;

	/* mark vertex as visited */
	v->visited = component;

	/*recursive call for all neighbors that are not visited so far */
	for (index = v->neighborhood; index; index = index->next) {
		if (index->endPoint->visited != component) {
			markConnectedComponent(index->endPoint, component);
		}
	}
}


/**
Mark all connected components with a unique number.
Indexing starts with 0 and is stored in ->visited.
Vertex 0 will always be in connected component 0.
Returns the number of connected components in the graph.
*/
int getAndMarkConnectedComponents(struct Graph* g) {
	int componentNumber = 0;
	int v;
	for (v=0; v<g->n; ++v) {
		g->vertices[v]->visited = -1;
	}
	for (v=0; v<g->n; ++v) {
		if (g->vertices[v]->visited == -1) {
			markConnectedComponent(g->vertices[v], componentNumber);
			++componentNumber;
		}
	}
	return componentNumber;
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
					pushEdge(newBiconnectedComponent, stackEdge);
//					newBiconnectedComponent->edges = push(newBiconnectedComponent->edges, stackEdge);
//					++(newBiconnectedComponent->m);
				}

				/* pop index edge */
				stack->next = stackEdge->next;
				pushEdge(newBiconnectedComponent, stackEdge);
//				newBiconnectedComponent->edges = push(newBiconnectedComponent->edges, stackEdge);
//				++(newBiconnectedComponent->m);

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
		fprintf(stderr, "Error: No cycle of shallow graphs after __tarjanFBC\n");
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
