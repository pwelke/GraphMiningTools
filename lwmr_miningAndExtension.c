#include "lwm_miningAndExtension.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "graph.h"
#include "searchTree.h"
#include "cs_Parsing.h"
#include "cs_Tree.h"
#include "treeEnumerationRooted.h"

#include "lwmr_miningAndExtension.h"


static void _extendPreviousLevelRooted(// input
		struct SupportSet* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		struct ShallowGraph* extensionEdges,
		size_t threshold,
		char useAprioriPruning,
		// output
		struct SupportSet** resultCandidateSupportSuperSets,
		struct Graph** resultCandidates,
		FILE* logStream,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

	assert(previousLevelSupportLists != NULL);
	assert(extensionEdges != NULL);

	if (useAprioriPruning) {
		assert(previousLevelSearchTree != NULL);
	}


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

	for (struct SupportSet* frequentPatternSupportList=previousLevelSupportLists; frequentPatternSupportList!=NULL; frequentPatternSupportList=frequentPatternSupportList->next) {
		struct Graph* frequentPattern = frequentPatternSupportList->first->data.h;

		// extend frequent pattern
		struct Graph* listOfExtensions = extendRootedPatternAllWays(frequentPattern, extensionEdges, gp);

		for (struct Graph* extension=popGraph(&listOfExtensions); extension!=NULL; extension=popGraph(&listOfExtensions)) {
			// count number of generated extensions
			++nAllGeneratedExtensions;

			/* filter out patterns that were already enumerated as the extension of some other pattern
				and are in the search tree; here a distinction between rooted and unrooted trees takes place */
			struct ShallowGraph* string = canonicalStringOfRootedTree(extension->vertices[0], extension->vertices[0], sgp);
			int previousNumberOfDistinctPatterns = currentLevelCandidateSearchTree->d;
			addToSearchTree(currentLevelCandidateSearchTree, string, gp, sgp);

			if (useAprioriPruning) {
				struct IntSet* aprioriParentIdSet;
				if (previousNumberOfDistinctPatterns == currentLevelCandidateSearchTree->d) {
					aprioriParentIdSet = NULL;
				} else {
					++nAllUniqueGeneratedExtensions;
					aprioriParentIdSet = aprioriCheckExtensionRootedReturnList(extension, previousLevelSearchTree, gp, sgp);
				}

				if (aprioriParentIdSet) {
					// count number of apriori survivors
					++nAllExtensionsPostApriori;

					// get (hopefully small) superset of the support set of the extension
					struct SupportSet* extensionSupportSuperSet = getCandidateSupportSuperSet(aprioriParentIdSet, previousLevelSupportLists, frequentPattern->number);
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
						dumpSupportSetCopy(extensionSupportSuperSet);

						++nDumped;
					}
				} else {
					// dump extension that does not fulfill apriori property
					dumpGraph(gp, extension);
					++nDumped;
				}
			} else {

				// add extension and support super set to list of candidates for next level 
				// without apriori et al. filtering, we keep each unique candidate and its support superset is just the support of the parent pattern.
				extension->next = *resultCandidates;
				*resultCandidates = extension;
				struct SupportSet *extensionSupportSuperSet = shallowCopySupportSet(frequentPatternSupportList);
				extensionSupportSuperSet->next = *resultCandidateSupportSuperSets;
				*resultCandidateSupportSuperSets = extensionSupportSuperSet;

				++nAddedToOutput;
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


static struct SupportSet* _getNextLevelRooted(// input
		struct SupportSet* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		size_t threshold,
		struct ShallowGraph* frequentEdges,
		// embedding operator function pointer,
		struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
		double importance,
		char useAprioriPruning,
		// output
		struct Vertex** currentLevelSearchTree,
		FILE* logStream,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {
	assert(previousLevelSupportLists != NULL);
	assert(previousLevelSearchTree != NULL);
	assert(frequentEdges != NULL);

	struct SupportSet* currentLevelCandidateSupportSets;
	struct Graph* currentLevelCandidates;

	_extendPreviousLevelRooted(previousLevelSupportLists, previousLevelSearchTree, frequentEdges, threshold, useAprioriPruning, 
			&currentLevelCandidateSupportSets, &currentLevelCandidates, logStream,
			gp, sgp);

	//iterate over all patterns in candidateSupports
	struct SupportSet* actualSupportLists = NULL;
	struct SupportSet* actualSupportListsTail = NULL;
	struct SupportSet* candidateSupport = NULL;
	struct Graph* candidate = NULL;
	for (candidateSupport=currentLevelCandidateSupportSets, candidate=currentLevelCandidates; 
	     candidateSupport!=NULL; 
		 candidateSupport=candidateSupport->next, candidate=candidate->next) {

		struct SupportSet* currentActualSupport = getSupportSet();

		//iterate over all graphs in the support
		for (struct SupportSetElement* e=candidateSupport->first; e!=NULL; e=e->next) {
			// create actual support list for candidate pattern
			struct SubtreeIsoDataStore result = embeddingOperator(e->data, candidate, importance, gp, sgp);

			if (result.foundIso) {
				appendSupportSetData(currentActualSupport, result);
			} else {
				dumpNewCube(result.S, result.g->n);
			}
		}

		// filter out candidates with support < threshold
		if (currentActualSupport->size < threshold) {
			// mark h as infrequent
			candidate->activity = 0;
			dumpSupportSet(currentActualSupport);
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
	if (useAprioriPruning) {
		// every candidate has its own candidate support set. dump all of them individually
		candidateSupport = currentLevelCandidateSupportSets;
		while (candidateSupport) {
			struct SupportSet* tmp = candidateSupport->next;
			candidateSupport->next = NULL;
			dumpSupportSetCopy(candidateSupport);
			candidateSupport = tmp;
		}
		fflush(stdout);
	} else {
		// all candidates share the same candidate support set. dump it and free the headers.
		candidateSupport = currentLevelCandidateSupportSets->next;
		dumpSupportSetCopy(currentLevelCandidateSupportSets);
		while(candidateSupport) {
			struct SupportSet* tmp = candidateSupport->next;
			free(candidateSupport);
			candidateSupport = tmp;
		}
	}

	// add frequent extensions to current level search tree output, set their numbers correctly
	// dump those candidates that are not frequent
	// TODO we could save reevaluating seen patterns by storing them in this or another search tree
	int nAllFrequentExtensions = 0;
	candidate = currentLevelCandidates;
	while (candidate) {
		struct Graph* tmp = candidate->next;
		candidate->next = NULL;
		if (candidate->activity == 0) {
			dumpGraph(gp, candidate);
		} else {
			++nAllFrequentExtensions;
			struct ShallowGraph* cString = canonicalStringOfRootedTree(candidate->vertices[0], candidate->vertices[0], sgp);
			cString->data = candidate->activity;
			addMultiSetToSearchTree(*currentLevelSearchTree, cString, gp, sgp);
			candidate->number = (*currentLevelSearchTree)->lowPoint;
		}
		candidate = tmp;
	}
	fprintf(logStream, "frequent patterns: %i\n", nAllFrequentExtensions);

	return actualSupportLists;
}


void BFSStrategyRooted(size_t startPatternSize,
					  size_t maxPatternSize,
		              size_t threshold,
					  struct Vertex* initialFrequentPatterns,
					  struct SupportSet* supportSets,
					  struct ShallowGraph* extensionEdges,
					  // embedding operator function pointer,
					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					  double importance,
					  FILE* featureStream,
					  FILE* patternStream,
					  FILE* logStream,
					  struct GraphPool* gp,
					  struct ShallowGraphPool* sgp) {

	const char useAprioriPruning = 1;

	// levelwise search for patterns with more than one vertex:
	struct Vertex* previousLevelSearchTree = shallowCopySearchTree(initialFrequentPatterns, gp);
	struct SupportSet* previousLevelSupportSets = supportSets;
	struct Vertex* currentLevelSearchTree = previousLevelSearchTree; // initialization for garbage collection in case of maxPatternSize == 1
	struct SupportSet* currentLevelSupportSets = previousLevelSupportSets; // initialization for garbage collection in case of maxPatternSize == 1

	for (size_t p=startPatternSize+1; (p<=maxPatternSize) && (previousLevelSearchTree->number>0); ++p) {
		fprintf(logStream, "Processing patterns with %zu vertices:\n", p); fflush(logStream);
		currentLevelSearchTree = getVertex(gp->vertexPool);
		offsetSearchTreeIds(currentLevelSearchTree, previousLevelSearchTree->lowPoint);

		currentLevelSupportSets = _getNextLevelRooted(previousLevelSupportSets, previousLevelSearchTree, threshold, extensionEdges, embeddingOperator, importance, useAprioriPruning, &currentLevelSearchTree, logStream, gp, sgp);

		printStringsInSearchTree(currentLevelSearchTree, patternStream, sgp);
		printSupportSetsSparse(currentLevelSupportSets, featureStream);

		// garbage collection:
		// what is now all previousLevel... data structures will not be used at all in the next iteration
		dumpSearchTree(gp, previousLevelSearchTree);
		while (previousLevelSupportSets) {
			struct SupportSet* tmp = previousLevelSupportSets->next;
			// ...hence, we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
			dumpSupportSetWithPattern(previousLevelSupportSets, gp);
			previousLevelSupportSets = tmp;
		}

		// previous level = current level
		previousLevelSearchTree = currentLevelSearchTree;
		previousLevelSupportSets = currentLevelSupportSets;
	}

	// garbage collection
	dumpSearchTree(gp, previousLevelSearchTree);

	while (previousLevelSupportSets) {
		struct SupportSet* tmp = previousLevelSupportSets->next;
		// we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
		dumpSupportSetWithPattern(previousLevelSupportSets, gp);
		previousLevelSupportSets = tmp;
	}
}


void DFSStrategyRooted(size_t startPatternSize,
					  size_t maxPatternSize,
		              size_t threshold,
					  struct Vertex* initialFrequentPatterns,
					  struct SupportSet* supportSets,
					  struct ShallowGraph* extensionEdges,
					  // embedding operator function pointer,
					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					  double importance,
					  FILE* featureStream,
					  FILE* patternStream,
					  FILE* logStream,
					  struct GraphPool* gp,
					  struct ShallowGraphPool* sgp) {

	const char dontUseApriori = 0;

	// init an array of searchtrees, one for each level
	// this should speed up the alg but will increase memory consumption in the long run
	struct Vertex** levelSearchTrees = malloc(maxPatternSize * sizeof(struct Vertex*));
	for (size_t i=0; i<maxPatternSize; ++i) {
		if (i != startPatternSize - 1) {
			levelSearchTrees[i] = getVertex(gp->vertexPool);
		} else {
			levelSearchTrees[i] = initialFrequentPatterns;
		}
	}

	// the frontier of candidate patterns
	struct SupportSet* candidateSupportSetStack = supportSets;
	size_t patternID = initialFrequentPatterns->lowPoint;

	while (candidateSupportSetStack) {

		// support set of current pattern, pattern, and pattern size
		struct SupportSet* currentPatternSupport = popSupportSet(&candidateSupportSetStack);
		struct Graph *currentPattern = currentPatternSupport->first->data.h;
		size_t currentPatternSize = currentPattern->n;

		// we extend all patterns that have at most maxPatternSize - 1 vertices
		if (currentPatternSize < maxPatternSize) {

			// logging
			fprintf(logStream, "Processing pattern %i with %zu vertices:\n", currentPattern->number, currentPatternSize);
			fflush(logStream);

			// extend current pattern and check frequencies
			struct Vertex* newFrequentPatternSearchTree = getVertex(gp->vertexPool);
			offsetSearchTreeIds(newFrequentPatternSearchTree, patternID);
			// TODO add infrequent patterns to some other search tree and output.
			struct SupportSet *newSupportSets = _getNextLevelRooted(currentPatternSupport, levelSearchTrees[currentPatternSize - 1], threshold, extensionEdges, embeddingOperator, importance, dontUseApriori, &newFrequentPatternSearchTree, logStream, gp, sgp);
			patternID = newFrequentPatternSearchTree->lowPoint;

			// print frequent patterns
			printStringsInSearchTree(newFrequentPatternSearchTree, patternStream, sgp);
			printSupportSetsSparse(newSupportSets, featureStream);

			// add patterns to frequent pattern list and support sets to frontier
			mergeSearchTrees(levelSearchTrees[currentPatternSize], 
							 newFrequentPatternSearchTree, 
							 1, 
							 NULL, // &searchTreeHelper, don't need results here 
							 0, // &searchTreePosition, don't need results here
							 levelSearchTrees[currentPatternSize], 
							 0, 
							 gp);
			dumpSearchTree(gp, newFrequentPatternSearchTree);
			candidateSupportSetStack = appendSupportSets(newSupportSets, candidateSupportSetStack);
			
		}

		// dump support set of currentCandidate is done by _BFSgetNextLevelRooted()
	}

	// garbage collection: TODO
	for (size_t i = 0; i < maxPatternSize; ++i) {
		if (i != startPatternSize - 1) {
			dumpSearchTree(gp, levelSearchTrees[i]);
		}
	}
	free(levelSearchTrees);
}
