/**
 * Implementation of an algorithm to find a minimum number of paths covering all vertices in a poset.
 */

#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>

#include "vertexQueue.h"
#include "bipartiteMatching.h"
#include "localEasySubtreeIsomorphism.h"
#include "minhashing.h"
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


void printPathArray(int* path, FILE* out) {
	fputc('[', out);
	for (int i=1; i<path[0]; ++i) {
		fprintf(out, "%i, ", path[i]);
	}
	fputc(']', out);
	fputc('\n', out);
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
	// This should work. TODO pathLength->arrayLength
	int arrayLength = 2;
	for (struct Vertex* x=highestVertex; x->lowPoint!=-1; x=flowInstance->vertices[x->lowPoint]) {
		if (x->number % 2 == 0) {
			++arrayLength;
		}
	}

	// alloc space for path and store vertex ids
	int* path = malloc(arrayLength * sizeof(int));
	path[0] = arrayLength;
	path[1] = v->number / 2;

	// to obtain a path in the original flowInstance, we keep only those vertices that have even ->number and convert them back
	// to original numbers by dividing by two.
	arrayLength -= 1;
	for (struct Vertex* x=highestVertex; arrayLength > 1; x=flowInstance->vertices[x->lowPoint]) {
		if (x->number % 2 == 0) {
			path[arrayLength] = x->number / 2;
			--arrayLength;
		}
	}

	return path;
}


static void countPossiblePaths(struct Graph* flowInstance, size_t* nPaths) {
	// how many paths are there?
	*nPaths = 0;
	for (int v=2; v<flowInstance->n; ++v) {
		if (flowInstance->vertices[v]->visited > 0) {
			*nPaths += flowInstance->vertices[v]->visited;
		}
	}
}


void printFlowInstanceDotFormat(struct Graph* g, FILE* out) {
	fputs("digraph anonymous {\n", out);
	// write vertices
	for (int v=0; v<g->n; ++v) {
		if (g->vertices[v]->visited == 0) {
			fprintf(out, "%i [label=\"%i: %i/%i\"];\n", v, v, g->vertices[v]->visited, g->vertices[v]->d);
		} else {
			fprintf(out, "%i [label=\"%i: %i/%i\", color=green, penwidth=3];\n", v, v, g->vertices[v]->visited, g->vertices[v]->d);
		}
	}
	// write edges
	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->number > e->startPoint->number) {
				if (e->flag > 0) {
					// original edge
					fprintf(out, "%i -> %i [label=\"%i/%i\", penwidth=3, color=red];\n", e->startPoint->number, e->endPoint->number, e->flag, e->used);
				} else {
					fprintf(out, "%i -> %i [label=\"%i/%i\", color=red];\n", e->startPoint->number, e->endPoint->number, e->flag, e->used);
				}
			} else {
				// residual edge
				fprintf(out, "%i -> %i [label=\"%i/%i\"];\n", e->startPoint->number, e->endPoint->number, e->flag, e->used);
			}
		}
	}
	fputs("}\n", out);
}


/**
 * Check whether the given set of paths covers g. Returns the number of uncovered vertices.
 */
int checkPathCover(struct Graph* g, int** pathset, size_t nPaths) {

	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->visited = 0;
	}

	for (size_t i=0; i<nPaths; ++i) {
		for (int j=1; j<pathset[i][0]; ++j) {
			g->vertices[pathset[i][j]]->visited = 1;
		}
	}

	int covered = 0;
	for (int v=0; v<g->n; ++v) {
		covered += g->vertices[v]->visited;
	}

	return g->n - covered;

}


/**
 * Given a poset as directed graph, this algorithm returns a minimum number of paths covering all vertices in the poset.
 *
 * This implementation follows Schrijver, Chapter 14.5a, Thm. 14.7 and Corr. 14.7b. It uses an augmenting path algorithm
 * to find the required maximum flow. It seems to be slower than the one below that uses Goldberg and Tarjans push relabel
 * algorithm for max flow.
 *
 * Assumes that edges in the original poset are guaranteed to go from smaller vertex->number to larger vertex->number.
 * Also assumes that the poset starts with an artificial vertex at position 0 that points to the smallest elements, but
 * is not a part of the poset.
 */
int** getPathCoverOfPosetUsingAugmentingPaths(struct Graph* g, size_t* nPaths, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

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

	// store outflow - inflow in v->visited
	for (int v=2; v<flowInstance->n; ++v) {
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			e->startPoint->visited += e->flag;
			e->endPoint->visited -= e->flag;
		}
	}

	// get minimum number of paths that cover all vertices
	countPossiblePaths(flowInstance, nPaths);

	// find paths from sources to sinks and store them in output
	int** paths = malloc(*nPaths * sizeof(int*));
	int i = 0;
	for (int v=2; v<flowInstance->n; v += 2) {
		struct Vertex* vertex = flowInstance->vertices[v];
		while (vertex->visited > 0) {
			paths[i] = getLongestPathInFlowInstance(vertex, flowInstance, sgp);
			++i;
		}
	}

	dumpGraph(gp, flowInstance);
	return paths;
}


/**
 * Given a poset as directed graph, this algorithm returns a minimum number of paths covering all vertices in the poset.
 *
 * This implementation follows Schrijver, Chapter 14.5a, Thm. 14.7 and Corr. 14.7b. It uses the push relabel algorithm
 * of Goldberg and Tarjan.
 *
 * Assumes that edges in the original poset are guaranteed to go from smaller vertex->number to larger vertex->number.
 * Also assumes that the poset starts with an artificial vertex at position 0 that points to the smallest elements, but
 * is not a part of the poset.
 */
int** getPathCoverOfPoset(struct Graph* g, size_t* nPaths, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct Graph* flowInstance = createVertexCoverFlowInstanceOfPoset(g, gp);
	pushRelabelMaxFlow(flowInstance, flowInstance->vertices[0], flowInstance->vertices[1], sgp);

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

	for (int vi=2; vi<flowInstance->n; ++vi) {
		struct Vertex* v = flowInstance->vertices[vi];
		struct VertexList* keep = NULL;
		v->visited = 0;
		v->d = 0;
		for (struct VertexList* e=v->neighborhood; e!=NULL; e=v->neighborhood) {
			v->neighborhood = e->next;
			if ((e->flag != 0) && (e->startPoint->number < e->endPoint->number)) {
				e->next = keep;
				keep = e;
			} else {
				dumpVertexList(gp->listPool, e);
			}
		}
		v->neighborhood = keep;
	}

	// store outflow - inflow in v->visited
	for (int v=2; v<flowInstance->n; ++v) {
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			e->startPoint->visited += e->flag;
			e->endPoint->visited -= e->flag;
		}
	}

	// get minimum number of paths that cover all vertices
	countPossiblePaths(flowInstance, nPaths);

	// find paths from sources to sinks and store them in output
	int** paths = malloc(*nPaths * sizeof(int*));
	int i = 0;
	for (int v=2; v<flowInstance->n; v += 2) {
		struct Vertex* vertex = flowInstance->vertices[v];
		while (vertex->visited > 0) {
			paths[i] = getLongestPathInFlowInstance(vertex, flowInstance, sgp);
			++i;
		}
	}

	dumpGraph(gp, flowInstance);
	return paths;
}


static int dfsRaySearch(struct Graph* g, struct EvaluationPlan p, struct Vertex* currentPattern, struct GraphPool* gp) {
	int nEvaluations = 0;
	if (!embeddingValueKnown(currentPattern)) {
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
	if (!embeddingValueKnown(currentPattern)) {
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
		char match;
		if (!embeddingValueKnown(p.poset->vertices[path[currentIdx]])) {
			struct Graph* pattern = (struct Graph*)(p.poset->vertices[path[currentIdx]]->label);
			match = isSubtree(g, pattern, gp);
			++nEvaluations;
			updateEvaluationPlan(p, path[currentIdx], match);
		} else {
			match = getMatch(p.poset->vertices[path[currentIdx]]);
		}

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
		char match;
		if (!embeddingValueKnown(p.poset->vertices[path[currentIdx]])) {
			struct Graph* pattern = (struct Graph*)(p.poset->vertices[path[currentIdx]]->label);
			match = subtreeCheckForSpanningtreeTree(spTree, pattern, gp);
			wipeCharacteristicsForLocalEasy(*spTree);
			++nEvaluations;
			updateEvaluationPlan(p, path[currentIdx], match);
		} else {
			match = getMatch(p.poset->vertices[path[currentIdx]]);
		}

		if (match) {
			minIdx = currentIdx + 1;
		} else {
			maxIdx = currentIdx - 1;
		}
	}

	return nEvaluations;
}

static double* getWeightsOfPath(struct Graph* poset, int* path, int databaseSize) {
	double* weights = malloc(path[0] * sizeof(double));

	weights[0] = (double)path[0];
	for (int i=1; i<path[0]; ++i) {
		weights[i] = ((struct Graph*)poset->vertices[path[i]]->label)->activity / (double)databaseSize;
	}
	return weights;
}

/**
 * We assume that the weights are strictly positive. This is reasonable for our use case of frequent patterns where the weights correspond
 * to the empirical frequency on the training data set.
 *
 * The computed 'knowledge score' is based on the conditional probability of an element in the chain
 * given that we know the embedding / truth value of the current pattern. The knowledge score is minimal if the
 * conditional probability is equal to 0.5. We want to return the index that minimizes the score.
 */
static void wbs_getNextIdx(char isMatch, double* probabilities, size_t* currentIdx, size_t* minIdx, size_t* maxIdx) {

	double probabilityOfCurrentMatch = probabilities[*currentIdx];
	double bestKnowledgeScore = 1.0; // the value of this can be at most 0.5

	if (isMatch) {
		*minIdx = *currentIdx + 1;
		for (size_t i=*minIdx; i<=*maxIdx; ++i) {
			double knowledgeScore = fabs(probabilities[i] / probabilityOfCurrentMatch - 0.5);
			if (knowledgeScore < bestKnowledgeScore) {
				*currentIdx = i;
				bestKnowledgeScore = knowledgeScore;
			} // TODO if the value increases then we can stop. this is not implemented, yet.
		}
	} else {
		*maxIdx = *currentIdx - 1;
		for (size_t i=*minIdx; i<=*maxIdx; ++i) {
			double knowledgeScore = fabs((probabilities[i] - probabilityOfCurrentMatch) / (1.0 - probabilityOfCurrentMatch) - 0.5);
			if (knowledgeScore < bestKnowledgeScore) {
				*currentIdx = i;
				bestKnowledgeScore = knowledgeScore;
			} // TODO if the value increases then we can stop. this is not implemented, yet.
		}
	}
}

static int weightedBinarySearchEvaluationLE(struct SpanningtreeTree* spTree, struct EvaluationPlan p, int* path, double* probabilities, struct GraphPool* gp) {
	size_t nEvaluations = 0;
	size_t minIdx = 1;
	size_t maxIdx = (size_t)path[0] - 1;
	size_t currentIdx = 0;

	wbs_getNextIdx(1, probabilities, &currentIdx, &minIdx, &maxIdx);

	while (minIdx <= maxIdx) {

		char match;
		if (!embeddingValueKnown(p.poset->vertices[path[currentIdx]])) {
			struct Graph* pattern = (struct Graph*)(p.poset->vertices[path[currentIdx]]->label);
			match = subtreeCheckForSpanningtreeTree(spTree, pattern, gp);
			wipeCharacteristicsForLocalEasy(*spTree);
			++nEvaluations;
			updateEvaluationPlan(p, path[currentIdx], match);
		} else {
			match = getMatch(p.poset->vertices[path[currentIdx]]);
		}

		wbs_getNextIdx(1, probabilities, &currentIdx, &minIdx, &maxIdx);
	}

	return nEvaluations;
}

static int weightedBinarySearchEvaluation(struct Graph* g, struct EvaluationPlan p, int* path, double* probabilities, struct GraphPool* gp) {
	size_t nEvaluations = 0;
	size_t minIdx = 1;
	size_t maxIdx = (size_t)path[0] - 1;
	size_t currentIdx = 0;

	wbs_getNextIdx(1, probabilities, &currentIdx, &minIdx, &maxIdx);

	while (minIdx <= maxIdx) {

		char match;
		if (!embeddingValueKnown(p.poset->vertices[path[currentIdx]])) {
			struct Graph* pattern = (struct Graph*)(p.poset->vertices[path[currentIdx]]->label);
			match = isSubtree(g, pattern, gp);
			++nEvaluations;
			updateEvaluationPlan(p, path[currentIdx], match);
		} else {
			match = getMatch(p.poset->vertices[path[currentIdx]]);
		}

		wbs_getNextIdx(1, probabilities, &currentIdx, &minIdx, &maxIdx);
	}

	return nEvaluations;
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
		if (!embeddingValueKnown(p.poset->vertices[i])) {
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
		if (!embeddingValueKnown(p.poset->vertices[i])) {
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


struct IntSet* latticeLongestWeightedPathEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, int databaseSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	int nEvaluations = 0;
	cleanEvaluationPlan(p);

	for (int i=1; i<p.poset->n; ++i) {
		if (!embeddingValueKnown(p.poset->vertices[i])) {
			int* path = getLongestPathInDAG(p.poset->vertices[i], p.poset, sgp);
			double* weights = getWeightsOfPath(p.poset, path, databaseSize);
			nEvaluations += weightedBinarySearchEvaluation(g, p, path, weights, gp);
			free(path);
			free(weights);
		}
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

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, 1, gp, sgp);

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

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, 1, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (int i=1; i<p.poset->n; ++i) {
		if (!embeddingValueKnown(p.poset->vertices[i])) {
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

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, 1, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (int i=1; i<p.poset->n; ++i) {
		if (!embeddingValueKnown(p.poset->vertices[i])) {
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

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, 1, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (size_t i=0; i<p.sketchSize; ++i) {
		nEvaluations += binarySearchEvaluationLE(&spTree, p, p.shrunkPermutations[i], gp);
	}

	dumpSpanningtreeTree(spTree, gp);

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}


struct IntSet* latticeLongestWeightedPathEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, int databaseSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SpanningtreeTree spTree = getSampledSpanningtreeTree(getBlockTreeT(g, sgp), sampleSize, 1, gp, sgp);

	int nEvaluations = 0;
	cleanEvaluationPlan(p);
	for (int i=1; i<p.poset->n; ++i) {
		if (!embeddingValueKnown(p.poset->vertices[i])) {
			int* path = getLongestPathInDAG(p.poset->vertices[i], p.poset, sgp);
			double* weights = getWeightsOfPath(p.poset, path, databaseSize);
			nEvaluations += weightedBinarySearchEvaluationLE(&spTree, p, path, weights, gp);
			free(path);
			free(weights);
		}
	}

	dumpSpanningtreeTree(spTree, gp);

	fprintf(stderr, "%i\n", nEvaluations);
	return patternPosetInfoToFeatureSet(p);
}

