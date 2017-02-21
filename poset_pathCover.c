/**
 * Implementation of an algorithm to find a minimum number of paths covering all vertices in a poset.
 */

#include <malloc.h>
#include <stddef.h>
#include <limits.h>

#include "vertexQueue.h"
#include "bipartiteMatching.h"
#include "localEasySubtreeIsomorphism.h"
#include "poset_pathCover.h"



/**
 * Count the number of reachable vertices.
 *
 * Assumes that the ->visited flags of all reachable vertices can be set to v->number to mark visited vertices.
 */
int numberOfReachableVertices(struct Vertex* v, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* queue = getShallowGraph(sgp);
	int markerID = v->number;
	addToVertexQueue(v, queue, sgp);
	v->visited = markerID;

	int reachableCount = 1;

	for (struct Vertex* w=popFromVertexQueue(queue, sgp); w!=NULL; w=popFromVertexQueue(queue, sgp)) {
		for (struct VertexList* e=w->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->visited != markerID) {
				e->endPoint->visited = markerID;
				addToVertexQueue(e->endPoint, queue, sgp);
				++reachableCount;
			}
		}
	}

	return reachableCount;
}


/**
 * Compute the number of reachable vertices for all vertices in g and store the value in ->lowPoint.
 * Uses ->visited flags of vertices. Does not clean up.
 */
void computeAllReachabilityCounts(struct Graph* g, struct ShallowGraphPool* sgp) {
	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->visited = -1;
	}

	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->lowPoint = numberOfReachableVertices(g->vertices[v], sgp);
	}
}



static struct Graph* createVertexCoverFlowInstanceOfPoset(struct Graph* g, struct GraphPool* gp) {
	struct Graph* flowInstance = createGraph(2 * g->n, gp);
	int maxCapacity = g->n;

	// source and sink of s-t flow instance
	struct Vertex* s = flowInstance->vertices[0];
	struct Vertex* t = flowInstance->vertices[1];

	// actual elements of poset start at vertex 1. they have infinite capacity.
	// HINT: if you ever want to change this for general graphs, consider that the
	// above s,t positions and size of flowInstance need to be changed.
	for (int v=1; v<g->n; ++v) {
		struct Vertex* vStart = flowInstance->vertices[2 * v];
		struct Vertex* vEnd = flowInstance->vertices[2 * v + 1];

		// add edge for each vertex in the original graph
		addResidualEdgesWithCapacity(vStart, vEnd, maxCapacity, gp->listPool);

		// add copy of original edges and residuals, they have infinite capacity
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			addResidualEdgesWithCapacity(vEnd, flowInstance->vertices[2 * e->endPoint->number], maxCapacity, gp->listPool);
		}

		// add edge from s to vEnd with capacity 1
		addResidualEdgesWithCapacity(s, vEnd, 1, gp->listPool);

		// add edge from v to t
		addResidualEdgesWithCapacity(vStart, t, 1, gp->listPool);
	}

	return flowInstance;
}


static int* getLongestPathInFlowInstance(struct Vertex* v, struct Graph* flowInstance, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* border = getShallowGraph(sgp);
	addToVertexQueue(v, border, sgp);
	v->lowPoint = -1;
	v->d = 1;

	// use ->lowPoint to store the a parent of w
	// for sake of simplicity we count the number of times a vertex is in the border and
	// at the end of this loop highestVertex stores the last visited pattern that,
	// by construction, is on the highest level of the poset.
	struct Vertex* highestVertex = v;
	for (struct Vertex* w=popFromVertexQueue(border, sgp); w!=NULL; w=popFromVertexQueue(border, sgp)) {
		highestVertex = w;
		w->d -= 1;
		if (w->d == 0) {
			for (struct VertexList* e=w->neighborhood; e!=NULL; e=e->next) {
				if ((e->flag > 0) && (e->endPoint->d == 0)) {
					e->endPoint->lowPoint = w->number;
					e->endPoint->d += 1;
					addToVertexQueue(e->endPoint, border, sgp);
				}
			}
		}
	}

	// remove path from graph by deleting flow on edges and fixing excess at start and endpoint of path
	for (struct Vertex* x=highestVertex; x->lowPoint!=-1; x=flowInstance->vertices[x->lowPoint]) {
		for (struct VertexList* e=flowInstance->vertices[x->lowPoint]->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint == x) {
				--e->flag;
				break; // ...looping over neighbors
			}
		}
	}
	--v->visited;
	++highestVertex->visited;

	// we want to obtain paths in the original poset we obtained flowInstance from.
	// Therefore we count only those vertices on the current path in flowinstance that have even ->number
	// and will store their ->numbers divided by two to obtain the vertex ids in the original poset.
	// This should work.
	int pathLength = 2;
	for (struct Vertex* x=highestVertex; x->lowPoint!=-1; x=flowInstance->vertices[x->lowPoint]) {
		if (x->number % 2 == 0) {
			++pathLength;
		}
	}

	// alloc space for path and store vertex ids
	int* path = malloc(pathLength * sizeof(int));
	path[0] = pathLength;
	path[1] = v->number / 2;

	// to obtain a path in the original flowInstance, we keep only those vertices that have even ->number and convert them back
	// to original numbers by dividing by two.
	pathLength -= 1;
	for (struct Vertex* x=highestVertex; pathLength > 1; x=flowInstance->vertices[x->lowPoint]) {
		if (x->number % 2 == 0) {
			path[pathLength] = x->number / 2;
			--pathLength;
		}
	}

	return path;
}



/**
 *
 * Assumes that edges in the original poset are guaranteed to go from smaller vertex->number to larger vertex->number.
 * Also assumes that the poset starts with an artificial vertex at position 0 that points to the smallest elements, but
 * is not a part of the poset.
 */
int** getPathCoverOfPoset(struct Graph* g, size_t* nPaths, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct Graph* flowInstance = createVertexCoverFlowInstanceOfPoset(g, gp);

	// find max number of s-t paths
	int nAugmentations = 0;
	while (augmentWithCapacity(flowInstance->vertices[0], flowInstance->vertices[1])) {
		++nAugmentations;
	}

	// add one flow value to those edges that correspond to vertices
	for (int v=2; v<flowInstance->n; v+=2) {
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->number == v+1) {
				e->flag += 1;
				break; // iterating over the neighbors
			}
		}
	}

	// remove all edges that have 0 flow, all residual edges (assumed to go from larger vertex id to smaller),
	// and all edges containing s and t
	// init ->visited of vertices
	dumpVertexList(gp->listPool, flowInstance->vertices[0]->neighborhood);
	dumpVertexList(gp->listPool, flowInstance->vertices[1]->neighborhood);
	flowInstance->vertices[0]->neighborhood = NULL;
	flowInstance->vertices[1]->neighborhood = NULL;

	for (int v=2; v<flowInstance->n; ++v) {
		struct VertexList* keep = NULL;
		flowInstance->vertices[v]->visited = 0;
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=flowInstance->vertices[v]->neighborhood) {
			flowInstance->vertices[v]->neighborhood = e->next;
			if ((e->flag != 0) && (e->startPoint->number < e->endPoint->number)) {
				e->next = keep;
				keep = e;
			} else {
				dumpVertexList(gp->listPool, e);
			}
		}
		flowInstance->vertices[v]->neighborhood = keep;
	}

	// store outdegree - indegree in v->visited
	for (int v=2; v<flowInstance->n; ++v) {
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			e->startPoint->visited += 1;
			e->endPoint->visited -= 1;
		}
	}

	// how many paths are there?
	*nPaths = 0;
	for (int v=2; v<flowInstance->n; ++v) {
		if (flowInstance->vertices[v]->visited > 0) {
			*nPaths += flowInstance->vertices[v]->visited;
		}
	}

	// find paths from sources to sinks and store them in output
	int** paths = malloc(*nPaths * sizeof(int*));
	for (int v=2, i=0; v<flowInstance->n; ++v) {
		while (flowInstance->vertices[v]->visited > 0) {
			paths[i] = getLongestPathInFlowInstance(flowInstance->vertices[v], flowInstance, sgp);
			++i;
		}
	}

	dumpGraph(gp, flowInstance);
	return paths;
}



static int dfsRaySearch(struct Graph* g, struct EvaluationPlan p, struct Vertex* currentPattern, struct GraphPool* gp) {
	int nEvaluations = 0;
	if (currentPattern->visited == 0) {
		struct Graph* pattern = (struct Graph*)(currentPattern->label);
		char match = isSubtree(g, pattern, gp);
		++nEvaluations;
		if (match) {
			for (struct VertexList* e=currentPattern->neighborhood; e!=NULL; e=e->next) {
				nEvaluations += dfsRaySearch(g, p, e->endPoint, gp);
			}
			updateEvaluationPlan(p, currentPattern->number, 1);
		} else {
			updateEvaluationPlan(p, currentPattern->number, 0);
		}
	}
	return nEvaluations;
}

static int dfsRaySearchLE(struct SpanningtreeTree* spTree, struct EvaluationPlan p, struct Vertex* currentPattern, struct GraphPool* gp) {
	int nEvaluations = 0;
	if (currentPattern->visited == 0) {
		struct Graph* pattern = (struct Graph*)(currentPattern->label);
		char match = subtreeCheckForSpanningtreeTree(spTree, pattern, gp);
		wipeCharacteristicsForLocalEasy(*spTree);
		++nEvaluations;
		if (match) {
			for (struct VertexList* e=currentPattern->neighborhood; e!=NULL; e=e->next) {
				nEvaluations += dfsRaySearchLE(spTree, p, e->endPoint, gp);
			}
			updateEvaluationPlan(p, currentPattern->number, 1);
		} else {
			updateEvaluationPlan(p, currentPattern->number, 0);
		}
	}
	return nEvaluations;
}


static int* getPathInDAG(struct Vertex* v) {
	int pathLength = 2;
	int* path = NULL;

	// DFS without backtracking in DAG
	for (struct VertexList* e=v->neighborhood; e!=NULL && (e->endPoint->visited == 0); e=e->endPoint->neighborhood) {
		++pathLength;
	}

	path = malloc(pathLength * sizeof(int));
	path[0] = pathLength;
	path[1] = v->number;

	pathLength = 2;
	// DFS without backtracking in DAG
	for (struct VertexList* e=v->neighborhood; e!=NULL && (e->endPoint->visited == 0); e=e->endPoint->neighborhood) {
		path[pathLength] = e->endPoint->number;
		++pathLength;
	}

	return path;
}

static int* getLongestPathInDAG(struct Vertex* v, struct Graph* poset, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* border = getShallowGraph(sgp);
	addToVertexQueue(v, border, sgp);
	v->lowPoint = -1;
	v->d = 1;

	// use ->lowPoint to store the a parent of w
	// for sake of simplicity we count the number of times a vertex is in the border and
	struct Vertex* highestVertex = v;
	for (struct Vertex* w=popFromVertexQueue(border, sgp); w!=NULL;  w=popFromVertexQueue(border, sgp)) {
		highestVertex = w;
		w->d -= 1;
		if (w->d == 0) {
			for (struct VertexList* e=w->neighborhood; e!=NULL; e=e->next) {
				if ((e->endPoint->visited == 0) && (e->endPoint->d == 0)) {
					e->endPoint->lowPoint = w->number;
					e->endPoint->d += 1;
					addToVertexQueue(e->endPoint, border, sgp);
				}
			}
		}
	}
	// here w stores the last visited pattern that, by construction, is on the highest level of the poset.
	// backtrack using the ->lowPoints from here.
	int pathLength = 2;
	int* path = NULL;
	for (struct Vertex* x=highestVertex; x->lowPoint!=-1; x=poset->vertices[x->lowPoint]) {
		++pathLength;
	}

	path = malloc(pathLength * sizeof(int));
	path[0] = pathLength;
	path[1] = v->number;

	pathLength -= 1;
	for (struct Vertex* x=highestVertex; x->lowPoint!=-1; x=poset->vertices[x->lowPoint]) {
		path[pathLength] = x->number;
		--pathLength;
	}

	return path;
}


static int binarySearchEvaluation(struct Graph* g, struct EvaluationPlan p, int* path, struct GraphPool* gp) {
	int nEvaluations = 0;
	int minIdx = 1;
	int maxIdx = path[0] - 1;

	while (minIdx <= maxIdx) {
		int currentIdx = (minIdx + maxIdx) / 2;

		struct Graph* pattern = (struct Graph*)(p.poset->vertices[path[currentIdx]]->label);
		char match = isSubtree(g, pattern, gp);
		++nEvaluations;
		updateEvaluationPlan(p, path[currentIdx], match);

		if (match) {
			minIdx = currentIdx + 1;
		} else {
			maxIdx = currentIdx - 1;
		}
	}

	return nEvaluations;
}

static int binarySearchEvaluationLE(struct SpanningtreeTree* spTree, struct EvaluationPlan p, int* path, struct GraphPool* gp) {
	int nEvaluations = 0;
	int minIdx = 1;
	int maxIdx = path[0] - 1;

	while (minIdx <= maxIdx) {
		int currentIdx = (minIdx + maxIdx) / 2;

		struct Graph* pattern = (struct Graph*)(p.poset->vertices[path[currentIdx]]->label);
		char match = subtreeCheckForSpanningtreeTree(spTree, pattern, gp);
		wipeCharacteristicsForLocalEasy(*spTree);
		++nEvaluations;
		updateEvaluationPlan(p, path[currentIdx], match);

		if (match) {
			minIdx = currentIdx + 1;
		} else {
			maxIdx = currentIdx - 1;
		}
	}

	return nEvaluations;
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


// FOR SUBTREEISO EMBEDDING OPERATOR

/**
 * Given a forest g and a pattern poset p, we want to compute an embedding of g into the space spanned by p.
 *
 * For this, the method uses the same idea as the fast minhash embedding computation method above to possibly deduce
 * embedding information of other patterns by single calls to the embedding oracle (i.e. subtree isomorphism).
 *
 * This variant goes through all patterns in order of their ids in the evaluation plan. If a pattern is not yet evaluated,
 * a simple dfs in the pattern space is started from this pattern. Once the dfs reaches an element in the negative border,
 * it marks all patterns above and including it negative and all elements below and including the previous pattern positive.
 *
 * This method outputs an IntSet containing the matching pattern ids.
 */
struct IntSet* dfsDownwardEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (int i=1; i<p.poset->n; ++i) {
		nEvaluations += dfsRaySearch(g, p, p.poset->vertices[i], gp);
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}

struct IntSet* latticePathEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	for (int i=1; i<p.poset->n; ++i) {
		if (p.poset->vertices[i]->visited == 0) {
			int* path = getPathInDAG(p.poset->vertices[i]);
			nEvaluations += binarySearchEvaluation(g, p, path, gp);
			free(path);
		}
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}

struct IntSet* latticeLongestPathEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	for (int i=1; i<p.poset->n; ++i) {
		if (p.poset->vertices[i]->visited == 0) {
			int* path = getLongestPathInDAG(p.poset->vertices[i], p.poset, sgp);
			nEvaluations += binarySearchEvaluation(g, p, path, gp);
			free(path);
		}
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}


struct IntSet* staticPathCoverEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	for (size_t i=0; i<p.sketchSize; ++i) {
		nEvaluations += binarySearchEvaluation(g, p, p.shrunkPermutations[i], gp);
	}

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}


// LOCAL EASY EMBEDDING OPERATOR

/**
 * Given a forest g and a pattern poset p, we want to compute an embedding of g into the space spanned by p.
 *
 * For this, the method uses the same idea as the fast minhash embedding computation method above to possibly deduce
 * embedding information of other patterns by single calls to the embedding oracle (i.e. subtree isomorphism).
 *
 * This variant goes through all patterns in order of their ids in the evaluation plan. If a pattern is not yet evaluated,
 * a simple dfs in the pattern space is started from this pattern. Once the dfs reaches an element in the negative border,
 * it marks all patterns above and including it negative and all elements below and including the previous pattern positive.
 *
 * This method outputs an IntSet containing the matching pattern ids.
 */
struct IntSet* dfsDownwardEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (int i=1; i<p.poset->n; ++i) {
		nEvaluations += dfsRaySearchLE(&spTree, p, p.poset->vertices[i], gp);
	}

	dumpSpanningtreeTree(spTree, gp);

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}


struct IntSet* latticePathEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (int i=1; i<p.poset->n; ++i) {
		if (p.poset->vertices[i]->visited == 0) {
			int* path = getPathInDAG(p.poset->vertices[i]);
			nEvaluations += binarySearchEvaluationLE(&spTree, p, path, gp);
			free(path);
		}
	}

	dumpSpanningtreeTree(spTree, gp);

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}

struct IntSet* latticeLongestPathEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (int i=1; i<p.poset->n; ++i) {
		if (p.poset->vertices[i]->visited == 0) {
			int* path = getLongestPathInDAG(p.poset->vertices[i], p.poset, sgp);
			nEvaluations += binarySearchEvaluationLE(&spTree, p, path, gp);
			free(path);
		}
	}

	dumpSpanningtreeTree(spTree, gp);

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}


struct IntSet* staticPathCoverEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (size_t i=0; i<p.sketchSize; ++i) {
		nEvaluations += binarySearchEvaluationLE(&spTree, p, p.shrunkPermutations[i], gp);
	}

	dumpSpanningtreeTree(spTree, gp);

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}

