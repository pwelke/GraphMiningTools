/*
 * lwm_embeddingOperators.c
 *
 *  Created on: Oct 28, 2016
 *      Author: pascal
 */

#include <stddef.h>
#include <stdlib.h>

#include "subtreeIsoUtils.h"
#include "iterativeSubtreeIsomorphism.h"
#include "importantSubtrees.h"
#include "localEasySubtreeIsomorphism.h"
#include "subtreeIsomorphismSampling.h"

#include "lwm_embeddingOperators.h"


void stupidPatternEvaluation(struct Graph** db, int nGraphs, struct Graph** patterns, int nPatterns, struct Vertex** pointers, struct GraphPool* gp) {
	int i;
	for (i=0; i<nGraphs; ++i) {
		int j;
		for (j=0; j<nPatterns; ++j) {
			if (isSubtree(db[i], patterns[j], gp)) {
				++pointers[j]->visited;
			}
		}
	}
}



// WRAPPERS FOR DIFFERENT EMBEDDING OPERATORS

/**
 * Exact embedding Operator for
 * tree pattern h
 * forest transaction data
 */
struct SubtreeIsoDataStore subtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	return noniterativeSubtreeCheck(data, h, gp);
}


/**
 * Exact embedding operator for
 * tree pattern h
 * forest transaction data
 *
 * The algorithm expects data to contain information on a direct predecessor of h.
 */
struct SubtreeIsoDataStore iterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	return iterativeSubtreeCheck(data, h, gp);
}


/**
 * Embedding operator with one-sided error for
 * tree pattern h
 * arbitrary graph transaction data
 *
 * importance must be an integer, specifying the number of sampled spanning trees per root node.
 * Note that this embedding operator resamples the local spanning trees and hence does not necessarily fulfil the downward closure / apriori property.
 */
struct SubtreeIsoDataStore localEasySubtreeCheckOperatorWithResampling(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;

	result.foundIso = isProbabilisticLocalSampleSubtree(result.g, result.h, (int)importance, gp, sgp);
	return result;
}


/**
 * Embedding operator that might be exact or have one-sided error for
 * tree pattern h
 * arbitrary graph transaction data
 *
 * Whether this operator is exact, or not, depends on the initialization of the local spanning tree data structure stored in data.postorder.
 * If this contains all local spanning trees, the algorithm is exact, if it only contains a subset, the algorithm has one-sided error.
 */
struct SubtreeIsoDataStore localEasySubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)importance; // unused
	(void)sgp; // unused

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	struct SpanningtreeTree* sptTree = (struct SpanningtreeTree*)data.postorder;

	result.foundIso = subtreeCheckForSpanningtreeTree(sptTree, h, gp);

	// clean up the spanning tree tree
	wipeCharacteristicsForLocalEasy(*sptTree);

	return result;
}


/**
 * Non-standard embedding operator for
 * tree pattern h
 * forest transaction data
 *
 * importance must be a double between 0.0 and 1.0
 * This operator returns true, if the pattern h occurs in at least importance * c connected components of the transaction forest data,
 * where c is the total number of connected components in data.
 *
 * If data is the full set of spanning trees of a graph, then this operator decides whether h is a mu-important tree in data (compare my dissertation).
 */
struct SubtreeIsoDataStore relativeImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = isImportantSubtreeRelative(result.g, result.h, importance, gp);
	return result;
}


/**
 * Non-standard embedding operator for
 * tree pattern h
 * forest transaction data
 *
 * importance must be an integer
 * This operator returns true, if the pattern h occurs in at least importance connected components of the transaction forest data.
 *
 */
struct SubtreeIsoDataStore absoluteImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = isImportantSubtreeAbsolute(result.g, result.h, importance, gp);
	return result;
}


/**
 * Embedding operator for
 * any object h
 * any transaction data
 *
 * that always returns true.
 *
 * It is intended e.g. for enumeration of all tree patterns up to isomorphism.
 */
struct SubtreeIsoDataStore alwaysReturnTrue(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)importance; // unused
	(void)sgp; // unused
	(void)gp; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = 1;
	return result;
}



// EXPERIMENTAL

/**
 * We assume that each block in sptTree has exactly k sampled spanning trees (k may be 1).
 * We create k sptTrees where each is the projection of sptTree to the k'th sampled spanning
 * tree for each block, combined with the spanning tree of the bridges of g.
 */
static struct SpanningtreeTree* expandSpanningtreeTree(int* k, struct SpanningtreeTree* sptTree, struct GraphPool* gp) {
	// find k
	*k = 0;
	for (int i=0; i<sptTree->nRoots; ++i) {
		int nLocalTrees = 0;
		for (struct Graph* localTree=sptTree->localSpanningTrees[i]; localTree!=NULL; localTree=localTree->next) {
			++nLocalTrees;
		}
		if (nLocalTrees > *k) {
			*k = nLocalTrees;
		}
	}

	struct SpanningtreeTree* results = malloc(*k * sizeof(struct SpanningtreeTree));
	for (int j=0; j<*k; ++j) {
		results[j] = *sptTree;
		results[j].localSpanningTrees = malloc(sptTree->nRoots * sizeof(struct Graph*));
		results[j].characteristics = malloc(sptTree->nRoots * sizeof(struct SupportSet*));
		for (int i=0; i<sptTree->nRoots; ++i) {
			results[j].characteristics[i] = getSupportSet();
		}
	}

	for (int i=0; i<sptTree->nRoots; ++i) {
		if (sptTree->localSpanningTrees[i]->next == NULL) {
			// clone bridge part k times
			for (int j=0; j<*k; ++j) {
				results[j].localSpanningTrees[i] = cloneGraph(sptTree->localSpanningTrees[i], gp);
			}
		} else {
			// copy each component once
			int j=0;
			for (struct Graph* localTree=sptTree->localSpanningTrees[i]; localTree!=NULL; localTree=localTree->next) {
				results[j].localSpanningTrees[i] = cloneGraph(localTree, gp);
				++j;
			}
		}
	}
	return results;
}

struct SubtreeIsoDataStore localEasySamplingSubtreeCheckOperatorIndependent(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void) importance;
	(void) sgp;

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;

	printf("pattern:\n");
	printGraphAidsFormat(h, stdout);

	int k = 0;
	struct SpanningtreeTree* sptTree = (struct SpanningtreeTree*)(result.postorder);
	struct SpanningtreeTree* expanded = expandSpanningtreeTree(&k, sptTree, gp);
	printf("%i spanningtreetrees:\n", k);
	for (int j=0; j<k; ++j) {
		printSptTree(expanded[j]);
		result.foundIso = subtreeCheckForSpanningtreeTree(&(expanded[j]), h, gp);
		printf("found iso: %i\n", result.foundIso);
		if (result.foundIso) break;
	}
	for (int j=0; j<k; ++j) {
		expanded[j].parents = NULL;
		expanded[j].roots = NULL;
		dumpSpanningtreeTree(expanded[j], gp);
	}
	free(expanded);
//	wipeCharacteristicsForLocalEasy(*sptTree);
	return result;
}


/**
 * Randomized embedding operator for
 * tree pattern h
 * graph transaction data
 *
 * The algorithm repeats to try to embed a randomly rooted shuffled version of the tree h
 * in some random place in the transaction graph. If it succeeds at some point, it returns 1.
 * This is another example of an embedding operator with one-sided error.
 */
struct SubtreeIsoDataStore subtreeIsomorphismSamplingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)gp; // unused
	(void)sgp; // unused

	struct SubtreeIsoDataStore result = {0};
	result.g = data.g;
	result.h = h;

	for (int i=0; i<importance; ++i) {
		result.foundIso = subtreeIsomorphismSampler(data.g, h);
		if (result.foundIso) {
			break;
		}
	}

	return result;
}


/**
 * Randomized embedding operator for
 * tree pattern h
 * graph transaction data
 *
 * The algorithm repeats to try to embed a randomly rooted shuffled version of the tree h
 * in some random place in the transaction graph. If it succeeds at some point, it returns 1.
 * This is another example of an embedding operator with one-sided error.
 *
 * This variant also shuffles the neighbors of the image vertex. should result in better
 * recall due to more randomness, but is slower.
 */
struct SubtreeIsoDataStore subtreeIsomorphismSamplingOperatorWithImageShuffling(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)gp; // unused
	(void)sgp; // unused

	struct SubtreeIsoDataStore result = {0};
	result.g = data.g;
	result.h = h;

	for (int i=0; i<importance; ++i) {
		result.foundIso = subtreeIsomorphismSamplerWithImageShuffling(data.g, h);
		if (result.foundIso) {
			break;
		}
	}

	return result;
}


/**
 * Randomized embedding operator for
 * tree pattern h
 * graph transaction data
 *
 * The algorithm repeats to try to embed a randomly rooted shuffled version of the tree h
 * in some random place in the transaction graph. If it succeeds at some point, it returns 1.
 * This is another example of an embedding operator with one-sided error.
 *
 * This variant shuffles the neighbors of the image vertex and the neighbors of the source vertex.
 * In addition, it computes a maximum matching (in contrast to the maximal matchings that the above two
 * variants of the FK algorithm compute). This should result in better recall but might be slower.
 */
struct SubtreeIsoDataStore subtreeIsomorphismSamplingOperatorWithMatching(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused

	struct SubtreeIsoDataStore result = {0};
	result.g = data.g;
	result.h = h;

	for (int i=0; i<importance; ++i) {
		result.foundIso = subtreeIsomorphismSamplerWithProperMatching(data.g, h, gp);
		if (result.foundIso) {
			break;
		}
	}

	return result;
}
