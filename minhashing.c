// this source file uses qsort_r which is not part of C99, but a GNU specific extension.
#define _GNU_SOURCE

#include <stdlib.h>
#include <malloc.h>

#include "graph.h"
#include "searchTree.h"
#include "cs_Tree.h"
#include "cs_Parsing.h"
#include "iterativeSubtreeIsomorphism.h"
#include "listComponents.h"
#include "intSet.h"
#include "importantSubtrees.h"
#include "minhashing.h"
#include "localEasySubtreeIsomorphism.h"
#include "subtreeIsoUtils.h"

// PERMUTATIONS

/**
 * Shuffle array.
 *
 * Implementing Fisher–Yates shuffle
 * from http://stackoverflow.com/questions/1519736/random-shuffling-of-an-array
 */
static void shuffleArray(int* array, int arraySize) {
	for (int i = arraySize - 1; i > 0; i--)
	{
		int index = rand() % (i + 1);
		// Simple swap
		int a = array[index];
		array[index] = array[i];
		array[i] = a;
	}
}

/** Return a random permutation of the numbers from 1 to n */
int* getRandomPermutation(int n) {
	int* permutation = malloc(n * sizeof(int));
	if (permutation) {
		for (int i=0; i<n; ++i) {
			permutation[i] = i+1;
		}
		shuffleArray(permutation, n);
	} else {
		fprintf(stderr, "Error allocating memory for permutation\n");
	}
	return permutation;
}

// POSET PERMUTATION SHRINK

/** given a permutation of the numbers 0,...,n-1 and a directed acyclic graph F,
 *  remove those numbers from the permutation that can never be a min-hash for the
 *  given permutation and poset.
 *
 *  The method returns the number of possible min hash positions and sets the values
 *  of the invalid positions to -1.
 */
int posetPermutationMark(int* permutation, size_t n, struct Graph* F) {
	for (int v=0; v<F->n; ++v) {
		F->vertices[v]->visited = 0;
	}
	size_t shrunkSize = 0;
	for (size_t i=0; i<n; ++i) {
		struct Vertex* v = F->vertices[permutation[i]];
		if (v->visited == 0) {
			markConnectedComponent(v, 1);
			++shrunkSize;
		} else {
			permutation[i] = -1;
		}
	}
	return shrunkSize;
}

/**
 * given a permutation of 0,...,n-1 that was processed with posetPermutationMark and the output
 * value shrunkSize of that function, return an array of shrunkSize that only contains the
 * valid positions.
 */
int* posetPermutationShrink(int* permutation, size_t n, size_t shrunkSize) {
	int* condensedSequence = malloc(shrunkSize * sizeof(int));
	size_t posInCondensed = 0;
	for (size_t i=0; i<n; ++i) {
		if (permutation[i] != -1) {
			condensedSequence[posInCondensed] = permutation[i];
			++posInCondensed;
		}
	}
	free(permutation);
	return condensedSequence;
}

int posPairSorter(const void* a, const void* b, void* context) {
	struct PosPair one = *(struct PosPair*)a;
	struct PosPair two = *(struct PosPair*)b;
	struct EvaluationPlan p = *(struct EvaluationPlan*)context;

	int oneID = p.shrunkPermutations[one.permutation][one.level];
	int twoID = p.shrunkPermutations[two.permutation][two.level];

	return oneID - twoID;
}


struct Graph* reverseGraph(struct Graph* g, struct GraphPool* gp) {
	struct Graph* reverse = emptyGraph(g, gp);
	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			struct VertexList* f = shallowCopyEdge(e, gp->listPool);
			f->startPoint = reverse->vertices[e->endPoint->number];
			f->endPoint = reverse->vertices[e->startPoint->number];
			addEdge(f->startPoint, f);
		}
	}
	reverse->m = g->m;
	return reverse;
}


struct EvaluationPlan buildEvaluationPlan(int** shrunkPermutations, size_t* permutationSizes, size_t K, struct Graph* F, struct GraphPool* gp) {
	struct EvaluationPlan p = {0};
	p.F = F;
	p.reverseF = reverseGraph(F, gp);
	p.sketchSize = K;
	p.shrunkPermutations = shrunkPermutations;

	size_t maxPermutationSize = 0;
	for (size_t i=0; i<K; ++i) {
		p.orderLength += permutationSizes[i];
		if (permutationSizes[i] > maxPermutationSize) {
			maxPermutationSize = permutationSizes[i];
		}
	}
	p.order = malloc(p.orderLength * sizeof(struct PosPair));
	if (p.order) {
		// fix some evaluation order.
		size_t position = 0;
		for (size_t level=0; level<maxPermutationSize; ++level) {
			size_t old_pos = position;
			for (size_t i=0; i<K; ++i) {
				if (level < permutationSizes[i]) {
					p.order[position].level = level;
					p.order[position].permutation = i;
					++position;
				}
			}
			// sort current level of the permutations based on the pattern id. This ensures that lower level
			// patterns come before higher level patterns in each level of the order.
			qsort_r(&(p.order[old_pos]), position - old_pos, sizeof(struct PosPair), &posPairSorter, &p);
//			for (size_t i=old_pos; i<position; ++i) {
//				printf("(%zu %zu: %i) ", p.order[i].level, p.order[i].permutation, p.shrunkPermutations[p.order[i].permutation][p.order[i].level]);
//			}
//			printf("\n");
		}

	} else {
		fprintf(stderr, "could not allocate space for evaluation plan. this program will break now.\n");
		p.F = NULL;
		p.order = NULL;
		p.shrunkPermutations = NULL;
		p.orderLength = 0;
		p.sketchSize = 0;

	}

	free(permutationSizes); // we do not need this information any more
	return p;
}

struct EvaluationPlan dumpEvaluationPlan(struct EvaluationPlan p, struct GraphPool* gp) {
	free(p.order);
	for (size_t i=0; i<p.sketchSize; ++i) {
		free(p.shrunkPermutations[i]);
	}
	free(p.shrunkPermutations);
	for (int v=1; v<p.F->n; ++v) { // start from 1, as empty pattern has no graph attached
		dumpGraph(gp, (struct Graph*)(p.F->vertices[v]->label));
		p.F->vertices[v]->label = NULL;
	}
	dumpGraph(gp, p.reverseF);
	dumpGraph(gp, p.F);
	struct EvaluationPlan empty = {0};
	return empty;
}

// BUILD TREE POSET

static int addEdgesFromSubtrees(struct Graph* pattern, struct Vertex* searchtree, struct Graph* F, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	// singletons have the empty graph as ancestor
	if (pattern->n == 1) {
		struct VertexList* e = getVertexList(gp->listPool);
		e->startPoint = F->vertices[0];
		e->endPoint = F->vertices[pattern->number];
		addEdge(e->startPoint, e);
		F->m += 1;
		return 1;
	}

	int edgeCount = 0;
	// create graph that will hold subgraphs of size n-1 (this assumes that all extension trees have the same size)
	struct Graph* subgraph = getGraph(gp);
	setVertexNumber(subgraph, pattern->n - 1);
	subgraph->m = subgraph->n - 1;

	for (int v=0; v<pattern->n; ++v) {
		// if the removed vertex is a leaf, we test if the resulting subtree is contained in the lower level
		if (isLeaf(pattern->vertices[v]) == 1) {
			// we invalidate current by removing the edge to v from its neighbor, which makes subgraph a valid tree
			struct VertexList* edge = snatchEdge(pattern->vertices[v]->neighborhood->endPoint, pattern->vertices[v]);
			// now copy pointers of all vertices \neq v to subgraph, this results in a tree of size current->n - 1
			int j = 0;
			for (int i=0; i<pattern->n; ++i) {
				if (i == v) {
					continue; // ...with next vertex vertices
				} else {
					subgraph->vertices[j] = pattern->vertices[i];
					subgraph->vertices[j]->number = j;
					++j;
				}
			}

			// compute canonical string of subtree and get its position in F
			struct ShallowGraph* subString = canonicalStringOfTree(subgraph, sgp);
			int subtreeID = getID(searchtree, subString);
			dumpShallowGraph(sgp, subString);

			// restore law and order in current (and invalidate subgraph)
			addEdge(edge->startPoint, edge);

			if (subtreeID != -1) {
				struct VertexList* e = getVertexList(gp->listPool);
				e->startPoint = F->vertices[subtreeID];
				e->endPoint = F->vertices[pattern->number];
				addEdge(e->startPoint, e);
				F->m += 1;
				++edgeCount;
			} else {
				fprintf(stderr, "subtree not found in searchtree. this should not happen\n");
			}
		}
	}

	// clean up
	for (int v=0; v<pattern->n; ++v) {
		pattern->vertices[v]->number = v;
	}
	// garbage collection
	for (int v=0; v<subgraph->n; ++v) {
		subgraph->vertices[v] = NULL;
	}
	dumpGraph(gp, subgraph);

	return edgeCount;
}


/**
 * Build a poset of trees given as graphs, ordered by subgraph isomorphism.
 *
 * Input: a list of graphs that are trees, ordered by number of vertices
 * Output: a graph F where each vertex corresponds to a canonical string in
 *         the input, except vertex 0 that corresponds to the empty pattern.
 *
 *
 * - The vertex number of a canonical string is given by the position of the
 *   canonical string in the input list (starting with 1).
 * - The vertex->label points to a graph representation of the canonical string in the input list (be
 *   careful to cast correctly to struct Graph* when using)
 * - There is an edge (v,w) \in E(F) <=> |V(v)| = |V(w)| - 1 AND v is subgraph
 *   isomorphic to w
 */
struct Graph* buildTreePosetFromGraphDB(struct Graph** db, int nGraphs, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	// add strings to a search tree with correct numbers
	struct Vertex* searchTree = getVertex(gp->vertexPool);
	for (int i=0; i<nGraphs; ++i) {
		db[i]->number = i + 1; // we assign new ids that match the position in our poset graph
		struct ShallowGraph* string = canonicalStringOfTree(db[i], sgp);
		addToSearchTree(searchTree, string, gp, sgp);
	}

	// create graph with vertices corresponding to strings.
	struct Graph* F = createGraph(nGraphs + 1, gp);
	for (int i=0; i<nGraphs; ++i) {
		struct Graph* g = db[i];
		F->vertices[i+1]->label = (char*)g; // misuse of char* pointer
		addEdgesFromSubtrees(g, searchTree, F, gp, sgp);
	}

	dumpSearchTree(gp, searchTree);
	return F;
}


// COMPUTATION OF MINHASHES

static void cleanEvaluationPlan(struct EvaluationPlan p) {
	for (int i=0; i<p.F->n; ++i) {
		p.F->vertices[i]->visited = 0;
		p.reverseF->vertices[i]->visited = 0;
	}
}

int* fastMinHashForAndOr(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp) {
	int* sketch = malloc(p.sketchSize * sizeof(int));
	if (!sketch) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}
	for (size_t i=0; i<p.sketchSize; ++i) {
		sketch[i] = -1; // init sketch values to 'infty'
	}
	for (size_t i=0; i<p.orderLength; ++i) {
		struct PosPair current = p.order[i];
		int currentGraphNumber = p.shrunkPermutations[current.permutation][current.level];

		// check if we already found level min for the permutation
		if (sketch[current.permutation] != -1) {
			continue;
		}

		// check if we already evaluated the embedding operator for this pattern or found a subpattern that had no match
		if (p.F->vertices[currentGraphNumber]->visited != 0) {
			// either the pattern with id currentGraphNumber was evaluated positively and we hence have found the
			// min value for the current permutation or we need to continue
			if (p.F->vertices[currentGraphNumber]->visited == 1) {
				sketch[current.permutation] = current.level;
			}
			continue;
		}

		// evaluate the embedding operator
		struct Graph* currentGraph = (struct Graph*)(p.F->vertices[currentGraphNumber]->label);
		char match = andorEmbedding(g, currentGraph, gp);
		if (match) {
			p.F->vertices[currentGraphNumber]->visited = 1;
			sketch[current.permutation] = current.level;
			continue;
		} else {
			markConnectedComponent(p.F->vertices[currentGraphNumber], -1);
		}

	}

	cleanEvaluationPlan(p);
	return sketch;
}


/**
Traverses the reverse graph of the poset graph F and marks all vertices reachable from v with the number given
by the argument component.

Hence, v needs to be a vertex in the reverse graph p.reverseF !

Careful: To avoid infinite runtime, the method tests if a visited vertex has ->visited == component.
Thus, either initialize ->visited's  with some value < 0 or start component counting with 1.
 */
static void rayOfLight(struct Vertex* v, int component, struct EvaluationPlan p) {

	/* mark vertex as visited */
	v->visited = component;
	p.F->vertices[v->number]->visited = component;


	/*recursive call for all neighbors that are not visited so far */
	for (struct VertexList* index = v->neighborhood; index; index = index->next) {
		if (index->endPoint->visited != component) {
			rayOfLight(index->endPoint, component, p);
		}
	}
}

/**
 *
 */
int* fastMinHashForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp) {
	int* sketch = malloc(p.sketchSize * sizeof(int));
	if (!sketch) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}
	int nEvaluations = 0;

	cleanEvaluationPlan(p);

	for (size_t i=0; i<p.sketchSize; ++i) {
		sketch[i] = -1; // init sketch values to 'infty'
	}
	for (size_t i=0; i<p.orderLength; ++i) {
		struct PosPair current = p.order[i];
		int currentGraphNumber = p.shrunkPermutations[current.permutation][current.level];

		// check if we already found level min for the permutation
		if (sketch[current.permutation] != -1) {
			continue;
		}

		// check if we already evaluated the embedding operator for this pattern or found a subpattern that had no match
		if (p.F->vertices[currentGraphNumber]->visited != 0) {
			// either the pattern with id currentGraphNumber was evaluated positively and we hence have found the
			// min value for the current permutation or we need to continue
			if (p.F->vertices[currentGraphNumber]->visited == 1) {
				sketch[current.permutation] = current.level;
			}
			continue;
		}

		// evaluate the embedding operator
		struct Graph* currentGraph = (struct Graph*)(p.F->vertices[currentGraphNumber]->label);
		char match = isSubtree(g, currentGraph, gp);
		++nEvaluations;
		if (match) {
//			p.F->vertices[currentGraphNumber]->visited = 1;
			rayOfLight(p.reverseF->vertices[currentGraphNumber], 1, p);
			// TODO mark parent patterns visited !
			sketch[current.permutation] = current.level;
//			continue;
		} else {
			markConnectedComponent(p.F->vertices[currentGraphNumber], -1);
		}
	}
	fprintf(stderr, "%i\n", nEvaluations);
	return sketch;
}


/**
 *
 */
int* fastMinHashForAbsImportantTrees(struct Graph* g, struct EvaluationPlan p, int importance, struct GraphPool* gp) {
	int* sketch = malloc(p.sketchSize * sizeof(int));
	if (!sketch) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}
	for (size_t i=0; i<p.sketchSize; ++i) {
		sketch[i] = -1; // init sketch values to 'infty'
	}
	for (size_t i=0; i<p.orderLength; ++i) {
		struct PosPair current = p.order[i];
		int currentGraphNumber = p.shrunkPermutations[current.permutation][current.level];

		// check if we already found level min for the permutation
		if (sketch[current.permutation] != -1) {
			continue;
		}

		// check if we already evaluated the embedding operator for this pattern or found a subpattern that had no match
		if (p.F->vertices[currentGraphNumber]->visited != 0) {
			// either the pattern with id currentGraphNumber was evaluated positively and we hence have found the
			// min value for the current permutation or we need to continue
			if (p.F->vertices[currentGraphNumber]->visited == 1) {
				sketch[current.permutation] = current.level;
			}
			continue;
		}

		// evaluate the embedding operator
		struct Graph* currentGraph = (struct Graph*)(p.F->vertices[currentGraphNumber]->label);
		char match = isImportantSubtreeAbsolute(g, currentGraph, importance, gp);
		if (match) {
			p.F->vertices[currentGraphNumber]->visited = 1;
			sketch[current.permutation] = current.level;
			continue;
		} else {
			markConnectedComponent(p.F->vertices[currentGraphNumber], -1);
		}

	}

	cleanEvaluationPlan(p);
	return sketch;
}

/**
 *
 */
int* fastMinHashForRelImportantTrees(struct Graph* g, struct EvaluationPlan p, double importance, struct GraphPool* gp) {
	int* sketch = malloc(p.sketchSize * sizeof(int));
	if (!sketch) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}
	for (size_t i=0; i<p.sketchSize; ++i) {
		sketch[i] = -1; // init sketch values to 'infty'
	}
	for (size_t i=0; i<p.orderLength; ++i) {
		struct PosPair current = p.order[i];
		int currentGraphNumber = p.shrunkPermutations[current.permutation][current.level];

		// check if we already found level min for the permutation
		if (sketch[current.permutation] != -1) {
			continue;
		}

		// check if we already evaluated the embedding operator for this pattern or found a subpattern that had no match
		if (p.F->vertices[currentGraphNumber]->visited != 0) {
			// either the pattern with id currentGraphNumber was evaluated positively and we hence have found the
			// min value for the current permutation or we need to continue
			if (p.F->vertices[currentGraphNumber]->visited == 1) {
				sketch[current.permutation] = current.level;
			}
			continue;
		}

		// evaluate the embedding operator
		struct Graph* currentGraph = (struct Graph*)(p.F->vertices[currentGraphNumber]->label);
		char match = isImportantSubtreeRelative(g, currentGraph, importance, gp);
		if (match) {
			p.F->vertices[currentGraphNumber]->visited = 1;
			sketch[current.permutation] = current.level;
			continue;
		} else {
			markConnectedComponent(p.F->vertices[currentGraphNumber], -1);
		}

	}

	cleanEvaluationPlan(p);
	return sketch;
}

static void addToBorder(struct Vertex* v, struct ShallowGraph* border, struct ShallowGraphPool* sgp) {
	struct VertexList* e = getVertexList(sgp->listPool);
	e->endPoint = v;
	appendEdge(border, e);
}

static struct Vertex* popFromBorder(struct ShallowGraph* border, struct ShallowGraphPool* sgp) {
	struct VertexList* e = popEdge(border);
	struct Vertex* v = e->endPoint;
	dumpVertexList(sgp->listPool, e);
	return v;
}

// FOR COMPARISON: EXPLICIT EVALUATION USING THE PATTERN POSET
struct IntSet* explicitEmbeddingForTrees(struct Graph* g, struct Graph* F, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	int nEvaluations = 0;

	//cleanup
	for (int v=0; v<F->n; ++v) { F->vertices[v]->visited = 0; }

	// init output
	struct IntSet* features = getIntSet();

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=F->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToBorder(e->endPoint, border, sgp);
	}

	// bfs evaluation with pruning through the poset
	while (border->m != 0) {
		struct Vertex* v = popFromBorder(border, sgp);
		struct Graph* pattern = (struct Graph*)(v->label);
		if (v->visited == 0) {
			char match = isSubtree(g, pattern, gp);
			++nEvaluations;
			if (match) {
				addIntSortedNoDuplicates(features, v->number - 1);
				v->visited = 1;
				for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
					if (e->endPoint->visited == 0) {
						addToBorder(e->endPoint, border, sgp);
					}
				}
			} else {
				markConnectedComponent(v, -1);
			}
		}
	}
	fprintf(stderr, "%i\n", nEvaluations);
	return features;

}


struct IntSet* explicitEmbeddingForLocalEasyOperator(struct Graph* g, struct Graph* F, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	int nEvaluations = 0;

	//cleanup
	for (int v=0; v<F->n; ++v) { F->vertices[v]->visited = 0; }

	// init output
	struct IntSet* features = getIntSet();

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=F->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToBorder(e->endPoint, border, sgp);
	}

	// init data structure for embedding operator
	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	struct SpanningtreeTree sptTree = getSampledSpanningtreeTree(blockTree, nLocalTrees, gp, sgp);
//	printSptTree(sptTree);

	// bfs evaluation with pruning through the poset
	while (border->m != 0) {
		struct Vertex* v = popFromBorder(border, sgp);
		struct Graph* pattern = (struct Graph*)(v->label);
		if (v->visited == 0) {
//			printGraph(pattern);
			char match = 0;
			if (pattern->n == 1) {
				match = 1;
				char* l = pattern->vertices[0]->label;
				for (int v=0; v<g->n; ++v) {
					// will be 0 after loop iff one label matches
					match &= labelCmp(l, g->vertices[v]->label);
				}
				// invert result
				match = !match;
			} else {
				match = noniterativeLocalEasySubtreeCheck(&sptTree, pattern, gp);
				wipeCharacteristicsForLocalEasy(sptTree);
			}

			// garbage collection in SpanningtreeTree
			if (sptTree.characteristics) {
				for (int r=0; r<sptTree.nRoots; ++r) {
					if (sptTree.characteristics[r]) {
						struct SubtreeIsoDataStoreElement* tmp;
						for (struct SubtreeIsoDataStoreElement* e=sptTree.characteristics[r]->first; e!=NULL; e=tmp) {
							tmp = e->next;
							dumpNewCube(e->data.S, e->data.g->n);
							free(e);
						}
					}
					free(sptTree.characteristics[r]);
				}
				free(sptTree.characteristics);
				sptTree.characteristics = NULL;
			}


			++nEvaluations;
			if (match) {
				addIntSortedNoDuplicates(features, v->number - 1);
				v->visited = 1;
				for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
					if (e->endPoint->visited == 0) {
						addToBorder(e->endPoint, border, sgp);
					}
				}
			} else {
				markConnectedComponent(v, -1);
			}
		}
	}
	dumpSpanningtreeTree(sptTree, gp);
	fprintf(stderr, "%i\n", nEvaluations);
	return features;

}

struct IntSet* explicitEmbeddingForAbsImportantTrees(struct Graph* g, struct Graph* F, size_t importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	//cleanup
	for (int v=0; v<F->n; ++v) { F->vertices[v]->visited = 0; }

	// init output
	struct IntSet* features = getIntSet();

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=F->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToBorder(e->endPoint, border, sgp);
	}

	// bfs evaluation with pruning through the poset
	while (border->m != 0) {
		struct Vertex* v = popFromBorder(border, sgp);
		struct Graph* pattern = (struct Graph*)(v->label);
		if (v->visited == 0) {
			char match = isImportantSubtreeAbsolute(g, pattern, importance, gp);
			if (match) {
				addIntSortedNoDuplicates(features, v->number - 1);
				v->visited = 1;
				for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
					if (e->endPoint->visited == 0) {
						addToBorder(e->endPoint, border, sgp);
					}
				}
			} else {
				markConnectedComponent(v, -1);
			}
		}
	}

	return features;

}

struct IntSet* explicitEmbeddingForRelImportantTrees(struct Graph* g, struct Graph* F, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	//cleanup
	for (int v=0; v<F->n; ++v) { F->vertices[v]->visited = 0; }

	// init output
	struct IntSet* features = getIntSet();

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=F->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToBorder(e->endPoint, border, sgp);
	}

	// bfs evaluation with pruning through the poset
	while (border->m != 0) {
		struct Vertex* v = popFromBorder(border, sgp);
		struct Graph* pattern = (struct Graph*)(v->label);
		if (v->visited == 0) {
			char match = isImportantSubtreeRelative(g, pattern, importance, gp);
			if (match) {
				addIntSortedNoDuplicates(features, v->number - 1);
				v->visited = 1;
				for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
					if (e->endPoint->visited == 0) {
						addToBorder(e->endPoint, border, sgp);
					}
				}
			} else {
				markConnectedComponent(v, -1);
			}
		}
	}

	return features;

}

// other approximate kernels

int* dotProductApproximationEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, int* randomProjection, int dimension, struct GraphPool* gp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	for (int i=0; i<dimension; ++i) {
		// if we don't know the value, yet we need to compute it.
		int currentGraphNumber = randomProjection[i];
		if (p.F->vertices[currentGraphNumber] == 0) {
			// evaluate the embedding operator
			struct Graph* currentGraph = (struct Graph*)(p.F->vertices[currentGraphNumber]->label);
			char match = isSubtree(g, currentGraph, gp);
			++nEvaluations;
			if (match) {
				rayOfLight(p.reverseF->vertices[currentGraphNumber], 1, p);
			} else {
				markConnectedComponent(p.F->vertices[currentGraphNumber], -1);
			}
		}
	}

	int* approximateEmbedding = malloc((p.F->n - 1) * sizeof(int));
	if (!approximateEmbedding) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}

	for (int i=1; i<p.F->n; ++i) {
		approximateEmbedding[i-1] = p.F->vertices[i]->visited; // init sketch values to 'infty'
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return approximateEmbedding;

}

int* dotProductApproximationEmbeddingLocalEasy(struct Graph* g, struct EvaluationPlan p, int* randomProjection, int dimension, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	struct SpanningtreeTree sptTree = getSampledSpanningtreeTree(blockTree, nLocalTrees, gp, sgp);

	for (int i=0; i<dimension; ++i) {
		// if we don't know the value, yet we need to compute it.
		int currentGraphNumber = randomProjection[i];
		if (p.F->vertices[currentGraphNumber] == 0) {
			// evaluate the embedding operator
			struct Graph* pattern = (struct Graph*)(p.F->vertices[currentGraphNumber]->label);
			char match = noniterativeLocalEasySubtreeCheck(&sptTree, pattern, gp);
			wipeCharacteristicsForLocalEasy(sptTree);
			++nEvaluations;
			if (match) {
				rayOfLight(p.reverseF->vertices[currentGraphNumber], 1, p);
			} else {
				markConnectedComponent(p.F->vertices[currentGraphNumber], -1);
			}
		}
	}

	dumpSpanningtreeTree(sptTree, gp);

	int* approximateEmbedding = malloc((p.F->n - 1) * sizeof(int));
	if (!approximateEmbedding) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}

	for (int i=1; i<p.F->n; ++i) {
		approximateEmbedding[i-1] = p.F->vertices[i]->visited; // init sketch values to 'infty'
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return approximateEmbedding;

}

