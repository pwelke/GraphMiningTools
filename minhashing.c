#include <stdlib.h>
#include <malloc.h>

#include "graph.h"
#include "searchTree.h"
#include "cs_Tree.h"
#include "listComponents.h"

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

/** Return a random permutation of the numbers from 0 to n-1 */
int* getRandomPermutation(int n) {
	int* permutation = malloc(n * sizeof(int));
	if (permutation) {
		for (int i=0; i<n; ++i) {
			permutation[i] = i;
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
int posetPermutationMark(int* permutation, int n, struct Graph* F) {
	for (int v=0; v<F->n; ++v) {
		F->vertices[v]->visited = 0;
	}
	int shrunkSize = 0;
	for (int i=0; i<n; ++i) {
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
int* posetPermutationShrink(int* permutation, int n, int shrunkSize) {
	int* condensedSequence = malloc(shrunkSize * sizeof(int));
	int posInCondensed = 0;
	for (int i=0; i<n; ++i) {
		if (permutation[i] != -1) {
			condensedSequence[posInCondensed] = permutation[i];
			++posInCondensed;
		}
	}
	return condensedSequence;
}

// BUILD TREE POSET


static int addEdgesFromSubtrees(struct Graph* pattern, struct Vertex* searchtree, struct Graph* F, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

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
				addEdgeBetweenVertices(subtreeID, pattern->number, NULL, F, gp);
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
 * Build a poset of trees given by their canononical strings, ordered by subgraph isomorphism.
 *
 * Input: a list of canonical strings of trees, ordered by number of vertices
 * Output: a graph F where each vertex corresponds to a canonical string.
 *
 * - The vertex number of a canonical string is given by the position of the
 *   canonical string in the input list.
 * - The vertex->label points to a graph representation of the canonical string in the input list (be
 *   careful to cast correctly to struct Graph* when using)
 * - There is an edge (v,w) \in E(F) <=> |V(v)| = |V(w)| - 1 AND v is subgraph
 *   isomorphic to w
 */
struct Graph* buildTreePosetFromStrings(struct ShallowGraph* strings, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	// count number of strings, add strings to a search tree with correct numbers
	int nStrings = 0;
	struct Vertex* searchTree = getVertex(gp->vertexPool);
	for (struct ShallowGraph* s=strings; s!=NULL; s=s->next) {
		addToSearchTree(searchTree, cloneShallowGraph(s, sgp), gp, sgp);
		++nStrings;
	}

	// create graph with vertices corresponding to strings.
	struct Graph* F = createGraph(nStrings, gp);
	nStrings = 0;
	for (struct ShallowGraph* s=strings; s!=NULL; s=s->next) {
		struct Graph* pattern = treeCanonicalString2Graph(s, gp);
		F->vertices[nStrings]->label = (char*)pattern; // misuse of char* pointer
		addEdgesFromSubtrees(pattern, searchTree, F, gp, sgp);
		++nStrings;
	}

	return F;
}

/**
 * Build a poset of trees given as graphs, ordered by subgraph isomorphism.
 *
 * Input: a list of graphs that are trees, ordered by number of vertices
 * Output: a graph F where each vertex corresponds to a canonical string.
 *
 * - The vertex number of a canonical string is given by the position of the
 *   canonical string in the input list.
 * - The vertex->label points to a graph representation of the canonical string in the input list (be
 *   careful to cast correctly to struct Graph* when using)
 * - There is an edge (v,w) \in E(F) <=> |V(v)| = |V(w)| - 1 AND v is subgraph
 *   isomorphic to w
 */
struct Graph* buildTreePosetFromGraphs(struct Graph* graphs, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	// count number of strings, add strings to a search tree with correct numbers
	int nStrings = 0;
	struct Vertex* searchTree = getVertex(gp->vertexPool);
	for (struct Graph* g=graphs; g!=NULL; g=g->next) {
		struct ShallowGraph* string = canonicalStringOfTree(g, sgp);
		addToSearchTree(searchTree, string, gp, sgp);
		++nStrings;
	}

	// create graph with vertices corresponding to strings.
	struct Graph* F = createGraph(nStrings, gp);
	nStrings = 0;
	for (struct Graph* g=graphs; g!=NULL; g=g->next) {
		F->vertices[nStrings]->label = (char*)g; // misuse of char* pointer
		addEdgesFromSubtrees(g, searchTree, F, gp, sgp);
		++nStrings;
	}

	return F;
}

// PARSING HELPER FUNCTION

struct ShallowGraph* readFrequentPatternStrings(FILE* stream, struct ShallowGraphPool* sgp) {
	char* buffer[50];

	int count = -1;
	int id = -1;
	struct ShallowGraph* strings = NULL;
	struct ShallowGraph* stringsTail = NULL;
	while (fscanf(stream, "%i\t%i\t", &count, &id) == 2) {
		struct ShallowGraph* string = parseCString(stream, buffer, sgp);
		if (stringsTail) {
			stringsTail->next = string;
			stringsTail = string;
		} else {
			strings = stringsTail = string;
		}
	}
	return strings;
}
