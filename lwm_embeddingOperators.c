/*
 * lwm_embeddingOperators.c
 *
 *  Created on: Oct 28, 2016
 *      Author: pascal
 */

#include <stddef.h>

#include "subtreeIsoUtils.h"
#include "subtreeIsomorphism.h"
#include "iterativeSubtreeIsomorphism.h"
#include "importantSubtrees.h"
#include "localEasySubtreeIsomorphism.h"

#include "lwm_embeddingOperators.h"


void stupidPatternEvaluation(struct Graph** db, int nGraphs, struct Graph** patterns, int nPatterns, struct Vertex** pointers, struct GraphPool* gp) {
	int i;
	for (i=0; i<nGraphs; ++i) {
		int j;
		for (j=0; j<nPatterns; ++j) {
			if (subtreeCheck3(db[i], patterns[j], gp)) {
				++pointers[j]->visited;
			}
		}
	}
}



// WRAPPERS FOR DIFFERENT EMBEDDING OPERATORS

struct SubtreeIsoDataStore noniterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	return noniterativeSubtreeCheck(data, h, gp);
}


struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperatorWithResampling(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;

	result.foundIso = isProbabilisticLocalSampleSubtree(result.g, result.h, (int)importance, gp, sgp);
	return result;
}


struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void) importance;
	(void) sgp;

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;

	struct SpanningtreeTree* sptTree = (struct SpanningtreeTree*)(result.postorder);
//	result.foundIso = isProbabilisticLocalSampleSubtree(result.g, result.h, (int)importance, gp, sgp);
	result.foundIso = subtreeCheckForSpanningtreeTree(sptTree, h, gp);
	wipeCharacteristicsForLocalEasy(*sptTree);
	return result;
}

/**
 * We assume that each block in sptTree has exactly k sampled spanning trees (k may be 1).
 * We create k sptTrees where each is the projection of sptTree to the k'th sampled spanning
 * tree for each block, combined with the spanning tree of the bridges of g.
 */
struct SpanningtreeTree* expandSpanningtreeTree(int* k, struct SpanningtreeTree* sptTree, struct GraphPool* gp) {
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

struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperatorIndependent(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void) importance;
	(void) sgp;

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;

	int k = 0;
	struct SpanningtreeTree* sptTree = (struct SpanningtreeTree*)(result.postorder);
	struct SpanningtreeTree* expanded = expandSpanningtreeTree(&k, sptTree, gp);
	for (int i=0; i<k; ++i) {
		result.foundIso = subtreeCheckForSpanningtreeTree(&(expanded[i]), h, gp);
		if (result.foundIso) break;
	}
	for (int j=0; j<k; ++j) {
		dumpSpanningtreeTree(expanded[j], gp);
	}
	free(expanded);
//	wipeCharacteristicsForLocalEasy(*sptTree);
	return result;
}


struct SubtreeIsoDataStore noniterativeLocalEasySubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)importance; // unused

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;

	result.foundIso = isLocalEasySubtree(result.g, result.h, gp, sgp);
	return result;
}

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


struct SubtreeIsoDataStore relativeImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = isImportantSubtreeRelative(result.g, result.h, importance, gp);
	return result;
}

struct SubtreeIsoDataStore absoluteImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = isImportantSubtreeAbsolute(result.g, result.h, importance, gp);
	return result;
}

struct SubtreeIsoDataStore andorEmbeddingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = andorEmbedding(result.g, result.h, gp);
	return result;
}

struct SubtreeIsoDataStore iterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	return iterativeSubtreeCheck(data, h, gp);
}
