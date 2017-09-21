#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "graph.h"
#include "searchTree.h"
#include "loading.h"
#include "outerplanar.h"
#include "cs_Parsing.h"
#include "cs_Tree.h"
#include "treeEnumeration.h"
#include "sampleSubtrees.h"
#include "levelwiseTreePatternMining.h"
#include "bitSet.h"
#include "graphPrinting.h"
#include "lwm_embeddingOperators.h"
#include "subtreeIsoUtils.h"
#include "localEasySubtreeIsomorphism.h"

#include "levelwiseGraphMining.h"

/**
 * to avoid memory leaks, vertex and edge labels need to be explicitly copied before underlying graph or shallow graphs are dumped
 */
static void hardCopyGraphLabels(struct Graph* g) {
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[vi];
		if (v->isStringMaster == 0) {
			v->label = copyString(v->label);
			v->isStringMaster = 1;
		}
		for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
			if (e->isStringMaster == 0) {
				e->label = copyString(e->label);
				e->isStringMaster = 1;
			}
		}
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


int getSpanningTreeSamplesOfDB(struct Graph*** db, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		struct Graph* h = NULL;
		// sample k spanning trees, canonicalize them and add them in a search tree (to avoid duplicates, i.e. isomorphic spanning trees)
		struct ShallowGraph* sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingWilson, g, k, k, gp, sgp);
		for (struct ShallowGraph* tree=sample; tree!=NULL; tree=tree->next) {
			if (tree->m != 0) {
				struct Graph* tmp = shallowGraphToGraph(tree, gp);
				tmp->next = h;
				h = tmp;
			}
		}
		dumpShallowGraphCycle(sgp, sample);

		h = mergeGraphs(h, gp);
		h->number = g->number;
		h->activity = g->activity;

		// avoid memory leaks, access to freed memory, and free unused stuff
		hardCopyGraphLabels(h);
		dumpGraph(gp, g);

		/* make space for storing graphs in array */
		if (dbSize <= i) {
			dbSize = dbSize == 0 ? 128 : dbSize * 2;
			*db = realloc(*db, dbSize * sizeof (struct Graph*));
		}
		/* store graph */
		(*db)[i] = h;
		++i;
	}

//	fprintf(stderr, "getSpanningTreeSamplesOfDB\n");
//	for (int x=0; x<i; ++x) {
//		printGraphAidsFormat((*db)[x], stderr);
//	}

	return i;
}


int getAllSpanningTreesOfDB(struct Graph*** db, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		struct Graph* h = NULL;
		// sample k spanning trees, canonicalize them and add them in a search tree (to avoid duplicates, i.e. isomorphic spanning trees)
		struct ShallowGraph* sample = runForEachConnectedComponent(&xlistSpanningTrees, g, k, k, gp, sgp);
		for (struct ShallowGraph* tree=sample; tree!=NULL; tree=tree->next) {
			if (tree->m != 0) {
				struct Graph* tmp = shallowGraphToGraph(tree, gp);
				tmp->next = h;
				h = tmp;
			}
		}
		dumpShallowGraphCycle(sgp, sample);

		h = mergeGraphs(h, gp);
		h->number = g->number;
		h->activity = g->activity;

		// avoid memory leaks, access to freed memory, and free unused stuff
		hardCopyGraphLabels(h);
		dumpGraph(gp, g);

		/* make space for storing graphs in array */
		if (dbSize <= i) {
			dbSize = dbSize == 0 ? 128 : dbSize * 2;
			*db = realloc(*db, dbSize * sizeof (struct Graph*));
		}
		/* store graph */
		(*db)[i] = h;
		++i;
	}

//	fprintf(stderr, "getSpanningTreeSamplesOfDB\n");
//	for (int x=0; x<i; ++x) {
//		printGraphAidsFormat((*db)[x], stderr);
//	}

	return i;
}


int getNonisomorphicSpanningTreeSamplesOfDB(struct Graph*** db, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		struct Vertex* searchTree = getVertex(gp->vertexPool);

		// sample k spanning trees, canonicalize them and add them in a search tree (to avoid duplicates, i.e. isomorphic spanning trees)
		struct ShallowGraph* sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingWilson, g, k, k, gp, sgp);
		for (struct ShallowGraph* tree=sample; tree!=NULL; tree=tree->next) {
			if (tree->m != 0) {
				struct Graph* tmp = shallowGraphToGraph(tree, gp);
				struct ShallowGraph* cString = canonicalStringOfTree(tmp, sgp);
				addToSearchTree(searchTree, cString, gp, sgp);
				/* garbage collection */
				dumpGraph(gp, tmp);
			}
		}

		// create a forest h of disjoint copies of the spanning trees
		struct Graph* h = NULL;
		struct ShallowGraph* strings = listStringsInSearchTree(searchTree, sgp);
		for (struct ShallowGraph* string=strings; string!=NULL; string=string->next) {
			struct Graph* tmp;
			tmp = treeCanonicalString2Graph(string, gp);
			tmp->next = h;
			h = tmp;
		}
		h = mergeGraphs(h, gp);
		h->number = g->number;
		h->activity = g->activity;

		// avoid memory leaks, access to freed memory, and free unused stuff
		hardCopyGraphLabels(h);
		dumpShallowGraphCycle(sgp, sample);
		dumpShallowGraphCycle(sgp, strings);
		dumpSearchTree(gp, searchTree);
		dumpGraph(gp, g);

		/* make space for storing graphs in array */
		if (dbSize <= i) {
			dbSize = dbSize == 0 ? 128 : dbSize * 2;
			*db = realloc(*db, dbSize * sizeof (struct Graph*));
		}
		/* store graph */
		(*db)[i] = h;
		++i;
	}

//	fprintf(stderr, "getNonisomorphicSpanningTreeSamplesOfDB\n");
//	for (int x=0; x<i; ++x) {
//		printGraphAidsFormat((*db)[x], stderr);
//	}

	return i;
}



int getDBfromCanonicalStrings(struct Graph*** db, FILE* stream, int bufferSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int dbSize = 0;
	int i = 0;
	int graphId = -1;
	int graphCount = -1;
	char buffer[bufferSize];

	while (fscanf(stream, "%i\t%i\t", &graphCount, &graphId) == 2) {
		struct ShallowGraph* g = parseCString(stream, buffer, sgp);
		/* make space for storing graphs in array */
		if (dbSize <= i) {
			dbSize = dbSize == 0 ? 128 : dbSize * 2;
			*db = realloc(*db, dbSize * sizeof (struct Graph*));
		}
		/* store graph */
		(*db)[i] = treeCanonicalString2Graph(g, gp);
		(*db)[i]->number = graphId;
		(*db)[i]->activity = graphCount;
		hardCopyGraphLabels((*db)[i]);
		dumpShallowGraph(sgp, g);
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
		int neededResultSize = g->n;
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


void iterativeDFS(struct SubtreeIsoDataStoreList* candidateSupport,
		size_t threshold,
		int maxPatternSize,
		struct ShallowGraph* frequentEdges,
		struct Vertex* processedPatterns,
	    // embedding operator function pointer,
	    struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
	    double importance,
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

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
			struct SubtreeIsoDataStore result = embeddingOperator(i->data, refinement, importance, gp, sgp);

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

			iterativeDFS(refinementSupport, threshold, maxPatternSize, frequentEdges, processedPatterns, embeddingOperator, importance, featureStream, patternStream, logStream, gp, sgp);
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
void iterativeDFSMain(size_t maxPatternSize,
		              size_t threshold,
					  // embedding operator function pointer,
					 struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					 double importance,
					 FILE* featureStream,
					 FILE* patternStream,
					 FILE* logStream,
					 struct GraphPool* gp,
					 struct ShallowGraphPool* sgp) {

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
				iterativeDFS(edgeSupport, threshold, maxPatternSize, extensionEdges, processedPatterns, embeddingOperator, importance, featureStream, patternStream, logStream, gp, sgp);
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
//	dumpCube();

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
//		struct Graph* listOfExtensions = extendPatternByLargerEdgesTMP(frequentPattern, extensionEdges, gp);

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

//
//struct SubtreeIsoDataStoreList* initIterativeBFSForEdges(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId) {
//	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
//	for (size_t i=0; i<nGraphs; ++i) {
//		struct SubtreeIsoDataStore base = {0};
//		base.g = db[i];
//		base.postorder = postoderDB[i];
//		struct SubtreeIsoDataStore data = initIterativeSubtreeCheckForEdge(base, h);
//		data.h->number = patternId;
//		appendSubtreeIsoDataStore(actualSupport, data);
//	}
//	return actualSupport;
//}

struct SubtreeIsoDataStoreList* initIterativeBFSForVertices(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	h->number = patternId;
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = postoderDB[i];
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheckForSingleton(base, h);
		if (data.foundIso) {
			appendSubtreeIsoDataStore(actualSupport, data);
		}
	}
	return actualSupport;
}

/**
 * For a graph g and a singleton graph h (consisting of a single vertex)
 * check, whether h is subgraph isomorphic to g. Aka. check if the vertex label of h
 * occurs among the vertex labels of g
 */
char singletonSubgraphCheck(struct Graph* g, struct Graph* h) {
	char* vertexLabel = h->vertices[0]->label;
	for (int v=0; v<g->n; ++v) {
		if (labelCmp(g->vertices[v]->label, vertexLabel) == 0) {
			return 1;
		}
	}
	return 0;
}

struct SubtreeIsoDataStoreList* initLocalEasyForVertices(struct SpanningtreeTree* spanningTreesDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	h->number = patternId;
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore data = {0};
		data.g = spanningTreesDB[i].g;
		data.h = h;
		data.postorder = (int*)&(spanningTreesDB[i]); // we store a spanningtreetree pointer instead of a int
		data.foundIso = singletonSubgraphCheck(data.g, data.h);
		if (data.foundIso) {
			appendSubtreeIsoDataStore(actualSupport, data);
		}
	}
	return actualSupport;
}


int** getPostorders(struct Graph** db, int nGraphs) {
	int** postorderDB = malloc(nGraphs * sizeof(int*));
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
 * create data structures for levelwise mining for subtree and iterative subtree embedding operator
 * for all frequent vertices in the db.
 * the ids of the frequent vertices in the search tree might be altered to ensure a sorted list of support sets.
 * To avoid leaks, the initial frequentVertices search tree must not be dumped until the end of all times.
 */
struct SubtreeIsoDataStoreList* initLevelwiseMiningForForestDB(struct Graph** db, int** postorders, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
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


/**
 * create data structures for levelwise mining for subtree and iterative subtree embedding operator
 * for all frequent vertices in the db.
 * the ids of the frequent vertices in the search tree might be altered to ensure a sorted list of support sets.
 * To avoid leaks, the initial frequentVertices search tree must not be dumped until the end of all times.
 */
struct SubtreeIsoDataStoreList* initLevelwiseMiningForLocalEasyDB(struct SpanningtreeTree* sptTrees, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	// init levelwise search data structures for patterns with one vertex

	// data structures for iterative levelwise search
	struct SubtreeIsoDataStoreList* vertexSupportSets = NULL;
	struct SubtreeIsoDataStoreList* vertexSupportSetsTail = NULL;

	int id = 1;
	for (struct VertexList* e=frequentVertices->neighborhood; e!=NULL; e=e->next) {

		struct Graph* candidate = createGraph(1, gp);
		candidate->vertices[0]->label = e->label;
		e->endPoint->lowPoint = id;

		struct SubtreeIsoDataStoreList* vertexSupport = initLocalEasyForVertices(sptTrees, nGraphs, candidate, id);

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


void madness(struct SubtreeIsoDataStoreList* previousLevelSupportSets, struct Vertex* previousLevelSearchTree,
		     struct ShallowGraph* extensionEdges, size_t maxPatternSize, size_t threshold,
			 struct SubtreeIsoDataStoreList** currentLevelSupportSets, struct Vertex** currentLevelSearchTree,
			 FILE* featureStream, FILE* patternStream, FILE* logStream,
			 struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	fprintf(logStream, "the madness begins!\n");

	*currentLevelSearchTree = NULL;
	*currentLevelSupportSets = NULL;
	for (size_t p=1; (p<=maxPatternSize) && (previousLevelSearchTree->number>0); ++p) {
		*currentLevelSearchTree = getVertex(gp->vertexPool);
		offsetSearchTreeIds(*currentLevelSearchTree, previousLevelSearchTree->lowPoint);
		struct Graph* currentLevelCandidates = NULL;

		extendPreviousLevel(previousLevelSupportSets, previousLevelSearchTree, extensionEdges, threshold,
				currentLevelSupportSets, &currentLevelCandidates, logStream,
				gp, sgp);

		// add frequent extensions to current level search tree output, set their numbers correctly
		// dump those candidates that are not frequent
		int nAllFrequentExtensions = 0;

		struct Graph* candidate;
		struct SubtreeIsoDataStoreList* candidateSupportSet;
		for (candidate=currentLevelCandidates, candidateSupportSet=*currentLevelSupportSets; candidate!=NULL; candidate=candidate->next, candidateSupportSet=candidateSupportSet->next) {
			++nAllFrequentExtensions;
			struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);
			addToSearchTree(*currentLevelSearchTree, cString, gp, sgp);
			candidate->number = (*currentLevelSearchTree)->lowPoint;

			// set candidate to be pattern in each of the support records
			for (struct SubtreeIsoDataStoreElement* e=candidateSupportSet->first; e!=NULL;e=e->next) {
				e->data.h = candidate;
				e->data.S = NULL;
			}
		}
		fprintf(logStream, "mad patterns: %i\n", nAllFrequentExtensions);

		printStringsInSearchTree(*currentLevelSearchTree, patternStream, sgp);
		printSubtreeIsoDataStoreListsSparse(*currentLevelSupportSets, featureStream);

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
		previousLevelSearchTree = *currentLevelSearchTree;
		previousLevelSupportSets = *currentLevelSupportSets;
	}
}

struct IterativeBfsForForestsDataStructures {
	struct Graph** db;
	int** postorders;
	struct Vertex* initialFrequentPatterns;
	struct ShallowGraph* extensionEdges;
	struct Graph* extensionEdgesVertexStore;
	int nGraphs;
};

size_t initIterativeBFSForForestDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

	(void)importance; // unused

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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = initLevelwiseMiningForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// store pointers for final garbage collection
	struct IterativeBfsForForestsDataStructures* x = malloc(sizeof(struct IterativeBfsForForestsDataStructures));
	x->db = db;
	x->postorders = postorders;
	x->nGraphs = nGraphs;
	x->extensionEdges = extensionEdges;
	x->extensionEdgesVertexStore = extensionEdgesVertexStore;
	x->initialFrequentPatterns = frequentVertices;

	// 'return'
	*initialFrequentPatterns = frequentVertices;
	*supportSets = frequentVerticesSupportSets;
	*extensionEdgeList = extensionEdges;
	*dataStructures = x;
	return 1; // returned patterns have 1 vertex
}


/**
 * Very inefficient variant of exact frequent subtree mining by representing a
 * graph by the set of all of its spanning trees
 */
size_t initIterativeBFSForAllGlobalTreeEnumerationExactMining(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

	struct Graph** db = NULL;
	int nGraphs = getAllSpanningTreesOfDB(&db, (int)importance, gp, sgp);
	int** postorders = getPostorders(db, nGraphs);

	struct Vertex* frequentVertices;
	struct Vertex* frequentEdges;
	getFrequentVerticesAndEdges(db, nGraphs, threshold, &frequentVertices, &frequentEdges, logStream, gp);

	/* convert frequentEdges to ShallowGraph of extension edges */
	struct Graph* extensionEdgesVertexStore = NULL;
	struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
	dumpSearchTree(gp, frequentEdges);

	// levelwise search for patterns with one vertex:
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = initLevelwiseMiningForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// store pointers for final garbage collection
	struct IterativeBfsForForestsDataStructures* x = malloc(sizeof(struct IterativeBfsForForestsDataStructures));
	x->db = db;
	x->postorders = postorders;
	x->nGraphs = nGraphs;
	x->extensionEdges = extensionEdges;
	x->extensionEdgesVertexStore = extensionEdgesVertexStore;
	x->initialFrequentPatterns = frequentVertices;

	// 'return'
	*initialFrequentPatterns = frequentVertices;
	*supportSets = frequentVerticesSupportSets;
	*extensionEdgeList = extensionEdges;
	*dataStructures = x;
	return 1; // returned patterns have 1 vertex
}


size_t initIterativeBFSForSampledProbabilisticTree(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

	struct Graph** db = NULL;
	int nGraphs = getNonisomorphicSpanningTreeSamplesOfDB(&db, (int)importance, gp, sgp);
	int** postorders = getPostorders(db, nGraphs);

	struct Vertex* frequentVertices;
	struct Vertex* frequentEdges;
	getFrequentVerticesAndEdges(db, nGraphs, threshold, &frequentVertices, &frequentEdges, logStream, gp);

	/* convert frequentEdges to ShallowGraph of extension edges */
	struct Graph* extensionEdgesVertexStore = NULL;
	struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
	dumpSearchTree(gp, frequentEdges);

	// levelwise search for patterns with one vertex:
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = initLevelwiseMiningForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// store pointers for final garbage collection
	struct IterativeBfsForForestsDataStructures* x = malloc(sizeof(struct IterativeBfsForForestsDataStructures));
	x->db = db;
	x->postorders = postorders;
	x->nGraphs = nGraphs;
	x->extensionEdges = extensionEdges;
	x->extensionEdgesVertexStore = extensionEdgesVertexStore;
	x->initialFrequentPatterns = frequentVertices;

	// 'return'
	*initialFrequentPatterns = frequentVertices;
	*supportSets = frequentVerticesSupportSets;
	*extensionEdgeList = extensionEdges;
	*dataStructures = x;
	return 1; // returned patterns have 1 vertex
}


size_t initBFSBase(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

	(void)importance; // unused

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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = initLevelwiseMiningForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// store pointers for final garbage collection
	struct IterativeBfsForForestsDataStructures* x = malloc(sizeof(struct IterativeBfsForForestsDataStructures));
	x->db = db;
	x->postorders = postorders;
	x->nGraphs = nGraphs;
	x->extensionEdges = extensionEdges;
	x->extensionEdgesVertexStore = extensionEdgesVertexStore;
	x->initialFrequentPatterns = frequentVertices;

	// 'return'
	*initialFrequentPatterns = frequentVertices;
	*supportSets = frequentVerticesSupportSets;
	*extensionEdgeList = extensionEdges;
	*dataStructures = x;
	return 1; // returned patterns have 1 vertex
}

struct IterativeBfsForLocalEasyDataStructures {
	struct SpanningtreeTree* sptTrees;
	struct Vertex* initialFrequentPatterns;
	struct ShallowGraph* extensionEdges;
	struct Graph* extensionEdgesVertexStore;
	int nGraphs;
};

size_t initIterativeBFSForExactLocalEasy(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

	(void)importance; // unused

	struct Graph** db = NULL;
	size_t nGraphs = getDB(&db);
	struct SpanningtreeTree* sptTrees = malloc(nGraphs * sizeof(struct SpanningtreeTree));
	for (size_t i=0; i<nGraphs; ++i) {
		struct BlockTree blockTree = getBlockTreeT(db[i], sgp);
		sptTrees[i] = getFullSpanningtreeTree(blockTree, gp, sgp);
	}

	struct Vertex* frequentVertices;
	struct Vertex* frequentEdges;
	getFrequentVerticesAndEdges(db, nGraphs, threshold, &frequentVertices, &frequentEdges, logStream, gp);

	/* convert frequentEdges to ShallowGraph of extension edges */
	struct Graph* extensionEdgesVertexStore = NULL;
	struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
	dumpSearchTree(gp, frequentEdges);
	free(db);

	// levelwise search for patterns with one vertex:
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = initLevelwiseMiningForLocalEasyDB(sptTrees, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// store pointers for final garbage collection
	struct IterativeBfsForLocalEasyDataStructures* x = malloc(sizeof(struct IterativeBfsForLocalEasyDataStructures));
	x->nGraphs = nGraphs;
	x->extensionEdges = extensionEdges;
	x->extensionEdgesVertexStore = extensionEdgesVertexStore;
	x->initialFrequentPatterns = frequentVertices;
	x->sptTrees = sptTrees;

	// 'return'
	*initialFrequentPatterns = frequentVertices;
	*supportSets = frequentVerticesSupportSets;
	*extensionEdgeList = extensionEdges;
	*dataStructures = x;
	return 1; // returned patterns have 1 vertex
}


size_t initIterativeBFSForSampledLocalEasy(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {

	struct Graph** db = NULL;
	size_t nGraphs = getDB(&db);
	struct SpanningtreeTree* sptTrees = malloc(nGraphs * sizeof(struct SpanningtreeTree));
	for (size_t i=0; i<nGraphs; ++i) {
		struct BlockTree blockTree = getBlockTreeT(db[i], sgp);
		sptTrees[i] = getSampledSpanningtreeTree(blockTree, (int)importance, gp, sgp);
	}

//	fprintf(stderr, "initIterativeBFSForSampledLocalEasy\n");
//	for (size_t i=0; i<nGraphs; ++i) {
//		printSptTree(sptTrees[i]);
//	}

	struct Vertex* frequentVertices;
	struct Vertex* frequentEdges;
	getFrequentVerticesAndEdges(db, nGraphs, threshold, &frequentVertices, &frequentEdges, logStream, gp);

	/* convert frequentEdges to ShallowGraph of extension edges */
	struct Graph* extensionEdgesVertexStore = NULL;
	struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
	dumpSearchTree(gp, frequentEdges);
	free(db);

	// levelwise search for patterns with one vertex:
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = initLevelwiseMiningForLocalEasyDB(sptTrees, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// store pointers for final garbage collection
	struct IterativeBfsForLocalEasyDataStructures* x = malloc(sizeof(struct IterativeBfsForLocalEasyDataStructures));
	x->nGraphs = nGraphs;
	x->extensionEdges = extensionEdges;
	x->extensionEdgesVertexStore = extensionEdgesVertexStore;
	x->initialFrequentPatterns = frequentVertices;
	x->sptTrees = sptTrees;

	// 'return'
	*initialFrequentPatterns = frequentVertices;
	*supportSets = frequentVerticesSupportSets;
	*extensionEdgeList = extensionEdges;
	*dataStructures = x;
	return 1; // returned patterns have 1 vertex
}

void iterativeBFSMain(size_t startPatternSize,
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

		currentLevelSupportSets = iterativeBFSOneLevel(previousLevelSupportSets, previousLevelSearchTree, threshold, extensionEdges, embeddingOperator, importance, &currentLevelSearchTree, logStream, gp, sgp);

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

void garbageCollectIterativeBFSForForestDB(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct IterativeBfsForForestsDataStructures* dataStructures = (struct IterativeBfsForForestsDataStructures*)y;

	dumpSearchTree(gp, dataStructures->initialFrequentPatterns);
	dumpShallowGraphCycle(sgp, dataStructures->extensionEdges);
	dumpGraph(gp, dataStructures->extensionEdgesVertexStore);

	for (int i=0; i<dataStructures->nGraphs; ++i) {
		dumpGraph(gp, dataStructures->db[i]);
		free(dataStructures->postorders[i]);
	}

	free(dataStructures->db);
	free(dataStructures->postorders);
	free(dataStructures);
}

void garbageCollectBFSBase(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct IterativeBfsForForestsDataStructures* dataStructures = (struct IterativeBfsForForestsDataStructures*)y;

	dumpSearchTree(gp, dataStructures->initialFrequentPatterns);
	dumpShallowGraphCycle(sgp, dataStructures->extensionEdges);
	dumpGraph(gp, dataStructures->extensionEdgesVertexStore);

	for (int i=0; i<dataStructures->nGraphs; ++i) {
		dumpGraph(gp, dataStructures->db[i]);
		free(dataStructures->postorders[i]);
	}

	free(dataStructures->db);
	free(dataStructures->postorders);
	free(dataStructures);
}

void garbageCollectIterativeBFSForLocalEasy(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct IterativeBfsForLocalEasyDataStructures* dataStructures = (struct IterativeBfsForLocalEasyDataStructures*)y;

	dumpSearchTree(gp, dataStructures->initialFrequentPatterns);
	dumpShallowGraphCycle(sgp, dataStructures->extensionEdges);
	dumpGraph(gp, dataStructures->extensionEdgesVertexStore);

	for (int i=0; i<dataStructures->nGraphs; ++i) {
		dumpGraph(gp, dataStructures->sptTrees[i].g);
		dumpSpanningtreeTree(dataStructures->sptTrees[i], gp);
	}

	free(dataStructures->sptTrees);
	free(dataStructures);
}
