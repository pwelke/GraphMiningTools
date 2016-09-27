#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "newCube.h"
#include "graph.h"
#include "listComponents.h"
#include "bipartiteMatching.h"
#include "wilsonsAlgorithm.h"
#include "subtreeIsoUtils.h"
#include "bitSet.h"
#include "cachedGraph.h"
#include "localEasySubtreeIsomorphism.h"

/* return the head of the list or NULL if list is empty.
 * remove head of list
 * (for speeds sake, don't change pointers
 */
static struct ShallowGraph* popShallowGraph(struct ShallowGraph** list) {
	struct ShallowGraph* head = *list;
	if (head != NULL) {
		*list = (*list)->next;
		head->next = NULL;
	}
	return head;
}

static int lowPointComparator(const void* a, const void* b) {
	struct Vertex* v = *(struct Vertex**)a;
	struct Vertex* w = *(struct Vertex**)b;

	return v->lowPoint - w->lowPoint;
}

struct BlockTree getBlockTreeT(struct Graph* g, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);

	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->visited = -1; // isRoot
		g->vertices[v]->d = -1; // parent root
	}

	/* we mark for each component the vertex with the lowest lowPoint.
	 * that is the root of the component.
	 *
	 * Due to the order in which listBiconnectedComponents() returns the bics,
	 * we obtain the right parent roots for the roots in this way. */

	for (struct ShallowGraph* bic=biconnectedComponents; bic!=NULL; bic=bic->next) {

		// listBiconnectedComponents returns bics where the first vertex in the first edge is the root of each bic
		struct Vertex* rootOfComponent = bic->edges->startPoint;

		// mark the root of this component as root.
		rootOfComponent->visited = 1;

		// store the root as information with each vertex and the component
		int rootId = rootOfComponent->number;
		bic->data = rootId;
		for (struct VertexList* e=bic->edges; e!=NULL; e=e->next) {
			e->startPoint->d = rootId;
			e->endPoint->d = rootId;
		}
	}

	// create output struct
	struct BlockTree blockTree = {0};
	blockTree.g = g;

	// count number of roots in g, init storage
	for (int v=0; v<g->n; ++v) {
		if (g->vertices[v]->visited == 1) {
			++blockTree.nRoots;
		}
	}
	blockTree.roots = malloc(blockTree.nRoots * sizeof(struct Vertex*));
	blockTree.parents = malloc(blockTree.nRoots * sizeof(struct Vertex*));
	blockTree.vRootedBlocks = malloc(blockTree.nRoots * sizeof(struct ShallowGraph*));

	// select and sort roots by lowpoint. this ensures bottom up traversal of the
	// blocktree if iterating through the array
	for (int v=0, r=0; v<g->n; ++v) {
		if (g->vertices[v]->visited == 1) {
			blockTree.roots[r] = g->vertices[v];
			++r;
		}
	}
	qsort(blockTree.roots, blockTree.nRoots, sizeof(struct Vertex*), &lowPointComparator);

	// add parents of roots to array (after sorting the above, of course)
	// init vRootedBlocks to NULL
	// set root->visited to its position in roots (so that we can map the bics efficiently)
	for (int v=0; v<blockTree.nRoots; ++v) {
		blockTree.parents[v] = g->vertices[blockTree.roots[v]->d];
		blockTree.vRootedBlocks[v] = NULL;
		blockTree.roots[v]->visited = v;
	}

	// add blocks to respective roots
	for (struct ShallowGraph* bic=popShallowGraph(&biconnectedComponents); bic!=NULL; bic=popShallowGraph(&biconnectedComponents)) {
		int rootPositionInBlockTreeArrays = g->vertices[bic->data]->visited;
		bic->next = blockTree.vRootedBlocks[rootPositionInBlockTreeArrays];
		blockTree.vRootedBlocks[rootPositionInBlockTreeArrays] = bic;
	}

	return blockTree;
}

/**
 * Merge two shallow graphs. The the result is the first shallow graph in list with
 * all the edges added, the second shallow graph is dumped.
 */
static struct ShallowGraph* mergeTwoShallowGraphs(struct ShallowGraph* first, struct ShallowGraph* second, struct ShallowGraphPool* sgp) {
	first->lastEdge->next = second->edges;
	first->lastEdge = second->lastEdge;
	first->m += second->m;

	second->edges = second->lastEdge = NULL;
	second->next = NULL;
	dumpShallowGraph(sgp, second);
	return first;
}


/**
 * Merge all shallow graphs in the list. The list is consumed, the result is the first shallow graph in list with
 * all the edges added.
 */
static struct ShallowGraph* mergeShallowGraphs(struct ShallowGraph* list, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* head = popShallowGraph(&list);
	head->next = NULL;

	for (struct ShallowGraph* pop=popShallowGraph(&list); pop!=NULL; pop=popShallowGraph(&list)) {
		mergeTwoShallowGraphs(head, pop, sgp);
	}
	return head;
}


/*
 * transform shallowgraph of v rooted components to graph rooted at v.
 * v will be vertex 0 in the resulting graph
 * w->d in the resulting graph gives the root array index of w, if w is a root, or -1 otherwise.
 * g->vertices[0] is the root (i.e. v in the notation of the paper)
 */
struct Graph* blockConverter(struct ShallowGraph* edgeList, struct GraphPool* gp) {

	int n = 0;

	/* clear all ->lowPoint s */
	for (struct VertexList* e=edgeList->edges; e; e=e->next) {
		e->startPoint->lowPoint = 0;
		e->endPoint->lowPoint = 0;
	}

	/* count number of distinct vertices and number vertices accordingly. */
	for (struct VertexList* e=edgeList->edges; e; e=e->next) {
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
	struct Graph* g = createGraph(n, gp);
	g->m = edgeList->m;


	for (struct VertexList* e=edgeList->edges; e; e=e->next) {
		/* add copies of edges and labels of vertices */
		struct VertexList* f = getVertexList(gp->listPool);
		f->startPoint = g->vertices[e->startPoint->lowPoint - 1];
		f->endPoint = g->vertices[e->endPoint->lowPoint - 1];
		f->label = e->label;
		f->startPoint->label = e->startPoint->label;
		f->endPoint->label = e->endPoint->label;

		addEdge(g->vertices[e->startPoint->lowPoint - 1], f);
		addEdge(g->vertices[e->endPoint->lowPoint - 1], inverseEdge(f, gp->listPool));

		/* the vertices in the original graph (on which edgeList is based upon) have their ->visited values
		 * set to -1 if they are not roots and to the index of the root in sptTree->root, if they are a root.
		 * Transfer this information to the resulting graph. */
		f->startPoint->d = e->startPoint->visited;
		f->endPoint->d = e->startPoint->visited;
	}

	return g;
}


/*
 * transform shallowgraphs of v rooted components to graph rooted at v.
 * shallowgraphs will be consumed and dumped
 * v will be vertex 0 in the resulting graph
 * w->d in the resulting graph gives the root array index of w, if w is a root, or -1 otherwise.
 * g->vertices[0] is the root (i.e. v in the notation of the paper)
 */
struct Graph* spanningTreeConverter(struct ShallowGraph* localTrees, struct Graph* component, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* resultingTrees = NULL;
	for (struct ShallowGraph* tree=popShallowGraph(&localTrees); tree!=NULL; tree=popShallowGraph(&localTrees)) {

		struct Graph* result = emptyGraph(component, gp);
		result->m = tree->m;
		// copy root information
		for (int v=0; v<result->n; ++v) {
			result->vertices[v]->d = component->vertices[v]->d;
		}

		// transfer edges, add to vertices
		rebaseShallowGraph(tree, result);
		for (struct VertexList* e=popEdge(tree); e!=NULL; e=popEdge(tree)) {
			addEdge(e->startPoint, e);
			addEdge(e->endPoint, inverseEdge(e, gp->listPool));
		}
		dumpShallowGraph(sgp, tree);
		result->next = resultingTrees;
		resultingTrees = result;
	}
	return resultingTrees;
}


/**
 * blockTree is comsumed
 */
struct SpanningtreeTree getSpanningtreeTree(struct BlockTree blockTree, int spanningTreesPerBlock, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct SpanningtreeTree sptTree = {0};
	sptTree.g = blockTree.g;
	sptTree.nRoots = blockTree.nRoots;
	sptTree.roots = blockTree.roots;
	sptTree.parents = blockTree.parents;
	sptTree.localSpanningTrees = malloc(sptTree.nRoots * sizeof(struct ShallowGraph*));

	for (int v=0; v<sptTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		// TODO make destructive. shallowGraphs are not used afterwards.
		struct Graph* mergedGraph = blockConverter(mergedEdges, gp);

		struct ShallowGraph* shallowSpanningtrees = NULL;
		if (mergedGraph->m != mergedGraph->n-1) {
			// sample spanning trees according to parameter
			for (int i=0; i<spanningTreesPerBlock; ++i) {
				struct ShallowGraph* spt = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
				spt->next = shallowSpanningtrees;
				shallowSpanningtrees = spt;
			}
		} else {
			// if the mergedGraph is a tree, we only 'sample' one spanning tree
			// TODO make faster
			shallowSpanningtrees = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
		}
		sptTree.localSpanningTrees[v] = spanningTreeConverter(shallowSpanningtrees, mergedGraph, gp, sgp);
//		printf("%i:", sptTree.roots[v]->number);
//		printGraph(sptTree.localSpanningTrees[v]);

		// garbage collection
		dumpShallowGraph(sgp, mergedEdges);
		dumpGraph(gp, mergedGraph);
	}

	//garbage collection
	free(blockTree.vRootedBlocks);

	return sptTree;
}




/* vertices of g have their ->visited values set to the postorder. Thus,
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
static struct Graph* makeBipartiteInstanceFromVerticesForLocalEasyCached(struct SubtreeIsoDataStore data, struct SubtreeIsoDataStore wcharacteristics, struct CachedGraph* cachedB, struct Vertex* u, struct Vertex* w, struct Vertex* wBelow, struct GraphPool* gp) {

	int sizeofX = degree(u);
	int sizeofY = degree(w);
	int sizeofZ = 0;
	if (wBelow) {
		sizeofZ = degree(wBelow);
	}

//	// if one of the neighbors of u is removed from this instance to see if there is a matching covering all neighbors but it,
//	// the bipartite graph must be smaller
//	if (removalVertex->number != u->number) {
//		--sizeofX;
//	}

	struct Graph* B = getCachedGraph(sizeofX + sizeofY + sizeofZ, cachedB);

	/* store size of first partitioning set */
	B->number = sizeofX;

	/* add vertex numbers of original vertices to ->lowPoint of each vertex in B
	and add edge labels to vertex labels to compare edges easily */
	int i = 0;
	for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
//		if (e->endPoint->number != removalVertex->number) {
		B->vertices[i]->lowPoint = e->endPoint->number;
		B->vertices[i]->label = e->label;
		++i;
//		}
	}
	for (struct VertexList* e=w->neighborhood; e!=NULL; e=e->next) {
		B->vertices[i]->lowPoint = e->endPoint->number;
		B->vertices[i]->label = e->label;
		++i;
	}
	if (wBelow) {
		for (struct VertexList* e=wBelow->neighborhood; e!=NULL; e=e->next) {
			B->vertices[i]->lowPoint = e->endPoint->number;
			B->vertices[i]->label = e->label;
			++i;
		}
	}

	int sizeofXY = sizeofX + sizeofY;
	/* add edge (x,y) if u in S(y,x) */
	for (i=0; i<sizeofX; ++i) {
		int x = B->vertices[i]->lowPoint;
		for (int j=sizeofX; j<sizeofXY; ++j) {
			int y = B->vertices[j]->lowPoint;

			/* y has to be a child of v */
			if (data.g->vertices[y]->visited < w->visited) {
				/* edge labels have to match, (v, child)->label in g == (u, child)->label in h
				these values were stored in B->vertices[i,j]->label */
				if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
					if (containsCharacteristic(data, u, data.h->vertices[x], data.g->vertices[y])) {
						addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
						++B->m;
					}
				}
			}
		}
		for (int j=sizeofXY; j<B->n; ++j) {
			int y = B->vertices[j]->lowPoint;

			/* y has to be a child of v */
			if (wcharacteristics.g->vertices[y]->visited < w->visited) {
				/* edge labels have to match, (v, child)->label in g == (u, child)->label in h
				these values were stored in B->vertices[i,j]->label */
				if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
					if (containsCharacteristic(wcharacteristics, u, wcharacteristics.h->vertices[x], wcharacteristics.g->vertices[y])) {
						addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
						++B->m;
					}
				}
			}
		}
	}
	return B;
}


/*
 * compute characteristics for one $\theta \in \Theta_{vw}(\tau)$
 *
 * implementation detail: w is the occurrence of w in \tau,
 * wBelow is the occurrence of w in the spanning tree of the w-rooted components
 */
void computeCharacteristics(struct SubtreeIsoDataStore* current, struct SubtreeIsoDataStore* wCharacteristics, struct CachedGraph* cachedB, struct Vertex* u, struct Vertex* w, struct Vertex* wBelow, struct GraphPool* gp) {

	// compute maximum matching
	struct Graph* B = makeBipartiteInstanceFromVerticesForLocalEasyCached(*current, *wCharacteristics, cachedB, u, w, wBelow, gp);
	int sizeofMatching = bipartiteMatchingEvenMoreDirty(B);
	int nNeighbors = B->number;

	// is there a subgraph iso here?
	if (sizeofMatching == nNeighbors) {
		addCharacteristic(current, u, u, w);
		current->foundIso = 1;

		returnCachedGraph(cachedB);
		dumpCachedGraph(cachedB);
		return; // early termination when subtree iso is found
	}

	// compute partial subgraph isomorphisms
	if (sizeofMatching == nNeighbors - 1) {
		addNoncriticalVertexCharacteristics(current, B, u, w);
	}

	returnCachedGraph(cachedB);
}


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
static void noniterativeLocalEasySubtreeCheck_intern(struct SubtreeIsoDataStore* current, struct SpanningtreeTree sptTree, struct GraphPool* gp) {

	struct Graph* g = current->g;
	struct Graph* h = current->h;

	struct CachedGraph* cachedB = initCachedGraph(gp, h->n);

	current->foundIso = 0;
	for (int wi=0; wi<g->n; ++wi) {
		struct Vertex* w = g->vertices[current->postorder[wi]];

		for (int ui=0; ui<h->n; ++ui) {
			struct Vertex* u = h->vertices[ui];

			// check if vertex labels match
			if (labelCmp(u->label, w->label) != 0) { continue; }

			if (w->d == -1) {
				computeCharacteristics(current, NULL, cachedB, u, w, NULL, gp);
				if (current->foundIso) {
					// todo clean up
					return;
				}

			} else {
				// w is a root, stuff gets complicated

				// we do not process v here
				if (w->number == 0) {
					continue;
				}

				// loop over the spanning trees of the w-rooted components
				for (struct SubtreeIsoDataStoreElement* e=sptTree.characteristics[w->d]->first; e!=NULL; e=e->next) {
					struct Vertex* wBelow = e->data.g->vertices[0];
					computeCharacteristics(current, &(e->data), cachedB, u, w, wBelow, gp);
					if (current->foundIso) {
						// todo clean up
						return;
					}
				}
			}
		}
	}
	dumpCachedGraph(cachedB);
}

void printSptTree(struct SpanningtreeTree sptTree) {
	printf("\nbase graph:\n");
	printGraph(sptTree.g);

	printf("\nnRoots: %i\n", sptTree.nRoots);

	printf("local spanning trees:\n");
	for (int v=0; v<sptTree.nRoots; ++v) {
		printf("root %i (-> %i):\n", sptTree.roots[v]->number, sptTree.parents[v]->number);
		printGraph(sptTree.localSpanningTrees);
	}

	printf("Characteristics:\n");
	for (int v=0; v<sptTree.nRoots; ++v) {
		printf("root %i (-> %i):\n", sptTree.roots[v]->number, sptTree.parents[v]->number);
		for (struct SubtreeIsoDataStoreElement* e=sptTree.characteristics[v]->first; e!=NULL; e=e->next) {
			printNewCubeCondensed(e->data.S, e->data.g->n, e->data.h->n, stdout);
		}
	}
}

char noniterativeLocalEasySubtreeCheck(struct SpanningtreeTree sptTree, struct Graph* h, struct GraphPool* gp) {
	sptTree.characteristics = malloc(sptTree.nRoots * sizeof(struct SubtreeIsoDataStore));
	for (int v=sptTree.nRoots-1; v>=0; --v) {
		sptTree.characteristics[v] = getSubtreeIsoDataStoreList();
		for (struct Graph* localTree=sptTree.localSpanningTrees[v]; localTree!=NULL; localTree=localTree->next) {
			struct SubtreeIsoDataStore info = {0};
			info.g = localTree;
			info.h = h;
			info.postorder = getPostorder(localTree, 0); // 0 is the root v of localTree
			info.S = createNewCube(info.g->n, info.h->n);

			noniterativeLocalEasySubtreeCheck_intern(&info, sptTree, gp);
			appendSubtreeIsoDataStore(sptTree.characteristics[v], info);

			if (info.foundIso) {
				// TODO clean up
				return 1;
			}
		}
	}
	// TODO clean up
	return 0;
}
