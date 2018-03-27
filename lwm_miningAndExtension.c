#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "graph.h"
#include "searchTree.h"
#include "cs_Parsing.h"
#include "cs_Tree.h"
#include "treeEnumeration.h"

#include "lwm_embeddingOperators.h"
//#include "lwm_initAndCollect.h"

#include "levelwiseGraphMining.h"



/**
 * Get a SubtreeIsoDataStoreList for each IntSet in the input.
 *
 * This method intersects all support sets of the parent patterns of a pattern p and
 * returns a list of data structures that the iterative subtree iso algorithm can use
 * to compute the support set of p.
 *
 * This implementation returns the data structures (cubes and pointers to the parent)
 * of the parent that has id parentIdToKeep.
 */
struct SubtreeIsoDataStoreList* getCandidateSupportSuperSet(struct IntSet* parentIds, struct SubtreeIsoDataStoreList* previousLevelSupportLists, int parentIdToKeep) {
	assert(parentIds != NULL);
	assert(previousLevelSupportLists != NULL);
	assert(containsInt(parentIds, parentIdToKeep));

	// obtain support sets of parents
	struct SubtreeIsoDataStoreList* parentSupportSets = getSupportSetsOfPatterns(previousLevelSupportLists, parentIds);

	// move desired parent support set to head of list to keep the correct subtreeIsoDataStores
	parentSupportSets = subtreeIsoDataStoreChangeHead(parentSupportSets, parentIdToKeep);

	// compute a candidate support set that is the intersection of the support sets of the apriori parents of the extension
	struct SubtreeIsoDataStoreList* candidateSupportSuperSet = intersectSupportSets(parentSupportSets);

	// garbage collection
	while (parentSupportSets) {
		struct SubtreeIsoDataStoreList* tmp = parentSupportSets->next;
		free(parentSupportSets);
		parentSupportSets = tmp;
	}

	return candidateSupportSuperSet;
}


void extendPreviousLevel(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		struct ShallowGraph* extensionEdges,
		size_t threshold,
		// output
		struct SubtreeIsoDataStoreList** resultCandidateSupportSuperSets,
		struct Graph** resultCandidates,
		FILE* logStream,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {
	assert(previousLevelSupportLists != NULL);
	assert(previousLevelSearchTree != NULL);
	assert(extensionEdges != NULL);


	*resultCandidateSupportSuperSets = NULL;
	*resultCandidates = NULL;

	struct Vertex* currentLevelCandidateSearchTree = getVertex(gp->vertexPool);

	int nAllGeneratedExtensions = 0;
	int nAllUniqueGeneratedExtensions = 0;
	int nAllExtensionsPostApriori = 0;
	int nAllExtensionsPostIntersectionFilter = 0;
	int nAddedToOutput = 0;
	int nDumped = 0;

	// generate a list of extensions of all frequent patterns
	// filter these extensions using an apriori property
	// for each extension, compute a list of ids of their apriori parents

	for (struct SubtreeIsoDataStoreList* frequentPatternSupportList=previousLevelSupportLists; frequentPatternSupportList!=NULL; frequentPatternSupportList=frequentPatternSupportList->next) {
		struct Graph* frequentPattern = frequentPatternSupportList->first->data.h;

		// extend frequent pattern
		struct Graph* listOfExtensions = extendPatternOnOuterShells(frequentPattern, extensionEdges, gp, sgp);
//		struct Graph* listOfExtensions = extendPatternOnLeaves(frequentPattern, extensionEdges, gp);
//		struct Graph* listOfExtensions = extendPatternByLargerEdgesTMP(frequentPattern, extensionEdges, gp);

		for (struct Graph* extension=popGraph(&listOfExtensions); extension!=NULL; extension=popGraph(&listOfExtensions)) {
			// count number of generated extensions
			++nAllGeneratedExtensions;

			/* filter out patterns that were already enumerated as the extension of some other pattern
				and are in the search tree */
			struct ShallowGraph* string = canonicalStringOfTree(extension, sgp);
			int previousNumberOfDistinctPatterns = currentLevelCandidateSearchTree->d;
			addToSearchTree(currentLevelCandidateSearchTree, string, gp, sgp);

			struct IntSet* aprioriParentIdSet;
			if (previousNumberOfDistinctPatterns == currentLevelCandidateSearchTree->d) {
				aprioriParentIdSet = NULL;
			} else {
				++nAllUniqueGeneratedExtensions;
				aprioriParentIdSet = aprioriCheckExtensionReturnList(extension, previousLevelSearchTree, gp, sgp);
			}

			if (aprioriParentIdSet) {
				// count number of apriori survivors
				 ++nAllExtensionsPostApriori;

				// get (hopefully small) superset of the support set of the extension
				struct SubtreeIsoDataStoreList* extensionSupportSuperSet = getCandidateSupportSuperSet(aprioriParentIdSet, previousLevelSupportLists, frequentPattern->number);
				dumpIntSet(aprioriParentIdSet);

				// check if support superset is larger than the threshold
				if (extensionSupportSuperSet->size >= threshold) {
					// count number of intersection/threshold survivors
					++nAllExtensionsPostIntersectionFilter;

					// add extension and support super set to list of candidates for next level
					extension->next = *resultCandidates;
					*resultCandidates = extension;
					extensionSupportSuperSet->next = *resultCandidateSupportSuperSets;
					*resultCandidateSupportSuperSets = extensionSupportSuperSet;

					++nAddedToOutput;
				} else {
					// dump extension and support superset
					dumpGraph(gp, extension);
					dumpSubtreeIsoDataStoreListCopy(extensionSupportSuperSet);

					++nDumped;
				}
			} else {
				// dump extension that does not fulfill apriori property
				dumpGraph(gp, extension);
				++nDumped;
			}
		}
	}

	dumpSearchTree(gp, currentLevelCandidateSearchTree);
	fprintf(logStream, "generated extensions: %i\n"
			"unique extensions: %i\n"
			"apriori filtered extensions: %i\n"
			"intersection filtered extensions: %i\n",
			nAllGeneratedExtensions, nAllUniqueGeneratedExtensions, nAllExtensionsPostApriori, nAllExtensionsPostIntersectionFilter);

	assert(nAddedToOutput + nDumped == nAllGeneratedExtensions);
}


struct SubtreeIsoDataStoreList* BFSgetNextLevel(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		size_t threshold,
		struct ShallowGraph* frequentEdges,
		// embedding operator function pointer,
		struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
		double importance,
		// output
		struct Vertex** currentLevelSearchTree,
		FILE* logStream,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {
	assert(previousLevelSupportLists != NULL);
	assert(previousLevelSearchTree != NULL);
	assert(frequentEdges != NULL);

	struct SubtreeIsoDataStoreList* currentLevelCandidateSupportSets;
	struct Graph* currentLevelCandidates;

	extendPreviousLevel(previousLevelSupportLists, previousLevelSearchTree, frequentEdges, threshold,
			&currentLevelCandidateSupportSets, &currentLevelCandidates, logStream,
			gp, sgp);

	//iterate over all patterns in candidateSupports
	struct SubtreeIsoDataStoreList* actualSupportLists = NULL;
	struct SubtreeIsoDataStoreList* actualSupportListsTail = NULL;
	struct SubtreeIsoDataStoreList* candidateSupport = NULL;
	struct Graph* candidate = NULL;
	for (candidateSupport=currentLevelCandidateSupportSets, candidate=currentLevelCandidates; candidateSupport!=NULL; candidateSupport=candidateSupport->next, candidate=candidate->next) {
		struct SubtreeIsoDataStoreList* currentActualSupport = getSubtreeIsoDataStoreList();
		//iterate over all graphs in the support
		for (struct SubtreeIsoDataStoreElement* e=candidateSupport->first; e!=NULL; e=e->next) {
			// create actual support list for candidate pattern
			struct SubtreeIsoDataStore result = embeddingOperator(e->data, candidate, importance, gp, sgp);

			if (result.foundIso) {
				appendSubtreeIsoDataStore(currentActualSupport, result);
			} else {
				dumpNewCube(result.S, result.g->n);
			}
		}
		// filter out candidates with support < threshold
		if (currentActualSupport->size < threshold) {
			// mark h as infrequent
			candidate->activity = 0;
			dumpSubtreeIsoDataStoreList(currentActualSupport);
		} else {
			// mark h as frequent
			candidate->activity = currentActualSupport->size;
			// add to output list, maintaining order. necessary
			if (actualSupportListsTail) {
				actualSupportListsTail->next = currentActualSupport;
				actualSupportListsTail = currentActualSupport;
			} else {
				actualSupportLists = currentActualSupport;
				actualSupportListsTail = currentActualSupport;
			}
		}
	}

	// garbage collection
	candidateSupport = currentLevelCandidateSupportSets;
	while (candidateSupport) {
		struct SubtreeIsoDataStoreList* tmp = candidateSupport->next;
		dumpSubtreeIsoDataStoreListCopy(candidateSupport);
		candidateSupport = tmp;
	}

	// add frequent extensions to current level search tree output, set their numbers correctly
	// dump those candidates that are not frequent
	int nAllFrequentExtensions = 0;
	candidate = currentLevelCandidates;
	while (candidate) {
		struct Graph* tmp = candidate->next;
		candidate->next = NULL;
		if (candidate->activity == 0) {
			dumpGraph(gp, candidate);
		} else {
			++nAllFrequentExtensions;
			struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);
			cString->data = candidate->activity;
			addMultiSetToSearchTree(*currentLevelSearchTree, cString, gp, sgp);
			candidate->number = (*currentLevelSearchTree)->lowPoint;
		}
		candidate = tmp;
	}
	fprintf(logStream, "frequent patterns: %i\n", nAllFrequentExtensions);

	return actualSupportLists;
}


void BFSStrategy(size_t startPatternSize,
					  size_t maxPatternSize,
		              size_t threshold,
					  struct Vertex* initialFrequentPatterns,
					  struct SubtreeIsoDataStoreList* supportSets,
					  struct ShallowGraph* extensionEdges,
					  // embedding operator function pointer,
					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					  double importance,
					  FILE* featureStream,
					  FILE* patternStream,
					  FILE* logStream,
					  struct GraphPool* gp,
					  struct ShallowGraphPool* sgp) {

	// levelwise search for patterns with more than one vertex:
	struct Vertex* previousLevelSearchTree = shallowCopySearchTree(initialFrequentPatterns, gp);
	struct SubtreeIsoDataStoreList* previousLevelSupportSets = supportSets;
	struct Vertex* currentLevelSearchTree = previousLevelSearchTree; // initialization for garbage collection in case of maxPatternSize == 1
	struct SubtreeIsoDataStoreList* currentLevelSupportSets = previousLevelSupportSets; // initialization for garbage collection in case of maxPatternSize == 1

	for (size_t p=startPatternSize+1; (p<=maxPatternSize) && (previousLevelSearchTree->number>0); ++p) {
		fprintf(logStream, "Processing patterns with %zu vertices:\n", p); fflush(logStream);
		currentLevelSearchTree = getVertex(gp->vertexPool);
		offsetSearchTreeIds(currentLevelSearchTree, previousLevelSearchTree->lowPoint);

		currentLevelSupportSets = BFSgetNextLevel(previousLevelSupportSets, previousLevelSearchTree, threshold, extensionEdges, embeddingOperator, importance, &currentLevelSearchTree, logStream, gp, sgp);

		printStringsInSearchTree(currentLevelSearchTree, patternStream, sgp);
		printSubtreeIsoDataStoreListsSparse(currentLevelSupportSets, featureStream);

		// garbage collection:
		// what is now all previousLevel... data structures will not be used at all in the next iteration
		dumpSearchTree(gp, previousLevelSearchTree);
		while (previousLevelSupportSets) {
			struct SubtreeIsoDataStoreList* tmp = previousLevelSupportSets->next;
			// ...hence, we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
			dumpSubtreeIsoDataStoreListWithH(previousLevelSupportSets, gp);
			previousLevelSupportSets = tmp;
		}

		// previous level = current level
		previousLevelSearchTree = currentLevelSearchTree;
		previousLevelSupportSets = currentLevelSupportSets;
	}

//	madness(currentLevelSupportSets, currentLevelSearchTree, extensionEdges, maxPatternSize, threshold, &previousLevelSupportSets, &previousLevelSearchTree, featureStream, patternStream, logStream, gp, sgp);

	// garbage collection
	dumpSearchTree(gp, previousLevelSearchTree);

	while (previousLevelSupportSets) {
		struct SubtreeIsoDataStoreList* tmp = previousLevelSupportSets->next;
		// we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
		dumpSubtreeIsoDataStoreListWithH(previousLevelSupportSets, gp);
		previousLevelSupportSets = tmp;
	}
}
