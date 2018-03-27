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
		struct ShallowGraph* sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingWilson, g, k, k, 1, gp, sgp);
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

	return i;
}


int getAllSpanningTreesOfDB(struct Graph*** db, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		struct Graph* h = NULL;
		// sample k spanning trees, canonicalize them and add them in a search tree (to avoid duplicates, i.e. isomorphic spanning trees)
		struct ShallowGraph* sample = runForEachConnectedComponent(&xlistSpanningTrees, g, k, k, 1, gp, sgp);
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

	return i;
}


int getNonisomorphicSpanningTreeSamplesOfDB(struct Graph*** db, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		struct Vertex* searchTree = getVertex(gp->vertexPool);

		// sample k spanning trees, canonicalize them and add them in a search tree (to avoid duplicates, i.e. isomorphic spanning trees)
		struct ShallowGraph* sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingWilson, g, k, k, 1, gp, sgp);
		for (struct ShallowGraph* tree=sample; tree!=NULL; tree=tree->next) {
			if (tree->m != 0) {
				struct Graph* tmp = shallowGraphToGraph(tree, gp);
				addToSearchTree(searchTree, canonicalStringOfTree(tmp, sgp), gp, sgp);
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


struct SubtreeIsoDataStoreList* getSupportSetsOfVertices(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	h->number = patternId;
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = postoderDB[i];
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheckForSingleton(base, h);
		if (data.foundIso) {
			appendSubtreeIsoDataStore(actualSupport, data);
		} else {
			dumpNewCube(data.S, data.g->n);
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
struct SubtreeIsoDataStoreList* createSingletonPatternSupportSetsForForestDB(struct Graph** db, int** postorders, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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

		struct SubtreeIsoDataStoreList* vertexSupport = getSupportSetsOfVertices(db, postorders, nGraphs, candidate, id);

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
struct SubtreeIsoDataStoreList* createSingletonPatternSupportSetsForLocalEasyDB(struct SpanningtreeTree* sptTrees, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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


struct IterativeBfsForForestsDataStructures {
	struct Graph** db;
	int** postorders;
	struct Vertex* initialFrequentPatterns;
	struct ShallowGraph* extensionEdges;
	struct Graph* extensionEdgesVertexStore;
	int nGraphs;
};

size_t initFrequentTreeMiningForForestDB(// input
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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = createSingletonPatternSupportSetsForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
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
size_t initGlobalTreeEnumerationForGraphDB(// input
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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = createSingletonPatternSupportSetsForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
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


size_t initProbabilisticTreeMiningForGraphDB(// input
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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = createSingletonPatternSupportSetsForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
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

size_t initExactLocalEasyForGraphDB(// input
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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = createSingletonPatternSupportSetsForLocalEasyDB(sptTrees, nGraphs, frequentVertices, gp, sgp);
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


/**
 * In this method, we remove duplicate sampled local spanning trees.
 */
size_t initSampledLocalEasyForGraphDB(// input
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
		sptTrees[i] = getSampledSpanningtreeTree(blockTree, (int)importance, 1, gp, sgp);
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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = createSingletonPatternSupportSetsForLocalEasyDB(sptTrees, nGraphs, frequentVertices, gp, sgp);
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


/**
 * In this method, we keep duplicate sampled local spanning trees
 */
size_t initSampledLocalEasyWithDuplicatesForGraphDB(// input
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
		sptTrees[i] = getSampledSpanningtreeTree(blockTree, (int)importance, 0, gp, sgp);
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
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = createSingletonPatternSupportSetsForLocalEasyDB(sptTrees, nGraphs, frequentVertices, gp, sgp);
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


void garbageCollectFrequentTreeMiningForForestDB(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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


void garbageCollectLocalEasyForGraphDB(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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


static struct SubtreeIsoDataStoreList* initPatternEnumerationForVertices(struct Graph** spanningTreesDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	h->number = patternId;
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore data = {0};
		data.g = spanningTreesDB[i];
		data.h = h;
		data.postorder = NULL;
		data.foundIso = 1;
		appendSubtreeIsoDataStore(actualSupport, data);
	}
	return actualSupport;
}


/**
 * create data structures for levelwise mining for subtree and iterative subtree embedding operator
 * for all frequent vertices in the db.
 * the ids of the frequent vertices in the search tree might be altered to ensure a sorted list of support sets.
 * To avoid leaks, the initial frequentVertices search tree must not be dumped until the end of all times.
 */
static struct SubtreeIsoDataStoreList* createSingletonPatternSupportSetsForPatternEnumeration(struct Graph** db, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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

		struct SubtreeIsoDataStoreList* vertexSupport = initPatternEnumerationForVertices(db, nGraphs, candidate, id);

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


size_t initPatternEnumeration(// input
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

	struct Vertex* frequentVertices;
	struct Vertex* frequentEdges;
	getFrequentVerticesAndEdges(db, nGraphs, threshold, &frequentVertices, &frequentEdges, logStream, gp);

	/* convert frequentEdges to ShallowGraph of extension edges */
	struct Graph* extensionEdgesVertexStore = NULL;
	struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
	dumpSearchTree(gp, frequentEdges);

	// levelwise search for patterns with one vertex:
	struct SubtreeIsoDataStoreList* frequentVerticesSupportSets = createSingletonPatternSupportSetsForPatternEnumeration(db, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSubtreeIsoDataStoreListsSparse(frequentVerticesSupportSets, featureStream);

	// store pointers for final garbage collection
	struct IterativeBfsForForestsDataStructures* x = malloc(sizeof(struct IterativeBfsForForestsDataStructures));
	x->db = db;
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



void garbagePatternEnumeration(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct IterativeBfsForForestsDataStructures* dataStructures = (struct IterativeBfsForForestsDataStructures*)y;

	dumpSearchTree(gp, dataStructures->initialFrequentPatterns);
	dumpShallowGraphCycle(sgp, dataStructures->extensionEdges);
	dumpGraph(gp, dataStructures->extensionEdgesVertexStore);

	for (int i=0; i<dataStructures->nGraphs; ++i) {
		dumpGraph(gp, dataStructures->db[i]);
	}

	free(dataStructures->db);
	free(dataStructures);
}
