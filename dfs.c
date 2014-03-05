#include <stdio.h>
#include "intMath.h"
#include "dfs.h"


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


/********************************************************************************/
/**************************** THE GOOD STUFF ************************************/
/********************************************************************************/






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
struct ShallowGraph* tarjanFBC(struct Vertex *v, struct Vertex* w, int i, struct VertexList* stack, struct ListPool* lp, struct ShallowGraphPool* gp) {
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
			tmp = tarjanFBC(index->endPoint, v, i + 1, stack, lp, gp);

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
struct ShallowGraph* findBiconnectedComponents(struct Graph* g, struct ShallowGraphPool *gp) {
	int i;

	/* tarjanFBC needs a static (artificial) head of stack */
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
			struct ShallowGraph* currentComponents = tarjanFBC(g->vertices[i], g->vertices[i], 0, headOfStack, gp->listPool, gp);
			allComponents = addComponent(allComponents, currentComponents);
		}
	}


	/* tarjanFBC returns a cycle of ShallowGraphs. Convert it to a list */
	if (allComponents) {
		allComponents->prev->next = NULL;
		allComponents->prev = NULL;
	} else {
		/* this case should not occur */
		printf("Error: No cycle of shallow graphs after tarjanFBC\n");
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



/******************************* List all cycles *********************************************/


/**
 * "This procedure explores graph to find an alternate route to s"
 * Assumes v->lowPoint to be zero for all v in V(G) we are allowed to visit.
 * Furthermore it avoids vertices that have visited set to nonzero except
 * for the vertex s.
 */
char findPathIntern(struct Vertex *v, struct Vertex* parent, struct Vertex *target, int allowance, struct ShallowGraph* path, struct ShallowGraphPool *sgp) {
	struct VertexList *idx;
	char found = 0;


	/* this is an exception for the case where the target is the neighbor of parent
	 * and backtrack calls findPath(target, parent, target, ...) */
	if (v == target) {
		return 1;
	}

	if (v->visited == 0) {

		v->lowPoint = 1;

		for (idx=v->neighborhood; idx; idx=idx->next) {

			if (idx->endPoint != parent) {
				/* this "removes" vertices that are too small from adjacency lists */
				if  (idx->endPoint->number >= allowance) {

					/* breaking condition: s is reached second argument avoids "cycles" of lenght 4
					 * of the type target-parent-v-parent-target */
					if (idx->endPoint == target){

						pushEdge(path, shallowCopyEdge(idx, sgp->listPool));
						return 1;

					} else {
						/* otherwise check if the endpoint was already visited. if not try recursively */
						if ((idx->endPoint->lowPoint == 0) && (idx->endPoint->visited == 0)) {

							found = findPathIntern(idx->endPoint, v, target, allowance, path, sgp);

							/* if we have found a path, return this path via path */
							if (found) {
								pushEdge(path, shallowCopyEdge(idx, sgp->listPool));
								break;
							} /* fi found */
						} /* fi not visited */
					} /* esle endPoint = s */
				} /* fi >= allowance */
			} /* fi not parent */
		} /* rof */
	} /* fi */
	return found;
}


void cleanUpLowPoint(struct Vertex*v) {
	struct VertexList* idx;
	v->lowPoint = 0;
	for (idx=v->neighborhood; idx; idx=idx->next) {
		if (idx->endPoint->lowPoint != 0) {
			cleanUpLowPoint(idx->endPoint);
		}
	}
}


char findPath(struct Vertex *v, struct Vertex* parent, struct Vertex *target, int allowance, struct ShallowGraph* path, struct ShallowGraphPool *sgp) {
	char found = findPathIntern(v, parent, target, allowance, path, sgp);
	cleanUpLowPoint(v);
	return found;
}


/**
 * Assumptions: DFS is called if the "algol-global" variable flag is false
 *
 * TODO get rid of one of the return 1 s
 */
char DFS(struct Vertex* v, struct Vertex* parent, struct Vertex* target, int allowance) {
	struct VertexList* idx;
	char flag = 0;

	if(v->number == target->number)
		return 1;

	/* mark current vertex as visited */
	v->d = 1;

	for (idx=v->neighborhood; idx; idx=idx->next) {
		/* do not consider vertices smaller than allowance and not the edge back to the parent, also only from desired bicomp */
		if ((idx->endPoint->number >= allowance) && (idx->endPoint->number != parent->number)) {
			if (!flag) {
				if (idx->endPoint->number == target->number)
					flag = 1;
				else if (idx->endPoint->d == 0)
					flag = DFS(idx->endPoint, v, target, allowance);
			}
		}
	}

	return flag;
}


/*
 * assumptions: vertex->visited = 1 iff vertex is in currentPath.
 * "vertex->visited plays the role of p(vertex) in [Read, Tarjan 1975]"
 */
struct ShallowGraph* backtrack(struct Graph* g, struct Vertex* v, struct Vertex* parent, struct Vertex* s, int allowance,
		struct ShallowGraph* currentPath, struct ShallowGraphPool* sgp) {
	int i,j;
	struct ShallowGraph *path = getShallowGraph(sgp);
	struct ShallowGraph *result = NULL;
	struct VertexList *neighbor, *idx;
	char flag = 0;


	/* while there exists a vertex $ w \in v->neighborhood $ with w > z
	 * s.t. there is a path from w to s which avoids the current path
	 */
	for (neighbor=v->neighborhood; neighbor; neighbor=neighbor->next) {

		/* this "removes" vertices that are too small from adjacency lists
		 * also, dont use the backwards edge to your parent */
		if ((neighbor->endPoint->number >= allowance) && (neighbor->endPoint != parent)) {

			/* " z < w " given some numbering of vertices. I take the one given by ->number.
			 * This only tries to find a path if the first condition holds. should be better,
			 * as the paths will be marked on vertex level. This seems to be not well elaborated
			 * right now..... shit
			 *
			 * z was a strange way to say: for each neighbor of v and even does harm, if incidence
			 * lists are not sorted by endPoint->number */
			if (findPath(neighbor->endPoint, v, s, allowance, path, sgp)) {
				struct Vertex* predecessorInPath = NULL;

				/* the current extension edge should be contained in the path */
				pushEdge(path, shallowCopyEdge(neighbor, sgp->listPool));

				i = 1;

				/* maybe this can be omitted as flag is not global any more*/
				flag = 0;

				/* mark all vertices on the current path as such, the others as others */
				/* TODO this could maybe be made faster by only traversing the vertices that are necessary with dfs */
				for (j=0; j<g->n; ++j) {
					if (g->vertices[j]) {
						g->vertices[j]->d = g->vertices[j]->visited;
					}
				}


				/* add vertices in path to the current path until a choice of ways to reach s is found */
				for (idx=path->edges; idx && (idx->endPoint != s) && !flag; idx=idx->next) {
					struct VertexList *innerVertexNeighbs;

					appendEdge(currentPath, shallowCopyEdge(idx, sgp->listPool));
					idx->endPoint->visited = idx->endPoint->d = 1;


					for (innerVertexNeighbs=idx->endPoint->neighborhood; innerVertexNeighbs; innerVertexNeighbs=innerVertexNeighbs->next) {
						if (innerVertexNeighbs->endPoint->number >= allowance) {
							/* if the endpoint of this edge is not the next vertex on path */
							if (innerVertexNeighbs->endPoint != idx->next->endPoint) {
								/* ...  or the parent */
								if (innerVertexNeighbs->endPoint->number != idx->startPoint->number) {
									/* ... and not visited yet, except if its s */
									if ((innerVertexNeighbs->endPoint->d == 0) || (innerVertexNeighbs->endPoint == s)) {

										/* search for a different way to s than the one given by path */
										flag = DFS(innerVertexNeighbs->endPoint, idx->endPoint, s, allowance);

										if (flag) break;
									}
								}
							}
						} /* fi allowance */

						/* remember the predecessor on the current path so that recursive call to backtrack
						 * knows, which edge not to use. */
						predecessorInPath = idx->startPoint;
					}
					++i;
				}


				/* this is where output is produced */
				if (!flag) {
					/* idx points to the edge closing the cycle, which is not yet included in the output */
					struct VertexList* closingEdge = shallowCopyEdge(idx, sgp->listPool);

					/* clone currentPath */
					struct ShallowGraph* clone = getShallowGraph(sgp);
					/* the output of this function is a cycle of shallow graphs, and addComponent
					 * expects its input, to be a cycle, too.
					 */
					clone->next = clone->prev = clone;

					for (idx=currentPath->edges; idx; idx=idx->next) {
						appendEdge(clone, shallowCopyEdge(idx, sgp->listPool));
					}

					appendEdge(clone, closingEdge);

					/* add it to the results list */
					result = addComponent(result, clone);

				} else {

					struct ShallowGraph* x = backtrack(g, idx->startPoint, predecessorInPath, s, allowance, currentPath, sgp);
					result = addComponent(result, x);
				}


				if (currentPath->edges->startPoint == v) {
					/* if v is the first vertex in the path, dump the whole path */
					struct VertexList* tmp1 = currentPath->edges;
					struct VertexList* tmp2 = currentPath->edges;
					currentPath->edges = NULL;
					currentPath->lastEdge = NULL;
					while(tmp1) {
						tmp1 = tmp1->next;
						tmp2->endPoint->visited = 0;
						dumpVertexList(sgp->listPool, tmp2);
						tmp2=tmp1;
					}
				} else {
					/* go up to v in current path, then delete all following edges and set their visited values to 0.
					 * Also: set the pointer to the last element in path to v */
					for (idx=currentPath->edges; idx; idx=idx->next) {
						if (idx->endPoint == v) {
							struct VertexList* tmp1 = idx->next;
							struct VertexList* tmp2 = idx->next;
							idx->next = NULL;
							currentPath->lastEdge = idx;
							while(tmp1) {
								tmp1 = tmp1->next;
								tmp2->endPoint->visited = 0;
								dumpVertexList(sgp->listPool, tmp2);
								tmp2=tmp1;
							}
							break;
						}
					}
				}

				/* if we go to the next neighbor of v, we want to start a fresh path */
				dumpShallowGraph(sgp, path);
				path = getShallowGraph(sgp);
			}
		}
	}

	/* garbage collection */
	dumpShallowGraph(sgp, path);

	return result;
}


/**
 * Given a biconnected component, list all contained unique cycles. This is the undirected
 * version, which lists each undirected cycle twice (once for every direction).
 *
 * Input: A graph struct containing the edges of a single biconnected component.
 * Due to implementation: Some entries in g->vertices may be NULL.
 *
 * Output: A ShallowGraph cycle of all contained cycles.
 *
 */
struct ShallowGraph* readTarjanListAllCycles(struct Graph *g, struct ShallowGraphPool *sgp) {
	int i;
	struct ShallowGraph* result = NULL;


	/* TODO should be: for each cycle start vertex */
	for (i=0; i<g->n; ++i) {
		/* check if this one belongs to some union of cycles */
		/* TODO remove this as we just compute stuff for cycle start vertices */
		if ((g->vertices[i] != NULL) && (g->vertices[i]->neighborhood != NULL)) {


			struct ShallowGraph *currentPath = getShallowGraph(sgp);
			struct ShallowGraph *newCycle;


			g->vertices[i]->visited = 1;

			newCycle = backtrack(g, g->vertices[i], g->vertices[i], g->vertices[i], i, currentPath, sgp);

			result = addComponent(result, newCycle);

			dumpShallowGraph(sgp, currentPath);
			g->vertices[i]->visited = 0;
		}
	}

	return result;
}
