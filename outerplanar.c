#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "cs_Parsing.h"
#include "cs_Compare.h"
#include "cs_Outerplanar.h"
#include "listComponents.h"
#include "connectedComponents.h"
#include "outerplanar.h"


/**
A tree is a connected graph with m = n-1 edges.
*/
char isTree(struct Graph* g) {
	if (isConnected(g)) {
		return g->m == g->n - 1;
	} else { 
		return 0;
	}
}


/**
A cactus is a connected graph where each block (i.e., a
biconnected component that is not a bridge) is a simple cycle.
Thus, n - 1 == m - numberOfBlocks.
*/
char isCactus(struct Graph* g, struct ShallowGraphPool* sgp) {
	if (isConnected(g)) {
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
		return g->m - compNumber == g->n - 1 ? 1 : 0;
	} else {
		return 0;
	}
}


/**
An outerplanar graph is a graph that can be drawn in the plane such that
(1) edges only intersect at vertices and
(2) each vertex can be reached from the outer face without crossing an edge.

A graph is outerplanar if and only if each of its biconnected components is outerplanar.
*/ 
char isOuterplanarGraph(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	char isOp = 1;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			isOp = isOuterplanarBlockShallow(comp, sgp, gp);
			if (isOp == 0) {
				break;
			}
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return isOp;
}



/**
 * Check if a biconnected graph g with more than one edge is outerplanar 
 * using the algorithm of Sarah Mitchell, except that instead of the linear
 * bucket sort, stdlibs qsort is used.
 *
 * The algorithm alters g but does not dump the remainder.
 *
 * Mitchell, S. [1979]: Linear Algorithms to Recognize Outerplanar and
 * Maximal Outerplanar Graphs, Information Processing Letters Volume 9,
 * number 5, 16.12.1979
 *
 */
char isOuterplanarBlock(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* list = getShallowGraph(sgp);
	struct ShallowGraph* pairs = getShallowGraph(sgp);
	struct ShallowGraph* edges = getGraphEdges(g, sgp);
	struct VertexList* node = NULL;
	struct VertexList* pair = NULL;
	struct VertexList** edgeArray;
	struct VertexList** pairArray;
	struct VertexList* idx;
	int i, j, k;
	int n = 0;
	char found;


	/*
	 * Get a list "list" of vertices of degree 2 and mark them as such in the graph
	 * by setting ->d = 1. Also, count the number of not-NULL vertices.
	 * This is useful if the algorithm gets some "induced subgraph" where some
	 * vertices are not used.
	 */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {

			++n;

			/* if vertex has degree 2, add it to list and mark it as such */
			if (isDegreeTwoVertex(g->vertices[i])) {
				struct VertexList* e = getVertexList(sgp->listPool);
				e->endPoint = g->vertices[i];
				pushEdge(list, e);

				g->vertices[i]->d = 1;
			}
		}
	}

	/* first check. number of edges ok ? */
	if (g->m > 2 * n - 3) {
		/* printf("incorrect edge count\n"); */
		dumpShallowGraph(sgp, list);
		dumpShallowGraph(sgp, pairs);
		dumpShallowGraph(sgp, edges);
		return 0;
	}

	/* second check: enough vertices of degree 2? */
	if (list->m < 2) {
		/* printf("not enough vertices of degree 2\n"); */
		dumpShallowGraph(sgp, list);
		dumpShallowGraph(sgp, pairs);
		dumpShallowGraph(sgp, edges);
		return 0;
	}

	/*
	 * Successively remove vertices of degree 2
	 */
	for (i=1, node=list->edges; /*node && */  (i<=n-2);  ++i, node=node->next) {

		struct Vertex* v = node->endPoint;

		/* near and next are the two vertices adjacent to node */
		struct Vertex* near = v->neighborhood->endPoint;
		struct Vertex* next = v->neighborhood->next->endPoint;

		/* if the edges leading to near and next are artificial triangulation
		 * edges that were added below in a previous step, add them to the
		 * edges list. Do it s.t. startPoint->number <= endPoint->number, as
		 * there will be lexicographic sorting */
		if (v->neighborhood->flag) {
			if (v->number < near->number) {
				appendEdge(edges, shallowCopyEdge(v->neighborhood, sgp->listPool));
			} else {
				appendEdge(edges, inverseEdge(v->neighborhood, sgp->listPool));
			}
		}
		if (v->neighborhood->next->flag) {
			if (v->number < next->number) {
				appendEdge(edges, shallowCopyEdge(v->neighborhood->next, sgp->listPool));
			} else {
				appendEdge(edges, inverseEdge(v->neighborhood->next, sgp->listPool));
			}
		}


		/* add the pair (near, next) to pairs list, sort lexicographically
		 we have to add pair to the adjacency list of the graph, if this edge
		 does not already exist. Mark newly added edges in the flag tag. */
		pair = getVertexList(sgp->listPool);

		if (near->number < next->number) {

			pair->startPoint = near;
			pair->endPoint = next;

			/* this is for the case where g is not maximal outerplanar and
			 * a "triangulation edge" is added */
			if (!isIncident(near, next)) {
				struct VertexList* tmp = shallowCopyEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(near, tmp);

				tmp = inverseEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(next, tmp);
			}
		} else {

			pair->startPoint = next;
			pair->endPoint = near;

			/* this is for the case where g is not maximal outerplanar and
			 * a "triangulation edge" is added */
			if (!isIncident(near, next)) {
				struct VertexList* tmp = shallowCopyEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(next, tmp);

				tmp =  inverseEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(near, tmp);
			}
		}

		pushEdge(pairs, pair);

		/* remove node from Graph, check if one of the neighbors
		 * becomes degree two vertex. If it is not already in list
		 * (which is encoded in ->d), add it to list.
		 *
		 * TODO ?
		 * BUG: not degree two vertex, but 2-vertex. this is a difference
		 * But 2-vertices make no sense in the context of ops */
		removeEdge(near, v, sgp->listPool);
		if (isDegreeTwoVertex(near)) {
			if (!near->d) {
				struct VertexList* e = getVertexList(sgp->listPool);
				e->endPoint = near;
				/* printf("add %i to list of deg 2 vertices\n", near->number); */
				appendEdge(list, e);
				near->d = 1;
			}
		}

		removeEdge(next, v, sgp->listPool);
		if (isDegreeTwoVertex(next)) {
			if (!next->d) {
				struct VertexList* e = getVertexList(sgp->listPool);
				/* printf("add %i to list of deg 2 vertices\n", next->number); */
				e->endPoint = next;
				appendEdge(list, e);
				next->d = 1;
			}
		}


		/* Here, the vertex should be removed from the graph,
		 * but isMaximalOuterplanar has no pointer to a VertexPool. Thus we
		 * only dump the neighborhood of v. */
		dumpVertexListRecursively(sgp->listPool, v->neighborhood);
		v->neighborhood = NULL;


		/* this part may be omitted as it seems to be the case that
		 * "searching the list of pairs for an occurrence of something that
		 * is not in the list of edges includes an abort if there are e.g.
		 * two copies of an edge in pairs, but only one copy f that edge in
		 * edges.
		 *
		 * mistake in the paper. one has to check, if the new edge
		 * is not contained in more than two triangles. As one triangle
		 * is already deleted by removing v there can be only one other
		 * triangle left without violation of this condition in the former
		 * graph */
		if (commonNeighborCount(near, next) > 1) {
			/* printf("removing node %i creates an edge that lies on more than two triangles\n", node->endPoint->number); */
			dumpShallowGraph(sgp, list);
			dumpShallowGraph(sgp, pairs);
			dumpShallowGraph(sgp, edges);
			return 0;
		}



		/* at this point, node->endPoint should be dumped, but this is not necessary
		 * as it is not considered any more and can be handled when dumping the whole
		 * graph
		dumpVertexList(sgp->listPool, popEdge(list)); */

		if (list->m - i < 2) {
			/* printf("removing node %i leaves not enough degree 2 vertices\n", node->endPoint->number); */
			dumpShallowGraph(sgp, list);
			dumpShallowGraph(sgp, pairs);
			dumpShallowGraph(sgp, edges);
			return 0;
		}
	}

	/* add the last edge (near, next) to edges */
	appendEdge(edges, shallowCopyEdge(pair, sgp->listPool));

	/* check for occurrence of something in pairs, that is not in edges
	 * for this, sort the two lists in lexicographic order and sweep through
	 * the two arrays afterwards */
	edgeArray = malloc(edges->m * sizeof(struct VertexList*));
	for (i=0, idx=edges->edges; i<edges->m; ++i, idx=idx->next) {
		edgeArray[i] = idx;
	}

	pairArray = malloc(pairs->m * sizeof(struct VertexList*));
	for (i=0, idx=pairs->edges; i<pairs->m; ++i, idx=idx->next) {
		pairArray[i] = idx;
	}

	qsort(pairArray, pairs->m, sizeof(struct VertexList*), &compareDirectedEdges);
	qsort(edgeArray, edges->m, sizeof(struct VertexList*), &compareDirectedEdges);


	/* sweep */
	for (j=0, k=0, found=1; j<pairs->m; ++j) {

		/* increment currentEdge as long as it is lex. smaller than sweeper. */
		for (; compareDirectedEdges(&(pairArray[j]), &(edgeArray[k])) > 0; ++k);

		/* check if the two are equal */
		if (compareDirectedEdges(&(pairArray[j]), &(edgeArray[k])) == 0) {
			++k;
			continue;

		} else {
			/* this is bad, g is no mop */
			found = 0;
			break;
		}
	}

	/* garbage collection */
	dumpShallowGraph(sgp, list);
	dumpShallowGraph(sgp, pairs);
	dumpShallowGraph(sgp, edges);
	free(edgeArray);
	free(pairArray);

	return found;
}

/**
 * convenience method to apply Mitchells test if g is a outerplanar biconnected component to a graph given as
 * ShallowGraph struct (i.e. a list of edges)
 *
 * Does not change original.
 */
char isOuterplanarBlockShallow(struct ShallowGraph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct Graph* g = shallowGraphToGraph(original, gp);
	char isOP = isOuterplanarBlock(g, sgp);
	dumpGraph(gp, g);
	return isOP;
}


/**
 * Create a Block and Bridge Tree struct from given input.
 * For constant time access, the list of blocks is converted to an array.
 *
 * If the underlying graph was a tree i.e. |V(blocks)| = 0, blocks is dumped
 * and bbTree->blocks = bbTree->blockComponents = NULL
 */
struct BBTree* createBBTree(struct Graph* tree, struct Graph* blocks, struct ShallowGraph* blockList, struct GraphPool* gp) {
	int i;
	struct BBTree* result = malloc(sizeof(struct BBTree));

	result->tree = tree;
	if (blocks->n > 0) {
		result->blocks = blocks;
		result->blockComponents = malloc(blocks->n * sizeof(struct ShallowGraph*));

		for (i=0; i<blocks->n; ++i) {
			result->blockComponents[i] = blockList;
			blockList = blockList->next;
		}
	} else {
		dumpGraph(gp, blocks);
		result->blockComponents = NULL;
		result->blocks = NULL;
	}

	return result;
}


/**
 * Destructor of BBTree structs. Dumps the Graph structs contained in the BBTree,
 * the ShallowGraph structs representing the components and frees the blockComponents
 * array before freeing the BBTree struct itself.
 */
void dumpBBTree(struct GraphPool* gp, struct ShallowGraphPool* sgp, struct BBTree* tree) {
	if (tree) {
		dumpGraph(gp, tree->tree);
		if (tree->blocks) {
			dumpGraph(gp, tree->blocks);
			dumpShallowGraphCycle(sgp, tree->blockComponents[0]);
			free(tree->blockComponents);
		}
		free(tree);
	}
}


struct VertexList* getContainmentEdge(struct ListPool* lp) {
	struct VertexList* e = getVertexList(lp);
	e->label = malloc(2*sizeof(char));
	sprintf(e->label, "#");
	e->isStringMaster = 1;
	return e;
}


/**
 * input: a doubly linked list, which is consumed.
 * the original graph (which is not consumed)
 *
 * This algorithm returns a block an bridge graph as defined in [1] if
 * the original graph is outerplanar or NULL, if not.
 *
 * [1] Horváth, T., Ramon, J., Wrobel, S. [2010]:
 * Frequent subgraph mining in outerplanar graphs. Data Mining and Knowledge
 * Discovery 21, p.472-508
 */
struct BBTree* createBlockAndBridgeTree(struct ShallowGraph* list, struct Graph *original, struct GraphPool* gp, struct ShallowGraphPool *sgp) {
	struct ShallowGraph* idx;
	struct ShallowGraph* tmp;
	struct Graph* bbTree = getGraph(gp);
	struct Graph* blocks = getGraph(gp);
	int blockNo = 0;
	int i;

	/* copy V(original) */
	setVertexNumber(bbTree, original->n);
	for (i=0; i<original->n; ++i) {
		bbTree->vertices[i] = shallowCopyVertex(original->vertices[i], gp->vertexPool);
	}

	/* copy all bridges to the new graph and delete them from list;
	 * count the number of blocks */
	for (idx=list; idx; idx=tmp) {

		tmp = idx->next;
		if (idx->m == 1) {
			struct VertexList* e = getVertexList(gp->listPool);

			/* add the edge to the bbTree */
			e->startPoint = bbTree->vertices[idx->edges->startPoint->number];
			e->endPoint = bbTree->vertices[idx->edges->endPoint->number];
			e->label = idx->edges->label;

			addEdge(bbTree->vertices[idx->edges->startPoint->number], e);
			addEdge(bbTree->vertices[idx->edges->endPoint->number], inverseEdge(e, gp->listPool));

			++bbTree->m;

			/* cut idx out of list */
			if (idx->prev) {
				idx->prev->next = tmp;
				if (tmp) {
					tmp->prev = idx->prev;
				}
			} else {
				list = tmp;
				if (tmp) {
					tmp->prev = NULL;
				}
			}

			/* dump idx */
			idx->next = NULL;
			dumpShallowGraph(sgp, idx);
		} else {
			/* we have found a block */
			++blockNo;
		}
	}

	/* add blocks */
	setVertexNumber(blocks, blockNo);
	for (i=0; i<blockNo; ++i) {
		blocks->vertices[i] = getVertex(gp->vertexPool);
		blocks->vertices[i]->number = - (i + 1);
	}

	/* check for outerplanarity of all blocks */
	for (idx=list, i=0; idx; idx=idx->next, ++i) {
		struct ShallowGraph* cString = canonicalStringOfOuterplanarGraph(idx, sgp, gp);
		/* if the current component is not outerplanar,
		 * dump stuff and return NULL
		 * TODO */
		if (cString) {
			blocks->vertices[i]->label = canonicalStringToChar(cString);
			blocks->vertices[i]->isStringMaster = 1;
			dumpShallowGraph(sgp, cString);
		} else {

			/* garbage collection */
			dumpGraph(gp, blocks);
			dumpGraph(gp, bbTree);
			dumpShallowGraphCycle(sgp, list);

			return NULL;
		}
	}

	/* add edges connecting blocks and contained vertices */
	for (idx=list, i=0; idx; idx=idx->next, ++i) {
		struct VertexList* e;
		for (e=idx->edges; e; e=e->next) {

			/* edge  (startPoint, block) if it isn't already there.
			 *
			 * Reason: Each Vertex in a block occurs at least twice below (as
			 * it is on at least one cycle), but we want just one edge connecting
			 * it to the block vertex
			 *
			 * The edge isnt already there, if there is no edge at all OR the endpoint
			 * of the previous edge is the block vertex of the current block */
			if ((bbTree->vertices[e->startPoint->number]->neighborhood == NULL)
					|| (bbTree->vertices[e->startPoint->number]->neighborhood->endPoint != blocks->vertices[i])) {

				struct VertexList* f = getContainmentEdge(gp->listPool);
				f->startPoint = bbTree->vertices[e->startPoint->number];
				f->endPoint = blocks->vertices[i];
				addEdge(bbTree->vertices[e->startPoint->number], f);
				/* addEdge(blocks->vertices[i], inverseEdge(f, gp->listPool));
				 * ... is not done as there is the possibility that we have to remove
				 * the vertex startPoint if it is contained in only one block and no bridge */
			}


			/* edge (endPoint, block) if ...*/
			if ((bbTree->vertices[e->endPoint->number]->neighborhood == NULL)
					|| (bbTree->vertices[e->endPoint->number]->neighborhood->endPoint != blocks->vertices[i])) {
				struct VertexList* f = getContainmentEdge(gp->listPool);
				f->startPoint = bbTree->vertices[e->endPoint->number];
				f->endPoint = blocks->vertices[i];
				addEdge(bbTree->vertices[e->endPoint->number], f);
				/* addEdge(blocks->vertices[i], inverseEdge(f, gp->listPool));
				 * as above	*/
			}

		}
	}

	/* remove vertices that do not belong to more than one biconnected component
	 * runtime O(n+m) */
	for (i=0; i<bbTree->n; ++i) {
		struct VertexList* e;
		char removeVertex = 1;

		if (bbTree->vertices[i]->neighborhood) {
			int firstNo = bbTree->vertices[i]->neighborhood->endPoint->number;

			/* if vertex belongs to just one comp but this is a bridge, keep the vertex */
			if (firstNo >= 0) {
				continue;
			}

			/* if vertex belongs to more than one comp keep it */
			for (e=bbTree->vertices[i]->neighborhood; e; e=e->next) {
				if (e->endPoint->number != firstNo) {
					removeVertex = 0;
					break;
				}
			}
		} else {
			/* this part only occurs, if there are isolated vertices in original.
			 * However, by definition an outerplanar graph does not have to be connected
			 * so we keep the vertex and obtain a block and bridge forest
			 */
			removeVertex = 0;
		}


		/* check if to remove the vertex. if so, dump vertex and all edges dangling at it.
		 * The corresponding entry in bbTree->vertices will be NULL */
		if (removeVertex) {
			struct VertexList* e;
			struct VertexList* tmp;
			for (e=bbTree->vertices[i]->neighborhood; e; e=tmp) {
				tmp = e->next;
				dumpVertexList(gp->listPool, e);
			}
			dumpVertex(gp->vertexPool, bbTree->vertices[i]);
			bbTree->vertices[i] = NULL;
		}
	}

	/* add reverse edges of the edges connecting a vertex in tree to a block */
	for (i=0; i<bbTree->n; ++i) {
		if (bbTree->vertices[i]) {
			struct VertexList* e;
			for (e=bbTree->vertices[i]->neighborhood; e; e=e->next) {
				if (e->endPoint->number < 0) {
					addEdge(e->endPoint, inverseEdge(e, gp->listPool));
				}
			}
		}
	}

	return createBBTree(bbTree, blocks, list, gp);
}
