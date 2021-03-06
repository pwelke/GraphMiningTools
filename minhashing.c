// this source file uses qsort_r which is not part of C99, but a GNU specific extension.
#define _GNU_SOURCE

#include <stdlib.h>

#include "graph.h"
#include "searchTree.h"
#include "cs_Tree.h"
#include "cs_Parsing.h"
#include "iterativeSubtreeIsomorphism.h"
#include "listComponents.h"
#include "intSet.h"
#include "importantSubtrees.h"
#include "localEasySubtreeIsomorphism.h"
#include "subtreeIsoUtils.h"
#include "vertexQueue.h"

#include "minhashing.h"

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


struct EvaluationPlan buildMinHashEvaluationPlan(int** shrunkPermutations, size_t* permutationSizes, size_t K, struct Graph* F, struct GraphPool* gp) {
	struct EvaluationPlan p = {0};
	p.poset = F;
	p.reversePoset = reverseGraph(F, gp);
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
		}

	} else {
		fprintf(stderr, "could not allocate space for evaluation plan. this program will break now.\n");
		p.poset = NULL;
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
	for (int v=1; v<p.poset->n; ++v) { // start from 1, as empty pattern has no graph attached
		dumpGraph(gp, (struct Graph*)(p.poset->vertices[v]->label));
		p.poset->vertices[v]->label = NULL;
	}
	dumpGraph(gp, p.reversePoset);
	dumpGraph(gp, p.poset);
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

	struct IntSet* subgraphs = getIntSet();

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
			int graphID = getID(searchtree, subString);
			addIntSortedNoDuplicates(subgraphs, graphID);
			dumpShallowGraph(sgp, subString);

			// restore law and order in current (and invalidate subgraph)
			addEdge(edge->startPoint, edge);
		}
	}

	// to filter out duplicates, we add the edges from a list just now.
	size_t missingSubgraphs = 0;
	for (struct IntElement* graphID=subgraphs->first; graphID!=NULL; graphID=graphID->next) {
		if (graphID->value != -1) {
			struct VertexList* e = getVertexList(gp->listPool);
			e->startPoint = F->vertices[graphID->value];
			e->endPoint = F->vertices[pattern->number];
			addEdge(e->startPoint, e);
			F->m += 1;
			++edgeCount;
		} else {
//			fprintf(stderr, "subtree not found in searchtree. this should not happen if you provided a pattern set that is closed wrt. subgraph iso.\n");
			++missingSubgraphs;
		}
	}

	if (missingSubgraphs == subgraphs->size) {
		struct VertexList* e = getVertexList(gp->listPool);
		e->startPoint = F->vertices[0];
		e->endPoint = F->vertices[pattern->number];
		addEdge(e->startPoint, e);
		F->m += 1;
		++edgeCount;
		fprintf(stderr, "found minimal pattern %i with %i vertices\n", pattern->number, pattern->n);
	} else {
		if (missingSubgraphs > 0) {
			fprintf(stderr, "pattern %i is not minimal, but not all subpatterns are present\n", pattern->n);
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
	dumpIntSet(subgraphs);
	dumpGraph(gp, subgraph);

	return edgeCount;
}


static int graphIdCmp(const void* a, const void* b) {
	struct Graph* g = *(struct Graph**)a;
	struct Graph* h = *(struct Graph**)b;
	return g->n - h->n;
}

/**
 * Build a poset of trees given as graphs, ordered by subgraph isomorphism.
 *
 * Input: a list of graphs that are trees
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

	// if patterns are not ordered by number of vertices, sort db array accordingly before adding edges
	for (int i=0; i<nGraphs-1; ++i) {
		if (db[i]->n < db[i+1]->n) {
			fprintf(stderr, "reordering patterns by number of vertices.\nThis changes pattern ids!\n");
			qsort(db, nGraphs, sizeof(struct Graph*), &graphIdCmp);
			break;
		}
	}

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


void cleanEvaluationPlan(struct EvaluationPlan p) {
	for (int i=0; i<p.poset->n; ++i) {
		p.poset->vertices[i]->visited = 0;
		p.reversePoset->vertices[i]->visited = 0;
	}
}


/** check whether the match value of pattern given by v is yet unknown */
inline int embeddingValueKnown(struct Vertex* v) {
	return v->visited != 0;
}

inline int getMatch(struct Vertex* v) {
	return v->visited == 1 ? 1 : 0;
}

/**
Traverses the reverse graph of the poset graph F and marks all vertices reachable from v as matches.

Hence, v needs to be a vertex in the reverse graph p.reversePoset !
 */
static void rayOfLight(struct Vertex* v, struct EvaluationPlan p) {

	/* mark vertex as visited */
	v->visited = 1;
	p.poset->vertices[v->number]->visited = 1;


	/*recursive call for all neighbors that are not visited so far */
	for (struct VertexList* index = v->neighborhood; index; index = index->next) {
		if (index->endPoint->visited != 1) {
			rayOfLight(index->endPoint, p);
		}
	}
}


/**
Traverses the pattern poset and marks all vertices reachable from v as non-matches.

Hence, v needs to be a vertex in the graph p.poset !
 */
static void rayOfDoom(struct Vertex* v, struct EvaluationPlan p) {

	/* mark vertex as visited */
	v->visited = -1;
	p.reversePoset->vertices[v->number]->visited = -1;


	/*recursive call for all neighbors that are not visited so far */
	for (struct VertexList* index = v->neighborhood; index; index = index->next) {
		if (index->endPoint->visited != -1) {
			rayOfDoom(index->endPoint, p);
		}
	}
}


/**
 * Whenever we shoot somewhere in the pattern poset and evaluate the embedding operator
 * for some pattern given by patternId, there are two possible results:
 *
 * - The pattern is a match: Then by the monotonicity all patterns smaller than the current
 *   in the poset are matches as well. We mark these positive.
 *   - The pattern is no match: Then by the monotonicity all patterns larger than the current
 *     in the poset are no matches as well. We mark these negative.
 */
void updateEvaluationPlan(struct EvaluationPlan p, int patternId, char match) {
	if (match) {
		rayOfLight(p.reversePoset->vertices[patternId], p);
	} else {
		rayOfDoom(p.poset->vertices[patternId], p);
	}
}


int getPositiveBorderSize(struct EvaluationPlan p) {
	int borderSize = 0;
	for (int v=0; v<p.poset->n; ++v) {
		int inBorder = 1;
		for (struct VertexList* e=p.poset->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			inBorder &= getMatch(e->endPoint);
		}
		borderSize += inBorder;
	}
	return borderSize;
}


// CREATE FEATURE SET

struct IntSet* patternPosetInfoToFeatureSet(struct EvaluationPlan p) {
	struct IntSet* features = getIntSet();
	for (int i=1; i<p.poset->n; ++i) {
		if (p.poset->vertices[i]->visited == 1) {
			addIntSortedNoDuplicates(features, i - 1);
		}
	}
	return features;
}



// COMPUTATION OF MINHASHES


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
		if (p.poset->vertices[currentGraphNumber]->visited != 0) {
			// either the pattern with id currentGraphNumber was evaluated positively and we hence have found the
			// min value for the current permutation or we need to continue
			if (p.poset->vertices[currentGraphNumber]->visited == 1) {
				sketch[current.permutation] = current.level;
			}
			continue;
		}

		// evaluate the embedding operator
		struct Graph* currentGraph = (struct Graph*)(p.poset->vertices[currentGraphNumber]->label);
		char match = isSubtree(g, currentGraph, gp);
		++nEvaluations;

		updateEvaluationPlan(p, currentGraphNumber, match);
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return sketch;
}


int* fastMinHashForAbsImportantTrees(struct Graph* g, struct EvaluationPlan p, int importance, struct GraphPool* gp) {
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
		if (p.poset->vertices[currentGraphNumber]->visited != 0) {
			// either the pattern with id currentGraphNumber was evaluated positively and we hence have found the
			// min value for the current permutation or we need to continue
			if (p.poset->vertices[currentGraphNumber]->visited == 1) {
				sketch[current.permutation] = current.level;
			}
			continue;
		}

		// evaluate the embedding operator
		struct Graph* currentGraph = (struct Graph*)(p.poset->vertices[currentGraphNumber]->label);
		char match = isImportantSubtreeAbsolute(g, currentGraph, importance, gp);
		++nEvaluations;

		updateEvaluationPlan(p, currentGraphNumber, match);
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return sketch;
}


int* fastMinHashForRelImportantTrees(struct Graph* g, struct EvaluationPlan p, double importance, struct GraphPool* gp) {
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
		if (p.poset->vertices[currentGraphNumber]->visited != 0) {
			// either the pattern with id currentGraphNumber was evaluated positively and we hence have found the
			// min value for the current permutation or we need to continue
			if (p.poset->vertices[currentGraphNumber]->visited == 1) {
				sketch[current.permutation] = current.level;
			}
			continue;
		}

		// evaluate the embedding operator
		struct Graph* currentGraph = (struct Graph*)(p.poset->vertices[currentGraphNumber]->label);
		char match = isImportantSubtreeRelative(g, currentGraph, importance, gp);
		++nEvaluations;

		updateEvaluationPlan(p, currentGraphNumber, match);
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return sketch;
}



// FOR COMPARISON: BFS EVALUATION USING THE PATTERN POSET


struct IntSet* bfsEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=p.poset->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToVertexQueue(e->endPoint, border, sgp);
		e->endPoint->d = 1;
	}

	for (struct Vertex* v=popFromVertexQueue(border, sgp); v!=NULL; v=popFromVertexQueue(border, sgp)) {
		v->d = 0;

		char match = 0;
		if (!embeddingValueKnown(v)) {
			struct Graph* pattern = (struct Graph*)(v->label);
			match = isSubtree(g, pattern, gp);
			++nEvaluations;
			updateEvaluationPlan(p, v->number, match);
		} else {
			match = getMatch(v);
		}

		// add extensions of pattern to border
		if (match) {
			for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
				if (e->endPoint->d == 0) {
					addToVertexQueue(e->endPoint, border, sgp);
					e->endPoint->d = 1;
				}
			}
		}
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}


struct IntSet* bfsEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=p.poset->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToVertexQueue(e->endPoint, border, sgp);
		e->endPoint->d = 1;
	}

	struct SpanningtreeTree sptTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), nLocalTrees, 1, gp, sgp);

	for (struct Vertex* v=popFromVertexQueue(border, sgp); v!=NULL; v=popFromVertexQueue(border, sgp)) {
		v->d = 0;

		char match = 0;
		if (!embeddingValueKnown(v)) {
			struct Graph* pattern = (struct Graph*)(v->label);
			match = subtreeCheckForSpanningtreeTree(&sptTree, pattern, gp);
			wipeCharacteristicsForLocalEasy(sptTree);
			++nEvaluations;
			updateEvaluationPlan(p, v->number, match);
		} else {
			match = getMatch(v);
		}

		// add extensions of pattern to border
		if (match) {
			for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
				if (e->endPoint->d == 0) {
					addToVertexQueue(e->endPoint, border, sgp);
					e->endPoint->d = 1;
				}
			}
		}
	}

	fprintf(stderr, "%i\n", nEvaluations);
	dumpSpanningtreeTree(sptTree, gp);
	return patternPosetInfoToFeatureSet(p);
}


struct IntSet* bfsEmbeddingForAbsImportantTrees(struct Graph* g, struct EvaluationPlan p, size_t importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=p.poset->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToVertexQueue(e->endPoint, border, sgp);
		e->endPoint->d = 1;
	}

	for (struct Vertex* v=popFromVertexQueue(border, sgp); v!=NULL; v=popFromVertexQueue(border, sgp)) {
		v->d = 0;

		char match = 0;
		if (!embeddingValueKnown(v)) {
			struct Graph* pattern = (struct Graph*)(v->label);
			match = isImportantSubtreeAbsolute(g, pattern, importance, gp);
			++nEvaluations;
			updateEvaluationPlan(p, v->number, match);
		} else {
			match = getMatch(v);
		}

		// add extensions of pattern to border
		if (match) {
			for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
				if (e->endPoint->d == 0) {
					addToVertexQueue(e->endPoint, border, sgp);
					e->endPoint->d = 1;
				}
			}
		}
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}


struct IntSet* bfsEmbeddingForRelImportantTrees(struct Graph* g, struct EvaluationPlan p, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	// add minimal elements to border
	struct ShallowGraph* border = getShallowGraph(sgp);
	for (struct VertexList* e=p.poset->vertices[0]->neighborhood; e!=NULL; e=e->next) {
		addToVertexQueue(e->endPoint, border, sgp);
		e->endPoint->d = 1;
	}

	for (struct Vertex* v=popFromVertexQueue(border, sgp); v!=NULL; v=popFromVertexQueue(border, sgp)) {
		v->d = 0;

		char match = 0;
		if (!embeddingValueKnown(v)) {
			struct Graph* pattern = (struct Graph*)(v->label);
			match = isImportantSubtreeRelative(g, pattern, importance, gp);
			++nEvaluations;
			updateEvaluationPlan(p, v->number, match);
		} else {
			match = getMatch(v);
		}

		// add extensions of pattern to border
		if (match) {
			for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
				if (e->endPoint->d == 0) {
					addToVertexQueue(e->endPoint, border, sgp);
					e->endPoint->d = 1;
				}
			}
		}
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}



// DOT PRODUCT APPROXIMATION BY RANDOM PROJECTIONS ! TODO NOT PROPERLY TESTED

/**
 * Given a forest g, a pattern poset p and a subset of patterns represented by a (random) projection, we want to compute an embedding of g
 * into the full space spanned by p by only evaluating (at most) all patterns in projection.
 *
 *
 * For this, the method uses the same idea as the fast minhash embedding computation method above.
 *
 * This method outputs an array o indexed by pattern ids containing values in {0,1,-1} where
 * o[i] == 1 : pattern i matches g
 * o[i] == -1: pattern i does not match g
 * o[i] == 0 : it is unknown whether i matches g or not.
 */
int* fullEmbeddingProjectionApproximationForTrees(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, struct GraphPool* gp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	for (int i=0; i<projectionSize; ++i) {
		// if we don't know the value we need to compute it.
		int currentGraphNumber = projection[i];
		if (!embeddingValueKnown(p.poset->vertices[currentGraphNumber])) {
			// evaluate the embedding operator
			struct Graph* currentGraph = (struct Graph*)(p.poset->vertices[currentGraphNumber]->label);
			char match = isSubtree(g, currentGraph, gp);
			++nEvaluations;
			updateEvaluationPlan(p, currentGraphNumber, match);
		}
	}

	int* approximateEmbedding = malloc((p.poset->n - 1) * sizeof(int));
	if (!approximateEmbedding) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}

	for (int i=1; i<p.poset->n; ++i) {
		approximateEmbedding[i-1] = p.poset->vertices[i]->visited; // init sketch values to 'infty'
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return approximateEmbedding;

}


/**
 * Given a graph g, a pattern poset p and a subset of patterns represented by a (random) projection, we want to compute an embedding of g
 * into the full space spanned by p by only evaluating (at most) all patterns in projection. This method uses the sampled local spanning tree
 * embedding embedding algorithm with sampling parameter nLocalTrees.
 *
 *
 * For this, the method uses the same idea as the fast minhash embedding computation method above.
 *
 * This method outputs an array o indexed by pattern ids containing values in {0,1,-1} where
 * o[i] == 1 : pattern i matches g
 * o[i] == -1: pattern i does not match g
 * o[i] == 0 : it is unknown whether i matches g or not.
 */
int* fullEmbeddingProjectionApproximationLocalEasy(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	struct SpanningtreeTree sptTree = getSampledSpanningtreeTree(blockTree, nLocalTrees, 1, gp, sgp);

	for (int i=0; i<projectionSize; ++i) {
		// if we don't know the value we need to compute it.
		int currentGraphNumber = projection[i];
		if (!embeddingValueKnown(p.poset->vertices[currentGraphNumber])) {
			// evaluate the embedding operator
			struct Graph* pattern = (struct Graph*)(p.poset->vertices[currentGraphNumber]->label);
			char match = subtreeCheckForSpanningtreeTree(&sptTree, pattern, gp);
			wipeCharacteristicsForLocalEasy(sptTree);
			++nEvaluations;
			updateEvaluationPlan(p, currentGraphNumber, match);
		}
	}

	dumpSpanningtreeTree(sptTree, gp);

	int* approximateEmbedding = malloc((p.poset->n - 1) * sizeof(int));
	if (!approximateEmbedding) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}

	for (int i=1; i<p.poset->n; ++i) {
		approximateEmbedding[i-1] = p.poset->vertices[i]->visited; // init sketch values to 'infty'
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return approximateEmbedding;

}


/**
 * Given a forest g, a pattern poset p and a subset of patterns represented by a (random) projection, we want to compute an embedding of g
 * into the space spanned by the subset of p given by projection by only evaluating (at most) all patterns in projection.
 *
 *
 * For this, the method uses the same idea as the fast minhash embedding computation method above.
 *
 * This method outputs an array o indexed by numbers from 0 to projectionSize-1 containing values in {-1,1} where
 * o[i] == 1 : pattern i matches g
 * o[i] == -1: pattern i does not match g
 */
int* randomProjectionEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, struct GraphPool* gp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	// alloc output array
	int* approximateEmbedding = malloc(projectionSize * sizeof(int));
	if (!approximateEmbedding) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}

	for (int i=0; i<projectionSize; ++i) {

		// if we don't know the value we need to compute it.
		int currentGraphNumber = projection[i];
		if (!embeddingValueKnown(p.poset->vertices[currentGraphNumber])) {

			// evaluate the embedding operator
			struct Graph* currentGraph = (struct Graph*)(p.poset->vertices[currentGraphNumber]->label);
			char match = isSubtree(g, currentGraph, gp);

			// update output
			approximateEmbedding[i] = match;

			// update evaluation Plan
			++nEvaluations;
			updateEvaluationPlan(p, currentGraphNumber, match);
		}
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return approximateEmbedding;

}


/**
 * Given a graph g, a pattern poset p and a subset of patterns represented by a (random) projection, we want to compute an embedding of g
 * into the space spanned by the subset of p given by projection by only evaluating (at most) all patterns in projection. This method uses
 * the sampled local spanning tree embedding embedding algorithm with sampling parameter nLocalTrees.
 *
 * For this, the method uses the same idea as the fast minhash embedding computation method above.
 *
 * This method outputs an array o indexed by numbers from 0 to projectionSize-1 containing values in {-1,1} where
 * o[i] == 1 : pattern i matches g
 * o[i] == -1: pattern i does not match g
 */
int* randomProjectionEmbeddingLocalEasy(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	struct BlockTree blockTree = getBlockTreeT(g, sgp);
	struct SpanningtreeTree sptTree = getSampledSpanningtreeTree(blockTree, nLocalTrees, 1, gp, sgp);

	// alloc output array
	int* approximateEmbedding = malloc((p.poset->n - 1) * sizeof(int));
	if (!approximateEmbedding) {
		fprintf(stderr, "Could not allocate memory for sketch. This is a bad thing.\n");
		return NULL;
	}

	for (int i=0; i<projectionSize; ++i) {

		// if we don't know the value we need to compute it.
		int currentGraphNumber = projection[i];
		if (!embeddingValueKnown(p.poset->vertices[currentGraphNumber])) {

			// evaluate the embedding operator
			struct Graph* pattern = (struct Graph*)(p.poset->vertices[currentGraphNumber]->label);
			char match = subtreeCheckForSpanningtreeTree(&sptTree, pattern, gp);
			wipeCharacteristicsForLocalEasy(sptTree);

			// update output
			approximateEmbedding[i] = match;

			// update evaluation plan
			++nEvaluations;
			updateEvaluationPlan(p, currentGraphNumber, match);
		}
	}

	dumpSpanningtreeTree(sptTree, gp);

	fprintf(stderr, "%i\n", nEvaluations);
	return approximateEmbedding;

}

