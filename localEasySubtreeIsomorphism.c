#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "newCube.h"
#include "graph.h"
#include "listComponents.h"
#include "listSpanningTrees.h"
#include "bipartiteMatching.h"
#include "wilsonsAlgorithm.h"
#include "subtreeIsoUtils.h"
#include "bitSet.h"
#include "cachedGraph.h"
#include "localEasySubtreeIsomorphism.h"
#include "graphPrinting.h"
#include "cs_Tree.h"
#include "searchTree.h"
#include "sampleSubtrees.h"


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


/* we have to make sure that the children of each node come after the node in the resulting order
 * hence we first compare lowpoints (depth in the dfs from the root) of the vertices. If those are identical, then we return the
 * difference between the lowpoints of the parents (their height in the tree).
 *
 * A root v and some root w that belongs to a v-rooted component have the same ->lowPoint, but have different parents (w has v
 * as a parent, while v has a different parent (unless v is the root of the component)
 */
static int lowPointComparator(const void* a, const void* b) {
	struct Vertex* v = *(struct Vertex**)a;
	struct Vertex* w = *(struct Vertex**)b;

	int depthDiff = v->lowPoint - w->lowPoint;
	int parentDepthDiff = v->visited - w->visited;
	return (depthDiff != 0) ? depthDiff : parentDepthDiff;
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
	struct BlockTree blockTree = {0,0,0,0,0};
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
			// set ->visited of each root to the lowpoint of its parent root
			// this way, we can order the roots in one block correctly using lowPointComparator
			g->vertices[v]->visited = g->vertices[g->vertices[v]->d]->lowPoint;
			// if v is its own parent, mark this with a special visited value
			if (g->vertices[g->vertices[v]->d]->number == g->vertices[v]->number) {
				g->vertices[v]->visited = 0;
			}
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
		f->endPoint->d = e->endPoint->visited;
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
		rebaseShallowGraphs(tree, result);
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


// init characteristics array
void initCharacteristicsArrayForLocalEasy(struct SpanningtreeTree* sptTree) {
	sptTree->characteristics = malloc(sptTree->nRoots * sizeof(struct SubtreeIsoDataStoreList*));
	for (int v=0; v<sptTree->nRoots; ++v) {
		sptTree->characteristics[v] = getSubtreeIsoDataStoreList();
	}
}


/**
 * Defines a total order on undirected, labeled edges.
 */
int compareEdgesAbsolute(const struct VertexList* e, const struct VertexList* f) {
	int es, ee, fs, fe = 0;
	if (e->startPoint->number < e->endPoint->number) {
		es = e->startPoint->number;
		ee = e->endPoint->number;
	} else {
		es = e->endPoint->number;
		ee = e->startPoint->number;
	}

	if (f->startPoint->number < f->endPoint->number) {
		fs = f->startPoint->number;
		fe = f->endPoint->number;
	} else {
		fs = f->endPoint->number;
		fe = f->startPoint->number;
	}

	if (es != fs) {
		return es - fs;
	}
	if (ee != fe) {
		return ee - fe;
	}
	return labelCmp(e->label, f->label);
}


/**
 * Wrapper method for use in qsort
 */
int absCompEdges(const void* e1, const void* e2) {

	struct VertexList* g = *((struct VertexList**)e1);
	struct VertexList* h = *((struct VertexList**)e2);

	return compareEdgesAbsolute(g, h);
}

/**
Computes the lexicographic order of two rooted paths given by their roots \verb+ r1+ and \verb+ r2+ recursively.
This function returns
\[ -1 if P1 < P2 \]
\[ 0 if P1 = P2 \]
\[ 1 if P1 > P2 \]
and uses the comparison of label strings as total ordering.
The two paths are assumed to have edge labels and vertex labels are not taken into account.
* TODO Merge the generic code here with the code in cs_Compare.
 */
int compareVertexListsGeneric(const struct VertexList* e1, const struct VertexList* e2, int (*compar)(const struct VertexList*, const struct VertexList*)) {

	/* if this value is larger than 0 the first label is lex. larger than the second etc. */
	int returnValue = compar(e1, e2);

	/* if the two paths are identical so far wrt. labels check the next vertex on each path */
	if (returnValue == 0) {

		if ((e1->next) && (e2->next)) {
			/* if both lists have a next, proceed recursively */
			return compareVertexListsGeneric(e1->next, e2->next, compar);
		} else {
			/* if the first list is shorter, its lex. smaller */
			if ((e1->next == NULL) && (e2->next)){
				return -1;
			}
			/* if the second is shorter, this one is lex.smaller */
			if ((e2->next == NULL) && (e1->next)){
				return 1;
			}
		}

		/* if none of the cases above was chosen, both paths end here and we have to return 0 as both strings
		represented by the lists are identical */
		return 0;

	} else {
		/* if the paths differ in the current vertices, we return the return value of the comparison function */
		return returnValue;
	}
}

/**
 * Compare two shallowgraphs.
 * This method imposes a lexicographical order on shallowgraphs seen as strings of edges.
 * The order on edges is defined by the comparator.
 * TODO Merge the generic code here with the code in cs_Compare.
 */
int compareShallowGraphs(struct ShallowGraph* g, struct ShallowGraph* h, int (*compar)(const struct VertexList*, const struct VertexList*)) {
	return compareVertexListsGeneric(g->edges, h->edges, compar);
}

/**
 * Compare two shallowgraphs.
 */
int compareShallowGraphsAbsloute(struct ShallowGraph* g, struct ShallowGraph* h) {
	return compareVertexListsGeneric(g->edges, h->edges, &compareEdgesAbsolute);
}



int absCompShallowGraphs(const void* e1, const void* e2) {

	struct ShallowGraph* g = *((struct ShallowGraph**)e1);
	struct ShallowGraph* h = *((struct ShallowGraph**)e2);

	return compareShallowGraphs(g, h, &compareEdgesAbsolute);
}



static void sortShallowGraphEdges(struct ShallowGraph* l, int (*compar)(const void*,const void*)) {
	if (l->m > 1) {
		// create array containing edges from l
		struct VertexList** larray = malloc(l->m * sizeof(struct VertexList*));
		larray[0] = l->edges;
		for (int i=1; i<l->m; ++i) {
			larray[i] = larray[i-1]->next;
		}

		qsort(larray, l->m, sizeof(struct VertexList*), compar);

		// change order of edges in l according to qsort
		for (int i=0; i<l->m-1; ++i) {
			larray[i]->next = larray[i+1];
		}
		larray[l->m-1]->next = NULL;
		l->edges = larray[0];
		free(larray);
	}
}

static struct ShallowGraph* sortListOfShallowGraphs(struct ShallowGraph* l, size_t length, int (*compar)(const void*,const void*)) {
	if (length > 1) {
		// create array containing edges from l
		struct ShallowGraph** larray = malloc(length * sizeof(struct VertexList*));
		larray[0] = l;
		for (size_t i=1; i<length; ++i) {
			larray[i] = larray[i-1]->next;
		}

		qsort(larray, length, sizeof(struct VertexList*), compar);

		// change order of edges in l according to qsort
		for (size_t i=0; i<length-1; ++i) {
			larray[i]->next = larray[i+1];
		}
		larray[length-1]->next = NULL;
		l = larray[0];
		free(larray);
	}
	return l;
}


/**
 * Filter a sorted list of shallow graphs for duplicate elements defined by comparator.
 * l is consumed and should not be referenced later, as the first element might be dumped in the process.
 */
struct ShallowGraph* filterSortedSpanningTreeList(struct ShallowGraph* l, int (*compar)(struct ShallowGraph*, struct ShallowGraph*), struct ShallowGraphPool* sgp) {
	struct ShallowGraph* result = NULL;
	struct ShallowGraph* garbage = NULL;
	for (struct ShallowGraph* g=l; g!=NULL; /* increment is done below */) {
		if (g->next != NULL) {
			struct ShallowGraph* next = g->next;
			if (compar(g, g->next) == 0) {
				g->next = garbage;
				garbage = g;
			} else {
				g->next = result;
				result = g;
			}
			g = next;
		} else {
			// the last element should always be in the result.
			g->next = result;
			result = g;
			g = NULL;
		}
	}
	dumpShallowGraphCycle(sgp, garbage);
	return result;
}


struct ShallowGraph* filterDuplicateSpanningTrees(struct ShallowGraph* sptrees, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* filteredResult = NULL;

	// sort edges in spanning trees, count number of spts
	size_t length = 0;
	for (struct ShallowGraph* l=sptrees; l!=NULL; l=l->next) {
		sortShallowGraphEdges(l, &absCompEdges);
		++length;
	}

	filteredResult = sortListOfShallowGraphs(sptrees, length, &absCompShallowGraphs);
	filteredResult = filterSortedSpanningTreeList(filteredResult, &compareShallowGraphsAbsloute, sgp);

	return filteredResult;
}


static struct PostorderList* __getPOL() {
	struct PostorderList* result = malloc(sizeof(struct PostorderList));
	result->postorder = NULL;
	result->next = NULL;
	return result;
}

/**
 * blockTree is consumed
 * spanningTreesPerBlock must be >= 1
 */
struct SpanningtreeTree getSampledSpanningtreeTree(struct BlockTree blockTree, int spanningTreesPerBlock, char removeDuplicates, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct SpanningtreeTree sptTree = {0,0,0,0,0,0,0};
	sptTree.g = blockTree.g;
	sptTree.nRoots = blockTree.nRoots;
	sptTree.roots = blockTree.roots;
	sptTree.parents = blockTree.parents;
	sptTree.localSpanningTrees = malloc(sptTree.nRoots * sizeof(struct Graph*));
	sptTree.localPostorders = malloc(sptTree.nRoots * sizeof(struct PostorderList*));

	for (int v=0; v<sptTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		struct Graph* mergedGraph = blockConverter(mergedEdges, gp);

		struct ShallowGraph* shallowSpanningtrees = NULL;
		if (mergedGraph->m != mergedGraph->n-1) {
			// sample spanning trees according to parameter
			for (int i=0; i<spanningTreesPerBlock; ++i) {
				struct ShallowGraph* spt = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
				spt->next = shallowSpanningtrees;
				shallowSpanningtrees = spt;
			}

			/* Duplicate spanning trees are filtered here.
			 * In contrast to normal spanning tree sampling, here we can only filter identical trees (seen as edge sets)
			 * and not trees up to isomorphism, as two isomorphic but different local spanning trees might result in different
			 * (and hence possibly nonisomorphic) global spanning trees, when combined. */
			if (removeDuplicates) {
				shallowSpanningtrees = filterDuplicateSpanningTrees(shallowSpanningtrees, sgp);
			}
		} else {
			// if the mergedGraph is a tree, we use it directly
			shallowSpanningtrees = getGraphEdges(mergedGraph, sgp);
		}
		sptTree.localSpanningTrees[v] = spanningTreeConverter(shallowSpanningtrees, mergedGraph, gp, sgp);

		sptTree.localPostorders[v] = NULL;
		struct PostorderList* tail = NULL;
		for (struct Graph* localSpanningTree=sptTree.localSpanningTrees[v]; localSpanningTree!=NULL; localSpanningTree=localSpanningTree->next) {
			struct PostorderList* tmp = __getPOL();
			tmp->postorder = getPostorder(localSpanningTree, 0);
			if (sptTree.localPostorders[v] == NULL) {
				sptTree.localPostorders[v] = tail = tmp;
			} else {
				// append at the end of the list
				tail->next = tmp;
				tail = tmp;
			}
		}

		// garbage collection
		dumpShallowGraph(sgp, mergedEdges);
		dumpGraph(gp, mergedGraph);
	}

	initCharacteristicsArrayForLocalEasy(&sptTree);

	//garbage collection
	free(blockTree.vRootedBlocks);

	return sptTree;
}


/**
 * blockTree is comsumed
 * spanningTreesPerBlock must be >= 1
 */
struct SpanningtreeTree getFullSpanningtreeTree(struct BlockTree blockTree, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct SpanningtreeTree sptTree = {0,0,0,0,0,0,0};
	sptTree.g = blockTree.g;
	sptTree.nRoots = blockTree.nRoots;
	sptTree.roots = blockTree.roots;
	sptTree.parents = blockTree.parents;
	sptTree.localSpanningTrees = malloc(sptTree.nRoots * sizeof(struct Graph*));
	sptTree.localPostorders = malloc(sptTree.nRoots * sizeof(struct PostorderList*));

	for (int v=0; v<sptTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		// TODO make destructive. shallowGraphs are not used afterwards.
		struct Graph* mergedGraph = blockConverter(mergedEdges, gp);

		struct ShallowGraph* shallowSpanningtrees = listSpanningTrees(mergedGraph, sgp, gp);
		sptTree.localSpanningTrees[v] = spanningTreeConverter(shallowSpanningtrees, mergedGraph, gp, sgp);

		sptTree.localPostorders[v] = NULL;
		struct PostorderList* tail = NULL;
		for (struct Graph* localSpanningTree=sptTree.localSpanningTrees[v]; localSpanningTree!=NULL; localSpanningTree=localSpanningTree->next) {
			struct PostorderList* tmp = __getPOL();
			tmp->postorder = getPostorder(localSpanningTree, 0);
			if (sptTree.localPostorders[v] == NULL) {
				sptTree.localPostorders[v] = tail = tmp;
			} else {
				// append at the end of the list
				tail->next = tmp;
				tail = tmp;
			}
		}

		// garbage collection
		dumpShallowGraph(sgp, mergedEdges);
		dumpGraph(gp, mergedGraph);
	}

	initCharacteristicsArrayForLocalEasy(&sptTree);

	//garbage collection
	free(blockTree.vRootedBlocks);

	return sptTree;
}


void wipeCharacteristicsForLocalEasy(struct SpanningtreeTree sptTree) {
	if (sptTree.characteristics) {
		for (int v=0; v<sptTree.nRoots; ++v) {
			struct SubtreeIsoDataStoreList* list = sptTree.characteristics[v];
			if (list) {
				struct SubtreeIsoDataStoreElement* tmp;
				for (struct SubtreeIsoDataStoreElement* e=list->first; e!=NULL; e=tmp) {
					tmp = e->next;
					dumpNewCube(e->data.S, e->data.g->n);
					free(e);
				}
				list->size = 0;
				list->first = list->last = NULL;
				list->next = NULL;
			}
		}
	}
}


void dumpSpanningtreeTree(struct SpanningtreeTree sptTree, struct GraphPool* gp) {
	for (int v=0; v<sptTree.nRoots; ++v) {
		dumpGraphList(gp, sptTree.localSpanningTrees[v]);
		if (sptTree.characteristics) {
			if (sptTree.characteristics[v]) {
				struct SubtreeIsoDataStoreElement* tmp;
				for (struct SubtreeIsoDataStoreElement* e=sptTree.characteristics[v]->first; e!=NULL; e=tmp) {
					tmp = e->next;
					dumpNewCube(e->data.S, e->data.g->n);
					free(e);
				}
			}
			free(sptTree.characteristics[v]);
		}
		if (sptTree.localPostorders) {
			if (sptTree.localPostorders[v]) {
				struct PostorderList* tmp = NULL;
				for (struct PostorderList* l=sptTree.localPostorders[v]; l!=NULL; l=tmp) {
					tmp = l->next;
					free(l->postorder);
					free(l);
				}
			}
		}
	}
	free(sptTree.parents);
	free(sptTree.roots);
	free(sptTree.localSpanningTrees);
	free(sptTree.localPostorders);
	if (sptTree.characteristics) {
		free(sptTree.characteristics);
	}
}


static int countNontrivialRoots(struct SpanningtreeTree sptTree) {
	int count = 0;
	for (int i=0; i<sptTree.nRoots; ++i) {
		if (sptTree.localSpanningTrees[i]->next) {
			++count;
		}
	}
	return count;
}


void printSptTree(struct SpanningtreeTree sptTree) {
	printf("\nbase graph:\n");
	printGraph(sptTree.g);

	printf("\nnRoots: %i\n", sptTree.nRoots);
	printf("nNonTrivialRoots: %i\n", countNontrivialRoots(sptTree));

	printf("local spanning trees:\n");
	for (int v=0; v<sptTree.nRoots; ++v) {
		printf("root %i (-> %i): lps (%i %i) \n", sptTree.roots[v]->number, sptTree.parents[v]->number, sptTree.roots[v]->lowPoint, sptTree.parents[v]->lowPoint);
		printGraph(sptTree.localSpanningTrees[v]);
	}

	printf("Characteristics:\n");
	for (int v=0; v<sptTree.nRoots; ++v) {
		printf("root %i (-> %i):\n", sptTree.roots[v]->number, sptTree.parents[v]->number);
		if (sptTree.characteristics && sptTree.characteristics[v]) {
			for (struct SubtreeIsoDataStoreElement* e=sptTree.characteristics[v]->first; e!=NULL; e=e->next) {
				printNewCubeCondensed(e->data.S, e->data.g->n, e->data.h->n, stdout);
			}
		} else {
			printf("empty\n");
		}
	}
}


/* vertices of g have their ->visited values set to the postorder. Thus,
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
static struct Graph* makeBipartiteInstanceFromVerticesForLocalEasyCached(struct SubtreeIsoDataStore data, struct SubtreeIsoDataStore* wcharacteristics, struct CachedGraph* cachedB, struct Vertex* u, struct Vertex* w, struct Vertex* wBelow, struct GraphPool* gp) {

	int sizeofX = degree(u);
	int sizeofY = degree(w);
	int sizeofZ = 0;
	if (wBelow) {
		sizeofZ = degree(wBelow);
	}

	struct Graph* B = getCachedGraph(sizeofX + sizeofY + sizeofZ, cachedB);

	/* store size of first partitioning set */
	B->number = sizeofX;

	/* add vertex numbers of original vertices to ->lowPoint of each vertex in B
	and add edge labels to vertex labels to compare edges easily */
	int i = 0;
	for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
		B->vertices[i]->lowPoint = e->endPoint->number;
		B->vertices[i]->label = e->label;
		++i;
	}
	for (struct VertexList* e=w->neighborhood; e!=NULL; e=e->next) {
		/* y has to be a child of v */
		if (e->endPoint->visited < w->visited) {
			B->vertices[i]->lowPoint = e->endPoint->number;
			B->vertices[i]->label = e->label;
			++i;
		} else {
			--sizeofY;
		}
	}
	if (wBelow) {
		for (struct VertexList* e=wBelow->neighborhood; e!=NULL; e=e->next) {
			/* contrary to the above computation, y is definitively a child of wBelow */
			B->vertices[i]->lowPoint = e->endPoint->number;
			B->vertices[i]->label = e->label;
			++i;
		}
	}

	int sizeofXY = sizeofX + sizeofY;
	int sizeofXYZ = sizeofXY + sizeofZ;
	/* add edge (x,y) if u in S(y,x) */
	for (i=0; i<sizeofX; ++i) {
		int x = B->vertices[i]->lowPoint;
		for (int j=sizeofX; j<sizeofXY; ++j) {
			int y = B->vertices[j]->lowPoint;

			/* edge labels have to match, (v, child)->label in g == (u, child)->label in h
			these values were stored in B->vertices[i,j]->label */
			if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
				if (containsCharacteristic(data, u, data.h->vertices[x], data.g->vertices[y])) {
					addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
					++B->m;
				}
			}
		}
		for (int j=sizeofXY; j<sizeofXYZ; ++j) {
			int y = B->vertices[j]->lowPoint;

			/* edge labels have to match, (v, child)->label in g == (u, child)->label in h
			these values were stored in B->vertices[i,j]->label */
			if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
				if (containsCharacteristic(*wcharacteristics, u, wcharacteristics->h->vertices[x], wcharacteristics->g->vertices[y])) {
					addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
					++B->m;
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
	struct Graph* B = makeBipartiteInstanceFromVerticesForLocalEasyCached(*current, wCharacteristics, cachedB, u, w, wBelow, gp);
	int sizeofMatching = bipartiteMatchingEvenMoreDirty(B);
	int nNeighbors = B->number;

	// is there a subgraph iso here?
	if (sizeofMatching == nNeighbors) {
//		addCharacteristic(current, u, u, w); // in early termination we do not need to keep the books correct here
		current->foundIso = 1;

		returnCachedGraph(cachedB);
		return; // early termination when subtree iso is found
	}

	// compute partial subgraph isomorphisms
	if (sizeofMatching == nNeighbors - 1) {
		addNoncriticalVertexCharacteristics(current, B, u, w);
	}

	returnCachedGraph(cachedB);
}


/**
Labeled Subtree Isomorphism Check for a single local spanning tree of some v-rooted blocks.
 */
static void subtreeCheckForOneBlockSpanningTree(struct SubtreeIsoDataStore* current, struct SpanningtreeTree* sptTree, int blockDoesNotContainGlobalRoot, struct GraphPool* gp) {

	struct Graph* g = current->g;
	struct Graph* h = current->h;


	struct CachedGraph* cachedB = initCachedGraph(gp, h->n);

	current->foundIso = 0;
	// we can skip the computation if w = v, unless v is the global root of the graph
	// v is the last vertex in the postorder.
	for (int wi=0; wi<g->n-blockDoesNotContainGlobalRoot; ++wi) {
		struct Vertex* w = g->vertices[current->postorder[wi]];

		for (int ui=0; ui<h->n; ++ui) {
			struct Vertex* u = h->vertices[ui];

			// check if vertex labels match
			if (labelCmp(u->label, w->label) != 0) { continue; }

			// if w is not a root, life is easy, we do not need to process all \theta \in \Theta_{vw}
			// if w = v (i.e. if it is the global root) we just compute characteristics in the current spanning tree
			if ((w->d == -1) || (w->number == 0)) {
				computeCharacteristics(current, NULL, cachedB, u, w, NULL, gp);
				if (current->foundIso) {
					dumpCachedGraph(cachedB);
					return;
				}

			} else {
				// if w is a root unequal v
				// loop over the spanning trees of the w-rooted components
				for (struct SubtreeIsoDataStoreElement* e=sptTree->characteristics[w->d]->first; e!=NULL; e=e->next) {
//					if (e->next) {
//						fprintf(stderr, "processing multiple children at v=%i for w=%i\n", g->vertices[0]->d, w->d);
//					}
					struct Vertex* wBelow = e->data.g->vertices[0];
					computeCharacteristics(current, &(e->data), cachedB, u, w, wBelow, gp);
					if (current->foundIso) {
						dumpCachedGraph(cachedB);
						return;
					}
				}
			}
		}
	}
	dumpCachedGraph(cachedB);
}


/**
 * expects a cleanly initialized sptTree and can then tell you if a tree h is subgraph isomorphic to one of the
 * spanning trees represented by sptTree.
 *
 * This algorithm implements a generalized version of the embedding operator in
 *
 * Welke, Horvath, Wrobel: On the Complexity of Frequent Subtree Mining in Very Simple Structures. ILP 2014
 */
char subtreeCheckForSpanningtreeTree(struct SpanningtreeTree* sptTree, struct Graph* h, struct GraphPool* gp) {

	// for each root, process each spanning tree of the v rooted components and compute characteristics
	for (int v=sptTree->nRoots-1; v>=0; --v) {
		// we need to compute characteristics for the global root, which is a special case.
		int blockDoesNotContainGlobalRoot = v==0 ? 0 : 1;

		struct Graph* localTree = NULL;
		struct PostorderList* localPostorder = NULL;
		for (localTree=sptTree->localSpanningTrees[v], localPostorder=sptTree->localPostorders[v];
				localTree!=NULL;
				localTree=localTree->next, localPostorder=localPostorder->next) {

			struct SubtreeIsoDataStore info = {0,0,0,0,0};
			info.g = localTree;
			info.h = h;
//			info.postorder = getPostorder(localTree, 0); // 0 is the root v of localTree
			info.postorder = localPostorder->postorder;
			info.S = createNewCube(info.g->n, info.h->n);

			subtreeCheckForOneBlockSpanningTree(&info, sptTree, blockDoesNotContainGlobalRoot, gp);
			appendSubtreeIsoDataStore(sptTree->characteristics[v], info);

//			free(info.postorder);
//			info.postorder = NULL;

			if (info.foundIso) {
				return 1;
			}
		}
	}

	return 0;
}


/**
 * Check if a tree h is subgraph isomorphic to an arbitrary graph g using a sampling variant of the local easy subtree isomorphism algorithm.
 *
 * nLocalTrees specifies the number of local spanning trees that should be sampled for the set of v-rooted components of each root v.
 *
 * This method results in a subgraph isomorphism algorithm with one-sided error: If h is found to be subgraph isomorphic to g,
 * the answer is always correct. If h is not found to be subgraph isomorphic, it might in fact still be. A higher sampling parameter
 * results in a lower error probability and, of course, higher runtime.
 *
 */
char isProbabilisticLocalSampleSubtree(struct Graph* g, struct Graph* h, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	struct SpanningtreeTree sptTree = getSampledSpanningtreeTree(blockTree, nLocalTrees, 1, gp, sgp);

	char result = subtreeCheckForSpanningtreeTree(&sptTree, h, gp);

	dumpSpanningtreeTree(sptTree, gp);

	return result;
}


/**
 * Check if a tree h is subgraph isomorphic to an arbitrary graph g using the local easy subtree isomorphsm algorithm.
 *
 * This algorithm is polynomial, if g is local easy. In particular, its runtime and space complexity depends on the number
 * of local spanning trees. See our paper for details.
 * TODO add reference.
 */
char isLocalEasySubtree(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	struct SpanningtreeTree sptTree = getFullSpanningtreeTree(blockTree, gp, sgp);

	char result = subtreeCheckForSpanningtreeTree(&sptTree, h, gp);

	dumpSpanningtreeTree(sptTree, gp);

	return result;
}


/**
 * Seriously, this is ugly. (hence very static and hidden from view)
 *
 * Create a list of shallow graphs that are the merge of the combinations of elements of the
 * shallowgraph lists in lists.
 * We explicitly construct these merged shallow graphs all at once, hence the space and the runtime
 * required to run this method are both exponential. In particular: $\prod_{i=1}^{nLists}\abs{lists[i]}$
 * elements are created. Each has the size of the union of its elements. Nice. Not.
 */
struct ShallowGraph* spanningTreeCombinations(struct ShallowGraph** lists, int currentPos, int nLists, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* resultList = NULL;

	if (currentPos == nLists-1) {
		for (struct ShallowGraph* g=lists[currentPos]; g!=NULL; g=g->next) {
			struct ShallowGraph* tmp = cloneShallowGraph(g, sgp);
			tmp->next = resultList;
			resultList = tmp;
		}
	} else {
		struct ShallowGraph* conquered = spanningTreeCombinations(lists, currentPos + 1, nLists, sgp);
		for (struct ShallowGraph* g=lists[currentPos]; g!=NULL; g=g->next) {
			for (struct ShallowGraph* h=conquered; h!=NULL; h=h->next) {
				struct ShallowGraph* tmp1 = cloneShallowGraph(g, sgp);
				struct ShallowGraph* tmp2 = cloneShallowGraph(h, sgp);
				mergeTwoShallowGraphs(tmp1, tmp2, sgp);
				tmp1->next = resultList;
				resultList = tmp1;
			}
		}
		// the result of the recursive invocation is now useless
		dumpShallowGraphCycle(sgp, conquered);
	}
	return resultList;
}


/*
 * transform shallowgraph of v rooted components to graph rooted at v.
 * v will be vertex 0 in the resulting graph
 * w->d in the resulting graph gives the root array index of w, if w is a root, or -1 otherwise.
 * g->vertices[0] is the root (i.e. v in the notation of the paper)
 */
static struct Graph* blockConverterKeepIDs(struct ShallowGraph* edgeList, struct GraphPool* gp) {

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
		f->endPoint->d = e->endPoint->visited;

		// keep the original vertex number in ->lowPoint
		f->startPoint->lowPoint = e->startPoint->number;
		f->endPoint->lowPoint = e->endPoint->number;
	}

	return g;
}


static struct ShallowGraph* xcanonicalStringOfTree(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)threshold;
	(void)k;
	(void)gp;
	return canonicalStringOfTree(g, sgp);
}


/**
 *
 */
int getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySampling(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	int numberOfNonisomorphicSpanningForestComponents = 0;

	struct ShallowGraph** localSpanningTrees = malloc(blockTree.nRoots * sizeof(struct Graph*));

	for (int v=0; v<blockTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		struct Graph* mergedGraph = blockConverterKeepIDs(mergedEdges, gp);

		struct ShallowGraph* shallowSpanningtrees = NULL;
		if (mergedGraph->m != mergedGraph->n-1) {
			// sample spanning trees according to parameter
			for (int i=0; i<k; ++i) {
				struct ShallowGraph* spt = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
				spt->next = shallowSpanningtrees;
				shallowSpanningtrees = spt;
			}

			/* Duplicate spanning trees are NOT filtered here.
			 * In contrast to normal spanning tree sampling, here we can only filter identical trees (seen as edge sets)
			 * and not trees up to isomorphism, as two isomorphic but different local spanning trees might result in different
			 * (and hence possibly nonisomorphic) global spanning trees, when combined. */
//			shallowSpanningtrees = filterDuplicateSpanningTrees(shallowSpanningtrees, sgp);

		} else {
			// if the mergedGraph is a tree, we use it directly
			shallowSpanningtrees = getGraphEdges(mergedGraph, sgp);
		}

		// make the shallowgraphs be subgraphs of g, not of mergedGraph
		rebaseShallowGraphsOnLowPoints(shallowSpanningtrees, g);

		localSpanningTrees[v] = shallowSpanningtrees;

		// garbage collection
		dumpShallowGraph(sgp, mergedEdges);
		dumpGraph(gp, mergedGraph);

	}

	struct Vertex* searchTree = getVertex(gp->vertexPool);

	struct ShallowGraph* combinations = spanningTreeCombinations(localSpanningTrees, 0, blockTree.nRoots, sgp);
	for (struct ShallowGraph* combination=popShallowGraph(&combinations); combination!=NULL; combination=popShallowGraph(&combinations)) {
		struct Graph* spForest = shallowGraphToGraph(combination, gp);
		addToSearchTree(searchTree, runForEachConnectedComponent(&xcanonicalStringOfTree, spForest, 0, 0, 0, gp, sgp), gp, sgp);
		dumpGraph(gp, spForest);
		dumpShallowGraph(sgp, combination);
	}

	numberOfNonisomorphicSpanningForestComponents = searchTree->d;

	//garbage collection
	dumpSearchTree(gp, searchTree);
	for (int v=0; v<blockTree.nRoots; ++v) {
		dumpShallowGraphCycle(sgp, localSpanningTrees[v]);
	}
	free(localSpanningTrees);
	free(blockTree.parents);
	free(blockTree.roots);
	free(blockTree.vRootedBlocks);

	return numberOfNonisomorphicSpanningForestComponents;
}


/**
 *
 */
int getNumberOfSpanningTreesObtainedByLocalEasySampling(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	int numberOfConsideredSpanningTrees = 1;

	struct ShallowGraph** localSpanningTrees = malloc(blockTree.nRoots * sizeof(struct Graph*));

	for (int v=0; v<blockTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		struct Graph* mergedGraph = blockConverterKeepIDs(mergedEdges, gp);

		struct ShallowGraph* shallowSpanningtrees = NULL;
		if (mergedGraph->m != mergedGraph->n-1) {
			// sample spanning trees according to parameter
			for (int i=0; i<k; ++i) {
				struct ShallowGraph* spt = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
				spt->next = shallowSpanningtrees;
				shallowSpanningtrees = spt;
			}

			/* Duplicate spanning trees are NOT filtered here.
			 * In contrast to normal spanning tree sampling, here we can only filter identical trees (seen as edge sets)
			 * and not trees up to isomorphism, as two isomorphic but different local spanning trees might result in different
			 * (and hence possibly nonisomorphic) global spanning trees, when combined. */
//			shallowSpanningtrees = filterDuplicateSpanningTrees(shallowSpanningtrees, sgp);

		} else {
			// if the mergedGraph is a tree, we use it directly
			shallowSpanningtrees = getGraphEdges(mergedGraph, sgp);
		}

		// make the shallowgraphs be subgraphs of g, not of mergedGraph
		rebaseShallowGraphsOnLowPoints(shallowSpanningtrees, g);

		localSpanningTrees[v] = shallowSpanningtrees;

		// garbage collection
		dumpShallowGraph(sgp, mergedEdges);
		dumpGraph(gp, mergedGraph);

	}

	for (int i=0; i<blockTree.nRoots; ++i) {
		int j=0;
		for (struct ShallowGraph* localTree=localSpanningTrees[i]; localTree!=NULL; localTree=localTree->next) {
			++j;
		}

		int tmp = numberOfConsideredSpanningTrees * j;
		if (j != 0 && tmp / j != numberOfConsideredSpanningTrees) {
			numberOfConsideredSpanningTrees = RAND_MAX;
			break;
		} else {
			numberOfConsideredSpanningTrees = tmp;
		}
	}

	//garbage collection
	for (int v=0; v<blockTree.nRoots; ++v) {
		dumpShallowGraphCycle(sgp, localSpanningTrees[v]);
	}
	free(localSpanningTrees);
	free(blockTree.parents);
	free(blockTree.roots);
	free(blockTree.vRootedBlocks);

	return numberOfConsideredSpanningTrees;
}


int getNumberOfSpanningTreesObtainedByLocalEasySamplingWithFiltering(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	int numberOfConsideredSpanningTrees = 1;

	struct ShallowGraph** localSpanningTrees = malloc(blockTree.nRoots * sizeof(struct Graph*));

	for (int v=0; v<blockTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		struct Graph* mergedGraph = blockConverterKeepIDs(mergedEdges, gp);

		struct ShallowGraph* shallowSpanningtrees = NULL;
		if (mergedGraph->m != mergedGraph->n-1) {
			// sample spanning trees according to parameter
			for (int i=0; i<k; ++i) {
				struct ShallowGraph* spt = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
				spt->next = shallowSpanningtrees;
				shallowSpanningtrees = spt;
			}

			/* Duplicate spanning trees are filtered here.
			 * In contrast to normal spanning tree sampling, here we can only filter identical trees (seen as edge sets)
			 * and not trees up to isomorphism, as two isomorphic but different local spanning trees might result in different
			 * (and hence possibly nonisomorphic) global spanning trees, when combined. */
			shallowSpanningtrees = filterDuplicateSpanningTrees(shallowSpanningtrees, sgp);

		} else {
			// if the mergedGraph is a tree, we use it directly
			shallowSpanningtrees = getGraphEdges(mergedGraph, sgp);
		}

		// make the shallowgraphs be subgraphs of g, not of mergedGraph
		rebaseShallowGraphsOnLowPoints(shallowSpanningtrees, g);

		localSpanningTrees[v] = shallowSpanningtrees;

		// garbage collection
		dumpShallowGraph(sgp, mergedEdges);
		dumpGraph(gp, mergedGraph);

	}

	for (int i=0; i<blockTree.nRoots; ++i) {
		int j=0;
		for (struct ShallowGraph* localTree=localSpanningTrees[i]; localTree!=NULL; localTree=localTree->next) {
			++j;
		}

		int tmp = numberOfConsideredSpanningTrees * j;
		if (j != 0 && tmp / j != numberOfConsideredSpanningTrees) {
			numberOfConsideredSpanningTrees = RAND_MAX;
			break;
		} else {
			numberOfConsideredSpanningTrees = tmp;
		}
	}

	//garbage collection
	for (int v=0; v<blockTree.nRoots; ++v) {
		dumpShallowGraphCycle(sgp, localSpanningTrees[v]);
	}
	free(localSpanningTrees);
	free(blockTree.parents);
	free(blockTree.roots);
	free(blockTree.vRootedBlocks);

	return numberOfConsideredSpanningTrees;
}


/**
 *
 */
int getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySamplingWithFiltering(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	int numberOfNonisomorphicSpanningForestComponents = 0;

	struct ShallowGraph** localSpanningTrees = malloc(blockTree.nRoots * sizeof(struct Graph*));

	for (int v=0; v<blockTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		struct Graph* mergedGraph = blockConverterKeepIDs(mergedEdges, gp);

		struct ShallowGraph* shallowSpanningtrees = NULL;
		if (mergedGraph->m != mergedGraph->n-1) {
			// sample spanning trees according to parameter
			for (int i=0; i<k; ++i) {
				struct ShallowGraph* spt = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
				spt->next = shallowSpanningtrees;
				shallowSpanningtrees = spt;
			}

			/* Duplicate spanning trees are NOT filtered here.
			 * In contrast to normal spanning tree sampling, here we can only filter identical trees (seen as edge sets)
			 * and not trees up to isomorphism, as two isomorphic but different local spanning trees might result in different
			 * (and hence possibly nonisomorphic) global spanning trees, when combined. */
			shallowSpanningtrees = filterDuplicateSpanningTrees(shallowSpanningtrees, sgp);

		} else {
			// if the mergedGraph is a tree, we use it directly
			shallowSpanningtrees = getGraphEdges(mergedGraph, sgp);
		}

		// make the shallowgraphs be subgraphs of g, not of mergedGraph
		rebaseShallowGraphsOnLowPoints(shallowSpanningtrees, g);

		localSpanningTrees[v] = shallowSpanningtrees;

		// garbage collection
		dumpShallowGraph(sgp, mergedEdges);
		dumpGraph(gp, mergedGraph);

	}

	struct Vertex* searchTree = getVertex(gp->vertexPool);

	struct ShallowGraph* combinations = spanningTreeCombinations(localSpanningTrees, 0, blockTree.nRoots, sgp);
	for (struct ShallowGraph* combination=popShallowGraph(&combinations); combination!=NULL; combination=popShallowGraph(&combinations)) {
		struct Graph* spForest = shallowGraphToGraph(combination, gp);
		addToSearchTree(searchTree, runForEachConnectedComponent(&xcanonicalStringOfTree, spForest, 0, 0, 0, gp, sgp), gp, sgp);
		dumpGraph(gp, spForest);
		dumpShallowGraph(sgp, combination);
	}

	numberOfNonisomorphicSpanningForestComponents = searchTree->d;

	//garbage collection
	dumpSearchTree(gp, searchTree);
	for (int v=0; v<blockTree.nRoots; ++v) {
		dumpShallowGraphCycle(sgp, localSpanningTrees[v]);
	}
	free(localSpanningTrees);
	free(blockTree.parents);
	free(blockTree.roots);
	free(blockTree.vRootedBlocks);

	return numberOfNonisomorphicSpanningForestComponents;
}


