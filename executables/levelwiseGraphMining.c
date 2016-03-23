#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <assert.h>

#include "../graph.h"
#include "../searchTree.h"
#include "../loading.h"
#include "../outerplanar.h"
#include "../cs_Parsing.h"
#include "../cs_Tree.h"
#include "../treeEnumeration.h"
#include "../levelwiseTreePatternMining.h"
#include "../subtreeIsomorphism.h"
#include "../bitSet.h"
#include "../graphPrinting.h"
#include "../intSet.h"
#include "../iterativeSubtreeIsomorphism.h"
#include "../subtreeIsoDataStoreList.h"


const char DEBUG_INFO = 1;


/**
 * Print --help message
 */
int printHelp() {
	FILE* helpFile = fopen("executables/levelwiseGraphMiningHelp.txt", "r");
	if (helpFile != NULL) {
		int c = EOF;
		while ((c = fgetc(helpFile)) != EOF) {
			fputc(c, stdout);
		}
		fclose(helpFile);
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Could not read helpfile\n");
		return EXIT_FAILURE;
	}
}


int getDB(struct Graph*** db) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		/* make space for storing graphs in array */
		if (dbSize <= i) {
			dbSize = dbSize == 0 ? 128 : dbSize * 2;
			*db = realloc(*db, dbSize * sizeof (struct Graph*));
		}
		/* store graph */	
		(*db)[i] = g;
		++i;
	}
	return i;
}


/**
Find the frequent vertices in a graph db given by an array of graphs.
The frequent vertices are stored in the search tree, the return value of this function is the size of the 
temporary data structure for merging search trees.
 */
int getFrequentVertices(struct Graph** db, int dbSize, struct Vertex* frequentVertices, struct GraphPool* gp) {
	int i = 0;
	struct compInfo* results = NULL;
	int resultSize = 0;

	/* iterate over all graphs in the database */
	for (i=0; i<dbSize; ++i) {
		struct Graph* g = db[i];

		int v;

		/* the vertices contained in g can be obtained from a single spanning tree, as all spanning trees contain
		the same vertex set. However, to omit multiplicity, we again resort to a temporary searchTree */
		struct Vertex* containedVertices = getVertex(gp->vertexPool);

		/* init temporary result storage if necessary */
		int neededResultSize = g->m;
		int resultPos = 0;
		if (neededResultSize > resultSize) {
			if (results) {
				free(results);
			}

			results = getResultVector(neededResultSize);
			resultSize = neededResultSize;
		}

		for (v=0; v<g->n; ++v) {
			/* See commented out how it would look if done by the book.
			However, this has to be fast and canonicalStringOfTree has
			too much overhead!
			    struct ShallowGraph* cString;
			    auxiliary->vertices[0]->label = patternGraph->vertices[v]->label;
			    cString = canonicalStringOfTree(auxiliary, sgp);
			    addToSearchTree(containedVertices, cString, gp, sgp); */
			struct VertexList* cString = getVertexList(gp->listPool);
			cString->label = g->vertices[v]->label;
			containedVertices->d += addStringToSearchTree(containedVertices, cString, gp);
			containedVertices->number += 1;
		}
		/* set multiplicity of patterns to 1 and add to global vertex pattern set, print to file */
		resetToUnique(containedVertices);
		mergeSearchTrees(frequentVertices, containedVertices, 1, results, &resultPos, frequentVertices, 0, gp);
		dumpSearchTree(gp, containedVertices);
	}
	if (results != NULL) {
		free(results);
	}
	return resultSize;
}


void getFrequentEdges(struct Graph** db, int dbSize, int initialResultSetSize, struct Vertex* frequentEdges, struct GraphPool* gp) {
	int i = 0;
	struct compInfo* results = NULL;
	int resultSize = 0;

	if (initialResultSetSize > 0) {
		results = getResultVector(initialResultSetSize);
		resultSize = initialResultSetSize;
	}

	/* iterate over all graphs in the database */
	for (i=0; i<dbSize; ++i) {
		struct Graph* g = db[i];
		int v;

		/* frequency of an edge increases by one if there exists a pattern for the current graph (a spanning tree) 
		that contains the edge. Thus we need to find all edges contained in any spanning tree and then add them 
		to frequentEdges once omitting multiplicity */
		struct Vertex* containedEdges = getVertex(gp->vertexPool);

		/* init temporary result storage if necessary */
		int neededResultSize = g->m;
		int resultPos = 0;
		if (neededResultSize > resultSize) {
			if (results) {
				free(results);
			}

			results = getResultVector(neededResultSize);
			resultSize = neededResultSize;
		}

		for (v=0; v<g->n; ++v) {
			struct VertexList* e;
			for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
				int w = e->endPoint->number;
				/* edges occur twice in patternGraph. just add them once to the search tree */
				if (w > v) {
					/* as for vertices, I use specialized code to generate 
					the canonical string of a single edge */
					struct VertexList* cString;
					if (strcmp(e->startPoint->label, e->endPoint->label) < 0) {
						/* cString = v e (w) */
						struct VertexList* tmp = getVertexList(gp->listPool);
						tmp->label = e->endPoint->label;

						cString = getTerminatorEdge(gp->listPool);
						tmp->next = cString;

						cString = getVertexList(gp->listPool);
						cString->label = e->label;
						cString->next = tmp;

						tmp = getInitialisatorEdge(gp->listPool);
						tmp->next = cString;

						cString = getVertexList(gp->listPool);
						cString->label = e->startPoint->label;
						cString->next = tmp;
					} else {
						/* cString = w e (v) */
						struct VertexList* tmp = getVertexList(gp->listPool);
						tmp->label = e->startPoint->label;

						cString = getTerminatorEdge(gp->listPool);
						tmp->next = cString;

						cString = getVertexList(gp->listPool);
						cString->label = e->label;
						cString->next = tmp;

						tmp = getInitialisatorEdge(gp->listPool);
						tmp->next = cString;

						cString = getVertexList(gp->listPool);
						cString->label = e->endPoint->label;
						cString->next = tmp;
					}
					/* add the string to the search tree */
					containedEdges->d += addStringToSearchTree(containedEdges, cString, gp);
					containedEdges->number += 1;
				} 
			}
		}

		/* set multiplicity of patterns to 1 and add to global edge pattern set */
		resetToUnique(containedEdges);
		mergeSearchTrees(frequentEdges, containedEdges, 1, results, &resultPos, frequentEdges, 0, gp);
		dumpSearchTree(gp, containedEdges);
	}

	if (results) { 
		free(results);
	}
}


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


void DFS(struct Graph** db, struct IntSet* candidateSupport, struct Graph* candidate, size_t threshold, int maxPatternSize, struct ShallowGraph* frequentEdges, struct Vertex* processedPatterns, FILE* featureStream, FILE* patternStream, FILE* logStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	// test if candidate is frequent
	struct IntSet* actualSupport = getIntSet();
	for (struct IntElement* i=candidateSupport->first; i!=NULL; i=i->next) {
		struct Graph* g = db[i->value];
		if (subtreeCheck3(g, candidate, gp)) {
			appendInt(actualSupport, i->value);
		}
	}
	struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);

	// if so, print results and generate refinements
	struct Graph* refinements = NULL;
	if (actualSupport->size >= threshold) {
		printCanonicalString(cString, patternStream);
		printIntSetSparse(actualSupport, candidate->number, featureStream);

		if (candidate->n < maxPatternSize) {
			// crazy ineffective
			refinements = extendPattern(candidate, frequentEdges, gp);
			refinements = basicFilter(refinements, processedPatterns, gp, sgp);
		}
	}

	// add canonical string of pattern to the set of processed patterns for filtering out candidates that were already tested.
	addToSearchTree(processedPatterns, cString, gp, sgp);

	// for each refinement recursively call DFS
	for (struct Graph* refinement=refinements; refinement!=NULL; refinement=refinement->next) {
		DFS(db, actualSupport, refinement, threshold, maxPatternSize, frequentEdges, processedPatterns, featureStream, patternStream, logStream, gp, sgp);
	}

	// garbage collection
	dumpIntSet(actualSupport);
	struct Graph* refinement = refinements;
	while (refinement!=NULL) {
		struct Graph* tmp = refinement->next;
		refinement->next = NULL;
		dumpGraph(gp, refinement);
		refinement = tmp;
	}

}


void iterativeDFS(struct SubtreeIsoDataStoreList* candidateSupport, size_t threshold, int maxPatternSize, struct ShallowGraph* frequentEdges, struct Vertex* processedPatterns, FILE* featureStream, FILE* patternStream, FILE* logStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct Graph* candidate = candidateSupport->first->data.h;

	// if so, print results and generate refinements
	struct Graph* refinements = NULL;
	if (candidate->n < maxPatternSize) {
		// crazy ineffective but not bottleneck right now.
		refinements = extendPattern(candidate, frequentEdges, gp);
		refinements = basicFilter(refinements, processedPatterns, gp, sgp); // adds all refinements, valid or not, to processedPatterns
	}

	// for each refinement recursively call DFS
	for (struct Graph* refinement=refinements; refinement!=NULL; refinement=refinement->next) {

		// test if candidate is frequent
		struct SubtreeIsoDataStoreList* refinementSupport = getSubtreeIsoDataStoreList();
		for (struct SubtreeIsoDataStoreElement* i=candidateSupport->first; i!=NULL; i=i->next) {
			struct SubtreeIsoDataStore result = iterativeSubtreeCheck(i->data, refinement, gp);

			if (result.foundIso) {
				appendSubtreeIsoDataStore(refinementSupport, result);
			} else {
				dumpNewCube(result.S, result.g->n);
			}
		}
		// if so, print and recurse
		if (refinementSupport->size >= threshold) {
			struct ShallowGraph* cString = canonicalStringOfTree(refinement, sgp);
			printCanonicalString(cString, patternStream);
			dumpShallowGraph(sgp, cString);
			printSubtreeIsoDataStoreListSparse(refinementSupport, featureStream);

			iterativeDFS(refinementSupport, threshold, maxPatternSize, frequentEdges, processedPatterns, featureStream, patternStream, logStream, gp, sgp);
		}
		// clean up
		dumpSubtreeIsoDataStoreList(refinementSupport);
	}

	// garbage collection
	struct Graph* refinement = refinements;
	while (refinement!=NULL) {
		struct Graph* tmp = refinement->next;
		refinement->next = NULL;
		dumpGraph(gp, refinement);
		refinement = tmp;
	}

}


struct SubtreeIsoDataStoreList* initIterativeDFS(struct Graph** db, size_t nGraphs, struct VertexList* e, int edgeId, struct GraphPool* gp) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = getPostorder(base.g, 0);
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheck(base, e, gp);
		data.h->number = edgeId;
		//		printNewCubeCondensed(data.S, data.g->n, data.h->n);
		appendSubtreeIsoDataStore(actualSupport, data);
	}
	return actualSupport;
}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
void iterativeDFSMain(size_t maxPatternSize, size_t threshold, FILE* featureStream, FILE* patternStream, FILE* logStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct Vertex* frequentVertices = getVertex(gp->vertexPool);

	struct Graph** db = NULL;
	int nGraphs = 128;
	int tmpResultSetSize = 0;

	/* init data structures */
	nGraphs = getDB(&db);
	destroyFileIterator(); // graphs are in memory now


	if (maxPatternSize > 0) {
		/* get frequent vertices */
		tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, gp);
		filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);

		/* output frequent vertices */
		fprintf(patternStream, "patterns size 0\n");
		printStringsInSearchTree(frequentVertices, patternStream, sgp);
		fprintf(logStream, "Frequent patterns in level 1: %i\n", frequentVertices->d); fflush(logStream);
	}

	if (maxPatternSize > 1) {
		/* get frequent edges: first edge id is given by number of frequent vertices */
		struct Vertex* frequentEdges = getVertex(gp->vertexPool);
		offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
		getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, gp);
		filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);

		/* output frequent edges */
		fprintf(patternStream, "patterns size 1\n");
		printStringsInSearchTree(frequentEdges, patternStream, sgp);

		/* convert frequentEdges to ShallowGraph */
		struct Graph* extensionEdgesVertexStore = NULL;
		struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
		fprintf(logStream, "Frequent patterns in level 2: %i\n", frequentEdges->d); fflush(logStream);

		// DFS
		struct Vertex* processedPatterns = getVertex(gp->vertexPool);

		struct Graph* candidate = createGraph(2, gp);
		addEdgeBetweenVertices(0, 1, NULL, candidate, gp);

		for (struct VertexList* e=extensionEdges->edges; e!=NULL; e=e->next) {
			candidate->vertices[0]->label = e->startPoint->label;
			candidate->vertices[1]->label = e->endPoint->label;
			candidate->vertices[0]->neighborhood->label = e->label;
			candidate->vertices[1]->neighborhood->label = e->label;

			fprintf(stdout, "==\n==\nPROCESSING NEXT EDGE:\n");
			struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);
			printCanonicalString(cString, stdout);

			if (!containsString(processedPatterns, cString)) {
				struct SubtreeIsoDataStoreList* edgeSupport = initIterativeDFS(db, nGraphs, e, -1, gp);
				addToSearchTree(processedPatterns, cString, gp, sgp);
				iterativeDFS(edgeSupport, threshold, maxPatternSize, extensionEdges, processedPatterns, featureStream, patternStream, logStream, gp, sgp);
				dumpSubtreeIsoDataStoreListWithPostorder(edgeSupport, gp);
			} else {
				dumpShallowGraph(sgp, cString);
			}
		}
		dumpGraph(gp, candidate);
		dumpShallowGraphCycle(sgp, extensionEdges);
		dumpGraph(gp, extensionEdgesVertexStore);
		dumpSearchTree(gp, frequentEdges);
		dumpSearchTree(gp, processedPatterns);
	}

	dumpSearchTree(gp, frequentVertices);
	dumpCube();

	for (int i=0; i<nGraphs; ++i) {
		dumpGraph(gp, db[i]);
	}
	free(db);
}


void DFSMain(size_t maxPatternSize, size_t threshold, FILE* featureStream, FILE* patternStream, FILE* logStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* extensionEdges = NULL;
	struct Graph* extensionEdgesVertexStore = NULL;

	struct Vertex* frequentVertices = getVertex(gp->vertexPool);
	struct Vertex* frequentEdges = getVertex(gp->vertexPool);
	struct Graph** db = NULL;
	int nGraphs = 128;
	int tmpResultSetSize = 0;

	/* init data structures */
	nGraphs = getDB(&db);
	destroyFileIterator(); // graphs are in memory now


	if (maxPatternSize > 0) {
		/* get frequent vertices */
		tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, gp);
		filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);

		/* output frequent vertices */
		printStringsInSearchTree(frequentVertices, patternStream, sgp);
		fprintf(logStream, "Frequent patterns in level 1: %i\n", frequentVertices->d); fflush(logStream);
	}

	if (maxPatternSize > 1) {
		/* get frequent edges: first edge id is given by number of frequent vertices */
		offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
		getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, gp);
		filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);

		/* output frequent edges */
		printStringsInSearchTree(frequentEdges, patternStream, sgp);

		/* convert frequentEdges to ShallowGraph */
		extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
		fprintf(logStream, "Frequent patterns in level 2: %i\n", frequentEdges->d); fflush(logStream);

	}

	// DFS
	struct Vertex* processedPatterns = getVertex(gp->vertexPool);

	struct IntSet* fullDataBase = getIntSet();
	for (int j=0; j<nGraphs; ++j) {
		appendInt(fullDataBase, j);
	}

	struct Graph* candidate = createGraph(2, gp);
	addEdgeBetweenVertices(0, 1, NULL, candidate, gp);

	for (struct VertexList* e=extensionEdges->edges; e!=NULL; e=e->next) {
		candidate->vertices[0]->label = e->startPoint->label;
		candidate->vertices[1]->label = e->endPoint->label;
		candidate->vertices[0]->neighborhood->label = e->label;
		candidate->vertices[1]->neighborhood->label = e->label;

		struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);
		printCanonicalString(cString, stdout);
		if (!containsString(processedPatterns, cString)) {
			addToSearchTree(processedPatterns, cString, gp, sgp);
			DFS(db, fullDataBase, candidate, threshold, maxPatternSize, extensionEdges, processedPatterns, featureStream, patternStream, logStream, gp, sgp);
		} else {
			dumpShallowGraph(sgp, cString);
		}
	}
	dumpGraph(gp, candidate);
	dumpIntSet(fullDataBase);

	dumpShallowGraphCycle(sgp, extensionEdges);
	dumpGraph(gp, extensionEdgesVertexStore);
	dumpSearchTree(gp, frequentEdges);
	dumpSearchTree(gp, frequentVertices);
	dumpCube();

	for (int i=0; i<nGraphs; ++i) {
		dumpGraph(gp, db[i]);
	}
	free(db);

}


void filterInfrequentCandidates(// input
		struct Graph* extensions,
		struct SubtreeIsoDataStoreList* supports,
		size_t threshold,
		// output
		struct Graph** filteredExtensions,
		struct SubtreeIsoDataStoreList** filteredSupports,
		// memory management
		struct GraphPool* gp) {

	*filteredExtensions = NULL;
	*filteredSupports = NULL;

	assert(extensions != NULL);
	assert(supports != NULL);

	// filter out those elements that have support less than threshold.
	struct Graph* extension=extensions;
	struct SubtreeIsoDataStoreList* candidateSupport=supports;
	while (extension!=NULL) {
		struct Graph* nextExt = extension->next;
		struct SubtreeIsoDataStoreList* nextSup = candidateSupport->next;
		if (candidateSupport->size >= threshold) {
			// add to output
			extension->next = *filteredExtensions;
			candidateSupport->next = *filteredSupports;
			*filteredExtensions = extension;
			*filteredSupports = candidateSupport;
		} else {
			// dump
			extension->next = NULL;
			dumpGraph(gp, extension);
			dumpSubtreeIsoDataStoreList(candidateSupport);
		}
		extension = nextExt;
		candidateSupport = nextSup;
	}
}


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

	int nAllExtensionsPreApriori = 0;
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
		struct Graph* listOfExtensions = extendPattern(frequentPattern, extensionEdges, gp);

		for (struct Graph* extension=popGraph(&listOfExtensions); extension!=NULL; extension=popGraph(&listOfExtensions)) {
			// count number of generated extensions
			++nAllExtensionsPreApriori;

			/* filter out patterns that were already enumerated as the extension of some other pattern
				and are in the search tree */
			struct ShallowGraph* string = canonicalStringOfTree(extension, sgp);
			int previousNumberOfDistinctPatterns = currentLevelCandidateSearchTree->d;
			addToSearchTree(currentLevelCandidateSearchTree, string, gp, sgp);

			struct IntSet* aprioriParentIdSet;
			if (previousNumberOfDistinctPatterns == currentLevelCandidateSearchTree->d) {
				aprioriParentIdSet = NULL;
			} else {
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
	fprintf(logStream, "generated extensions: %i\napriori filtered extensions: %i\nintersection filtered extensions: %i\n", nAllExtensionsPreApriori, nAllExtensionsPostApriori, nAllExtensionsPostIntersectionFilter);

	assert(nAddedToOutput + nDumped == nAllExtensionsPreApriori);
}


struct SubtreeIsoDataStoreList* iterativeBFSOneLevel(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		size_t threshold,
		struct ShallowGraph* frequentEdges,
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
			struct SubtreeIsoDataStore result = iterativeSubtreeCheck(e->data, candidate, gp);

			if (result.foundIso) {
				//TODO store result id somehow
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
			candidate->activity = 1;
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
			addToSearchTree(*currentLevelSearchTree, cString, gp, sgp);
			candidate->number = (*currentLevelSearchTree)->lowPoint;
		}
		candidate = tmp;
	}
	fprintf(logStream, "frequent patterns: %i\n", nAllFrequentExtensions);

	return actualSupportLists;
}


struct SubtreeIsoDataStoreList* initIterativeBFSForEdges(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = postoderDB[i];
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheckForEdge(base, h);
		data.h->number = patternId;
		appendSubtreeIsoDataStore(actualSupport, data);
	}
	return actualSupport;
}

struct SubtreeIsoDataStoreList* initIterativeBFSForVertices(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = postoderDB[i];
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheckForSingleton(base, h);
		data.h->number = patternId;
		appendSubtreeIsoDataStore(actualSupport, data);
	}
	return actualSupport;
}


int** getPostorders(struct Graph** db, int nGraphs) {
	int** postorderDB = malloc(nGraphs * sizeof(int**));
	for (int g=0; g<nGraphs; ++g) {
		postorderDB[g] = getPostorder(db[g], 0);
	}
	return postorderDB;
}


void getFrequentVerticesAndEdges(struct Graph** db, int nGraphs, size_t threshold, struct Vertex** frequentVertices, struct Vertex** frequentEdges, FILE* logStream, struct GraphPool* gp) {
	*frequentVertices = getVertex(gp->vertexPool);
	*frequentEdges = getVertex(gp->vertexPool);

	/* get frequent vertices */
	int tmpResultSetSize = getFrequentVertices(db, nGraphs, *frequentVertices, gp);
	filterSearchTree(*frequentVertices, threshold, *frequentVertices, gp);
	fprintf(logStream, "Number of frequent vertices: %i\n", (*frequentVertices)->d); fflush(logStream);

	/* get frequent edges: first edge id is given by number of frequent vertices */
	offsetSearchTreeIds(*frequentEdges, (*frequentVertices)->lowPoint);
	getFrequentEdges(db, nGraphs, tmpResultSetSize, *frequentEdges, gp);
	filterSearchTree(*frequentEdges, threshold, *frequentEdges, gp);
	fprintf(logStream, "Number of frequent edges: %i\n", (*frequentEdges)->d); fflush(logStream);

}

/**
 * create data structures for levelwise mining for all frequent vertices in the db.
 * the ids of the frequent vertices in the search tree might be altered to ensure a sorted list of support sets.
 * To avoid leaks, the initial frequentVertices search tree must not be dumped until the end of all times.
 */
struct SubtreeIsoDataStoreList* initLevelwiseMining(struct Graph** db, int** postorders, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp) {
	// init levelwise search data structures for patterns with one vertex

	// data structures for iterative levelwise search
	struct SubtreeIsoDataStoreList* vertexSupportSets = NULL;
	struct SubtreeIsoDataStoreList* vertexSupportSetsTail = NULL;

	int id = 1;
	for (struct VertexList* e=frequentVertices->neighborhood; e!=NULL; e=e->next) {

		struct Graph* candidate = createGraph(1, gp);
		candidate->vertices[0]->label = e->label;
		e->endPoint->lowPoint = id;

		struct SubtreeIsoDataStoreList* vertexSupport = initIterativeBFSForVertices(db, postorders, nGraphs, candidate, id);

		if (vertexSupportSetsTail != NULL) {
			vertexSupportSetsTail->next = vertexSupport;
			vertexSupportSetsTail = vertexSupport;
		} else {
			vertexSupportSets = vertexSupport;
			vertexSupportSetsTail = vertexSupport;
		}
		++id;
	}
	return vertexSupportSets;
}

void iterativeBFSMain(size_t maxPatternSize, size_t threshold, FILE* featureStream, FILE* patternStream, FILE* logStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	/* init data structures */
	struct Graph** db = NULL;
	int nGraphs = getDB(&db);
	int** postorders = getPostorders(db, nGraphs);

	struct Vertex* frequentVertices;
	struct Vertex* frequentEdges;
	getFrequentVerticesAndEdges(db, nGraphs, threshold, &frequentVertices, &frequentEdges, logStream, gp);

	/* convert frequentEdges to ShallowGraph of extension edges */
	struct Graph* extensionEdgesVertexStore = NULL;
	struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
	dumpSearchTree(gp, frequentEdges);

	// levelwise search for patterns with one vertex:
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = initLevelwiseMining(db, postorders, nGraphs, frequentVertices, gp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// levelwise search for patterns with more than one vertex:
	struct Vertex* previousLevelSearchTree = shallowCopySearchTree(frequentVertices, gp);
	struct SubtreeIsoDataStoreList* previousLevelSupportSets = frequentVerticesSupportSets;
	struct Vertex* currentLevelSearchTree = previousLevelSearchTree; // initialization for garbage collection in case of maxPatternSize == 1
	struct SubtreeIsoDataStoreList* currentLevelSupportSets = previousLevelSupportSets; // initialization for garbage collection in case of maxPatternSize == 1

	for (size_t p=2; (p<=maxPatternSize) && (previousLevelSearchTree->number>0); ++p) {
		currentLevelSearchTree = getVertex(gp->vertexPool);
		offsetSearchTreeIds(currentLevelSearchTree, previousLevelSearchTree->lowPoint);

		currentLevelSupportSets = iterativeBFSOneLevel(previousLevelSupportSets, previousLevelSearchTree, threshold, extensionEdges, &currentLevelSearchTree, logStream, gp, sgp);

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

	// garbage collection
	dumpSearchTree(gp, frequentVertices);
	dumpShallowGraphCycle(sgp, extensionEdges);
	dumpGraph(gp, extensionEdgesVertexStore);
	dumpSearchTree(gp, previousLevelSearchTree);

	while (previousLevelSupportSets) {
		struct SubtreeIsoDataStoreList* tmp = previousLevelSupportSets->next;
		// we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
		dumpSubtreeIsoDataStoreListWithH(previousLevelSupportSets, gp);
		previousLevelSupportSets = tmp;
	}

	for (int i=0; i<nGraphs; ++i) {
		dumpGraph(gp, db[i]);
		free(postorders[i]);
	}

	free(db);
	free(postorders);

}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int main(int argc, char** argv) {

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* pointer to the current graph which is returned by the input iterator */

	/* user input handling variables */
	int threshold = 1000;
	unsigned int maxPatternSize = 20;
	void (*miningStrategy)(size_t, size_t, FILE*, FILE*, FILE*, struct GraphPool*, struct ShallowGraphPool*) = &iterativeBFSMain;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ht:p:m:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
			break;
		case 't':
			if (sscanf(optarg, "%i", &threshold) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'p':
			if (sscanf(optarg, "%u", &maxPatternSize) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'm':
			if (strcmp(optarg, "dfs") == 0) {
				miningStrategy = &iterativeDFSMain;
				break;
			}
			if (strcmp(optarg, "bfs") == 0) {
				miningStrategy = &iterativeBFSMain;
				break;
			}
			fprintf(stderr, "Unknown mining technique: %s\n", optarg);
			return EXIT_FAILURE;
		case '?':
			return EXIT_FAILURE;
			break;
		}
	}

	/* init object pools */
	lp = createListPool(10000);
	vp = createVertexPool(10000);
	sgp = createShallowGraphPool(1000, lp);
	gp = createGraphPool(100, vp, lp);

	/* initialize the stream to read graphs from
   check if there is a filename present in the command line arguments
   if so, open the file, if not, read from stdin */
	if (optind < argc) {
		char* filename = argv[optind];
		/* if the present filename is not '-' then init a file iterator for that file name */
		if (strcmp(filename, "-") != 0) {
			createFileIterator(filename, gp);
		} else {
			createStdinIterator(gp);
		}
	} else {
		createStdinIterator(gp);
	}

	// start frequent subgraph mining

	FILE* featureStream = stdout;
	FILE* patternStream = stdout;
	FILE* logStream = stderr;

	miningStrategy(maxPatternSize, threshold, featureStream, patternStream, logStream, gp, sgp);

	destroyFileIterator(); // graphs are in memory now

//	fclose(kvStream);

	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}
