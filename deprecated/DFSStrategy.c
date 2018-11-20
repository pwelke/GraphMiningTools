#include <stddef.h>

void iterativeDFS(struct SupportSet* candidateSupport,
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
		refinements = extendPatternOnLeaves(candidate, frequentEdges, gp);
		refinements = basicFilter(refinements, processedPatterns, gp, sgp); // adds all refinements, valid or not, to processedPatterns
	}

	// for each refinement recursively call DFS
	for (struct Graph* refinement=refinements; refinement!=NULL; refinement=refinement->next) {

		// test if candidate is frequent
		struct SupportSet* refinementSupport = getSupportSet();
		for (struct SupportSetElement* i=candidateSupport->first; i!=NULL; i=i->next) {
			struct SubtreeIsoDataStore result = embeddingOperator(i->data, refinement, importance, gp, sgp);

			if (result.foundIso) {
				appendSupportSetData(refinementSupport, result);
			} else {
				dumpNewCube(result.S, result.g->n);
			}
		}
		// if so, print and recurse
		if (refinementSupport->size >= threshold) {
			struct ShallowGraph* cString = canonicalStringOfTree(refinement, sgp);
			printCanonicalString(cString, patternStream);
			dumpShallowGraph(sgp, cString);
			printSupportSetSparse(refinementSupport, featureStream);

			iterativeDFS(refinementSupport, threshold, maxPatternSize, frequentEdges, processedPatterns, embeddingOperator, importance, featureStream, patternStream, logStream, gp, sgp);
		}
		// clean up
		dumpSupportSet(refinementSupport);
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

/**
 * Extend the 'frequent pattern' set for a few more BFS levels without actually computing the support.
 * Only do apriori and intersection filtering of the candidate patterns.
 */
void madness(struct SupportSet* previousLevelSupportSets, struct Vertex* previousLevelSearchTree,
		     struct ShallowGraph* extensionEdges, size_t maxPatternSize, size_t threshold,
			 struct SupportSet** currentLevelSupportSets, struct Vertex** currentLevelSearchTree,
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
		struct SupportSet* candidateSupportSet;
		for (candidate=currentLevelCandidates, candidateSupportSet=*currentLevelSupportSets; candidate!=NULL; candidate=candidate->next, candidateSupportSet=candidateSupportSet->next) {
			++nAllFrequentExtensions;
			struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);
			addToSearchTree(*currentLevelSearchTree, cString, gp, sgp);
			candidate->number = (*currentLevelSearchTree)->lowPoint;

			// set candidate to be pattern in each of the support records
			for (struct SupportSetElement* e=candidateSupportSet->first; e!=NULL;e=e->next) {
				e->data.h = candidate;
				e->data.S = NULL;
			}
		}
		fprintf(logStream, "mad patterns: %i\n", nAllFrequentExtensions);

		printStringsInSearchTree(*currentLevelSearchTree, patternStream, sgp);
		printSupportSetsSparse(*currentLevelSupportSets, featureStream);

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
		previousLevelSearchTree = *currentLevelSearchTree;
		previousLevelSupportSets = *currentLevelSupportSets;
	}
}


//struct SupportSet* initIterativeDFS(struct Graph** db, size_t nGraphs, struct VertexList* e, int edgeId, struct GraphPool* gp) {
//	struct SupportSet* actualSupport = getSupportSet();
//	for (size_t i=0; i<nGraphs; ++i) {
//		struct SubtreeIsoDataStore base = {0};
//		base.g = db[i];
//		base.postorder = getPostorder(base.g, 0);
//		struct SubtreeIsoDataStore data = initIterativeSubtreeCheck(base, e, gp);
//		data.h->number = edgeId;
//		//		printNewCubeCondensed(data.S, data.g->n, data.h->n);
//		appendSupportSetData(actualSupport, data);
//	}
//	return actualSupport;
//}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
//void iterativeDFSMainOld(size_t startPatternSize,
//					  size_t maxPatternSize,
//		              size_t threshold,
//					  struct Vertex* initialFrequentPatterns,
//					  struct SupportSet* supportSets,
//					  struct ShallowGraph* extensionEdges,
//					  // embedding operator function pointer,
//					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
//					  double importance,
//					  FILE* featureStream,
//					  FILE* patternStream,
//					  FILE* logStream,
//					  struct GraphPool* gp,
//					  struct ShallowGraphPool* sgp) {
//
//	struct Vertex* frequentVertices = getVertex(gp->vertexPool);
//
//	struct Graph** db = NULL;
//	int nGraphs = 128;
//	int tmpResultSetSize = 0;
//
//	/* init data structures */
//	nGraphs = getDB(&db);
//	destroyFileIterator(); // graphs are in memory now
//
//
//	if (maxPatternSize > 0) {
//		/* get frequent vertices */
//		tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, gp);
//		filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);
//
//		/* output frequent vertices */
//		fprintf(patternStream, "patterns size 0\n");
//		printStringsInSearchTree(frequentVertices, patternStream, sgp);
//		fprintf(logStream, "Frequent patterns in level 1: %i\n", frequentVertices->d); fflush(logStream);
//	}
//
//	if (maxPatternSize > 1) {
//		/* get frequent edges: first edge id is given by number of frequent vertices */
//		struct Vertex* frequentEdges = getVertex(gp->vertexPool);
//		offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
//		getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, gp);
//		filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);
//
//		/* output frequent edges */
//		fprintf(patternStream, "patterns size 1\n");
//		printStringsInSearchTree(frequentEdges, patternStream, sgp);
//
//		/* convert frequentEdges to ShallowGraph */
//		struct Graph* extensionEdgesVertexStore = NULL;
//		struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
//		fprintf(logStream, "Frequent patterns in level 2: %i\n", frequentEdges->d); fflush(logStream);
//
//		// DFS
//		struct Vertex* processedPatterns = getVertex(gp->vertexPool);
//
//		struct Graph* candidate = createGraph(2, gp);
//		addEdgeBetweenVertices(0, 1, NULL, candidate, gp);
//
//		for (struct VertexList* e=extensionEdges->edges; e!=NULL; e=e->next) {
//			candidate->vertices[0]->label = e->startPoint->label;
//			candidate->vertices[1]->label = e->endPoint->label;
//			candidate->vertices[0]->neighborhood->label = e->label;
//			candidate->vertices[1]->neighborhood->label = e->label;
//
//			fprintf(stdout, "==\n==\nPROCESSING NEXT EDGE:\n");
//			struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);
//			printCanonicalString(cString, stdout);
//
//			if (!containsString(processedPatterns, cString)) {
//				struct SupportSet* edgeSupport = initIterativeDFS(db, nGraphs, e, -1, gp);
//				addToSearchTree(processedPatterns, cString, gp, sgp);
//				iterativeDFS(edgeSupport, threshold, maxPatternSize, extensionEdges, processedPatterns, embeddingOperator, importance, featureStream, patternStream, logStream, gp, sgp);
//				dumpSupportSetWithPostorder(edgeSupport, gp);
//			} else {
//				dumpShallowGraph(sgp, cString);
//			}
//		}
//		dumpGraph(gp, candidate);
//		dumpShallowGraphCycle(sgp, extensionEdges);
//		dumpGraph(gp, extensionEdgesVertexStore);
//		dumpSearchTree(gp, frequentEdges);
//		dumpSearchTree(gp, processedPatterns);
//	}
//
//	dumpSearchTree(gp, frequentVertices);
////	dumpCube();
//
//	for (int i=0; i<nGraphs; ++i) {
//		dumpGraph(gp, db[i]);
//	}
//	free(db);
//}


//void iterativeDFSMain(size_t startPatternSize,
//					  size_t maxPatternSize,
//		              size_t threshold,
//					  struct Vertex* initialFrequentPatterns,
//					  struct SupportSet* supportSets,
//					  struct ShallowGraph* extensionEdges,
//					  // embedding operator function pointer,
//					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
//					  double importance,
//					  FILE* featureStream,
//					  FILE* patternStream,
//					  FILE* logStream,
//					  struct GraphPool* gp,
//					  struct ShallowGraphPool* sgp) {
//
//	// depth first search for patterns with more than one vertex:
//	struct Vertex* patternSearchTree = initialFrequentPatterns;
////	struct Vertex* previousLevelSearchTree = shallowCopySearchTree(initialFrequentPatterns, gp);
//	struct SupportSet* previousLevelSupportSets = supportSets;
////	struct Vertex* currentLevelSearchTree = previousLevelSearchTree; // initialization for garbage collection in case of maxPatternSize == 1
//	struct SupportSet* currentLevelSupportSets = previousLevelSupportSets; // initialization for garbage collection in case of maxPatternSize == 1
//
//	maintain a searchtree of all frequent patterns
//	maintain a searchtree of all generated infrequent patterns
//
//	for all single patterns h
//		call dfs (h, support, freq patterns, infreq patterns, etal.)
//
//
//	for (struct SupportSet* support) {
////		fprintf(logStream, "Processing patterns with %zu vertices:\n", p); fflush(logStream);
////		currentLevelSearchTree = getVertex(gp->vertexPool);
//		offsetSearchTreeIds(currentLevelSearchTree, previousLevelSearchTree->lowPoint);
//
//		currentLevelSupportSets = BFSgetNextLevel(previousLevelSupportSets, previousLevelSearchTree, threshold, extensionEdges, embeddingOperator, importance, &currentLevelSearchTree, logStream, gp, sgp);
//
//		printStringsInSearchTree(currentLevelSearchTree, patternStream, sgp);
//		printSupportSetsSparse(currentLevelSupportSets, featureStream);
//
//		// garbage collection:
//		// what is now all previousLevel... data structures will not be used at all in the next iteration
//		dumpSearchTree(gp, previousLevelSearchTree);
//		while (previousLevelSupportSets) {
//			struct SupportSet* tmp = previousLevelSupportSets->next;
//			// ...hence, we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
//			dumpSupportSetWithPattern(previousLevelSupportSets, gp);
//			previousLevelSupportSets = tmp;
//		}
//
//		// previous level = current level
//		previousLevelSearchTree = currentLevelSearchTree;
//		previousLevelSupportSets = currentLevelSupportSets;
//	}
//
////	madness(currentLevelSupportSets, currentLevelSearchTree, extensionEdges, maxPatternSize, threshold, &previousLevelSupportSets, &previousLevelSearchTree, featureStream, patternStream, logStream, gp, sgp);
//
//	// garbage collection
//	dumpSearchTree(gp, previousLevelSearchTree);
//
//	while (previousLevelSupportSets) {
//		struct SupportSet* tmp = previousLevelSupportSets->next;
//		// we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
//		dumpSupportSetWithPattern(previousLevelSupportSets, gp);
//		previousLevelSupportSets = tmp;
//	}
//
//}

//
//void filterInfrequentCandidates(// input
//		struct Graph* extensions,
//		struct SupportSet* supports,
//		size_t threshold,
//		// output
//		struct Graph** filteredExtensions,
//		struct SupportSet** filteredSupports,
//		// memory management
//		struct GraphPool* gp) {
//
//	*filteredExtensions = NULL;
//	*filteredSupports = NULL;
//
//	assert(extensions != NULL);
//	assert(supports != NULL);
//
//	// filter out those elements that have support less than threshold.
//	struct Graph* extension=extensions;
//	struct SupportSet* candidateSupport=supports;
//	while (extension!=NULL) {
//		struct Graph* nextExt = extension->next;
//		struct SupportSet* nextSup = candidateSupport->next;
//		if (candidateSupport->size >= threshold) {
//			// add to output
//			extension->next = *filteredExtensions;
//			candidateSupport->next = *filteredSupports;
//			*filteredExtensions = extension;
//			*filteredSupports = candidateSupport;
//		} else {
//			// dump
//			extension->next = NULL;
//			dumpGraph(gp, extension);
//			dumpSupportSet(candidateSupport);
//		}
//		extension = nextExt;
//		candidateSupport = nextSup;
//	}
//}
//
