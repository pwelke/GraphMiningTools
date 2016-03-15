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

char DEBUG_INFO = 1;

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
int getFrequentVertices(struct Graph** db, int dbSize, struct Vertex* frequentVertices, FILE* keyValueStream, struct GraphPool* gp) {
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

		/* write (graph->number, pattern id) pairs to stream */
		for (v=0; v<resultPos; ++v) {
			fprintf(keyValueStream, "%i %i\n", g->number, results[v].id);
		}
	}
	if (results != NULL) {
		free(results);
	}
	return resultSize;
}


void getFrequentEdges(struct Graph** db, int dbSize, int initialResultSetSize, struct Vertex* frequentEdges, FILE* keyValueStream, struct GraphPool* gp) {
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

		/* write (graph->number, pattern id) pairs to stream, add the patterns to the bloom
		filter of the graph (i) for pruning */
		for (v=0; v<resultPos; ++v) {
			fprintf(keyValueStream, "%i %i\n", g->number, results[v].id);
			// initialAddToPruningSet(results[v].id, i);
		}
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


void DFS(struct Graph** db, struct IntSet* candidateSupport, struct Graph* candidate, size_t threshold, int maxPatternSize, struct ShallowGraph* frequentEdges, struct Vertex* processedPatterns, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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
		fprintf(stdout, "============\nNEW PATTERN:\n");
		printCanonicalString(cString, stdout);
		printIntSet(actualSupport, stdout);

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
		DFS(db, actualSupport, refinement, threshold, maxPatternSize, frequentEdges, processedPatterns, gp, sgp);
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

struct SubtreeIsoDataStoreElement {
	struct SubtreeIsoDataStore data;
	struct SubtreeIsoDataStoreElement* next;
};

struct SubtreeIsoDataStoreList {
	struct SubtreeIsoDataStoreElement* first;
	struct SubtreeIsoDataStoreElement* last;
	struct SubtreeIsoDataStoreList* next;
	size_t size;
	int patternId;
};

//void appendSubtreeIsoDataStoreElement(struct SubtreeIsoDataStoreList* l, struct SubtreeIsoDataStoreElement* e) {
//	l->size++;
//	e->next = l->first;
//	l->first = e;
//}

void appendSubtreeIsoDataStoreElement(struct SubtreeIsoDataStoreList* s, struct SubtreeIsoDataStoreElement* e) {
	if (s->last != NULL) {
		s->last->next = e;
		s->last = e;
	} else {
		s->first = s->last = e;
	}
	s->size += 1;
}

void appendSubtreeIsoDataStore(struct SubtreeIsoDataStoreList* l, struct SubtreeIsoDataStore data) {
	struct SubtreeIsoDataStoreElement* e = calloc(1, sizeof(struct SubtreeIsoDataStoreElement));
	e->data = data;
	appendSubtreeIsoDataStoreElement(l, e);
}

void printSubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* l, FILE* out) {
	fprintf(out, "%zu elements: [", l->size);
	for (struct SubtreeIsoDataStoreElement* i=l->first; i!=NULL; i=i->next) {
		fprintf(out, "%i, ", i->data.g->number);
	}
	fprintf(out, "]\n");

}

struct SubtreeIsoDataStoreList* getSubtreeIsoDataStoreList() {
	return calloc(1, sizeof(struct SubtreeIsoDataStoreList));
}

void shallowdumpSubtreeIsoDataStoreElements(struct SubtreeIsoDataStoreElement* e) {
	if (e->next != NULL) {
		shallowdumpSubtreeIsoDataStoreElements(e->next);
	}
	free(e);
}

void dumpSubtreeIsoDataStoreListCopy(struct SubtreeIsoDataStoreList* s) {
	if (s->size > 0) {
		shallowdumpSubtreeIsoDataStoreElements(s->first);
	}
	free(s);
}

void dumpSubtreeIsoDataStoreElements(struct SubtreeIsoDataStoreElement* e) {
	if (e->next != NULL) {
		dumpSubtreeIsoDataStoreElements(e->next);
	}

	dumpNewCubeFast(e->data.S, e->data.g->n, e->data.h->n);
	free(e);
}

void dumpSubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* s) {
	if (s->size > 0) {
		dumpSubtreeIsoDataStoreElements(s->first);
	}
	free(s);
}


void dumpSubtreeIsoDataStoreElementsWithPostorder(struct SubtreeIsoDataStoreElement* e, struct GraphPool* gp) {
	if (e->next != NULL) {
		dumpSubtreeIsoDataStoreElementsWithPostorder(e->next, gp);
	}

	//	dumpNewCube(e->data.S, e->data.g->n, e->data.h->n);
	dumpNewCubeFast(e->data.S, e->data.g->n, e->data.h->n);
	dumpGraph(gp, e->data.h);
	free(e->data.postorder);
	free(e);
}

void dumpSubtreeIsoDataStoreListWithPostorder(struct SubtreeIsoDataStoreList* s, struct GraphPool* gp) {
	if (s->size > 0) {
		dumpSubtreeIsoDataStoreElementsWithPostorder(s->first, gp);
	}
	free(s);
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

void iterativeDFS(struct SubtreeIsoDataStoreList* candidateSupport, size_t threshold, int maxPatternSize, struct ShallowGraph* frequentEdges, struct Vertex* processedPatterns, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

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
				dumpNewCubeFast(result.S, result.g->n, result.h->n);
			}
		}
		// if so, print and recurse
		if (refinementSupport->size >= threshold) {
			fprintf(stdout, "============\nNEW PATTERN:\n");
			struct ShallowGraph* cString = canonicalStringOfTree(refinement, sgp);
			printCanonicalString(cString, stdout);
			dumpShallowGraph(sgp, cString);
			printSubtreeIsoDataStoreList(refinementSupport, stdout);

			iterativeDFS(refinementSupport, threshold, maxPatternSize, frequentEdges, processedPatterns, gp, sgp);
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


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int mainIterativeDFS(int argc, char** argv) {

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* pointer to the current graph which is returned by the input iterator */

	/* user input handling variables */
	int threshold = 1000;
	int maxPatternSize = 20;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ht:p:u";
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
			if (sscanf(optarg, "%i", &maxPatternSize) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
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
	{
		// // refactor
		// // fopen("/dev/null", "w");
		FILE* kvStream = fopen("/dev/null", "w");
		FILE* featureStream = kvStream;
		FILE* patternStream = stdout;

		int debugInfo = 1;

		struct Vertex* frequentVertices = getVertex(vp);

		struct Graph** db = NULL;
		int nGraphs = 128;
		int tmpResultSetSize = 0;

		/* init data structures */
		nGraphs = getDB(&db);
		destroyFileIterator(); // graphs are in memory now


		if (maxPatternSize > 0) {
			/* get frequent vertices */
			tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, kvStream, gp);
			filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);

			/* output frequent vertices */
			fprintf(patternStream, "patterns size 0\n");
			printStringsInSearchTree(frequentVertices, patternStream, sgp);
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 1: %i\n", frequentVertices->d); fflush(stderr); }
		}

		if (maxPatternSize > 1) {
			/* get frequent edges: first edge id is given by number of frequent vertices */
			struct Vertex* frequentEdges = getVertex(vp);
			offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
			getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, kvStream, gp);
			filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);

			/* output frequent edges */
			fprintf(patternStream, "patterns size 1\n");
			printStringsInSearchTree(frequentEdges, patternStream, sgp);

			/* convert frequentEdges to ShallowGraph */
			struct Graph* extensionEdgesVertexStore = NULL;
			struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 2: %i\n", frequentEdges->d); fflush(stderr); }



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
					iterativeDFS(edgeSupport, threshold, maxPatternSize, extensionEdges, processedPatterns, gp, sgp);
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
		fclose(kvStream);
		dumpCube();

		for (int i=0; i<nGraphs; ++i) {
			dumpGraph(gp, db[i]);
		}
		free(db);
	}

	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int mainDFS(int argc, char** argv) {

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* pointer to the current graph which is returned by the input iterator */

	/* user input handling variables */
	int threshold = 1000;
	int maxPatternSize = 20;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ht:p:u";
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
			if (sscanf(optarg, "%i", &maxPatternSize) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			} 
			break;
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
	{
		// // refactor
		// // fopen("/dev/null", "w");
		FILE* kvStream = fopen("/dev/null", "w");
		FILE* featureStream = kvStream;
		FILE* patternStream = stdout;

		int debugInfo = 1;

		struct ShallowGraph* extensionEdges = NULL;
		struct Graph* extensionEdgesVertexStore = NULL;

		struct Vertex* frequentVertices = getVertex(vp);
		struct Vertex* frequentEdges = getVertex(vp);
		struct Graph** db = NULL;
		int nGraphs = 128;
		int tmpResultSetSize = 0;

		/* init data structures */
		nGraphs = getDB(&db);
		destroyFileIterator(); // graphs are in memory now


		if (maxPatternSize > 0) {
			/* get frequent vertices */
			tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, kvStream, gp);
			filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);

			/* output frequent vertices */
			fprintf(patternStream, "patterns size 0\n");
			printStringsInSearchTree(frequentVertices, patternStream, sgp);
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 1: %i\n", frequentVertices->d); fflush(stderr); }
		}

		if (maxPatternSize > 1) {
			/* get frequent edges: first edge id is given by number of frequent vertices */
			offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
			getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, kvStream, gp);
			filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);

			/* output frequent edges */
			fprintf(patternStream, "patterns size 1\n");
			printStringsInSearchTree(frequentEdges, patternStream, sgp);

			/* convert frequentEdges to ShallowGraph */
			extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);	
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 2: %i\n", frequentEdges->d); fflush(stderr); }

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

			fprintf(stdout, "==\n==\nPROCESSING NEXT EDGE:\n");
			struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);
			printCanonicalString(cString, stdout);
			if (!containsString(processedPatterns, cString)) {
				addToSearchTree(processedPatterns, cString, gp, sgp);
				DFS(db, fullDataBase, candidate, threshold, maxPatternSize, extensionEdges, processedPatterns, gp, sgp);
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
		fclose(kvStream);
		dumpCube();

		for (int i=0; i<nGraphs; ++i) {
			dumpGraph(gp, db[i]);
		}
		free(db);
	}

	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int mainBFS(int argc, char** argv) {

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* user input handling variables */
	int threshold = 1000;
	int maxPatternSize = 20;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ht:p:u";
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
			if (sscanf(optarg, "%i", &maxPatternSize) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			} 
			break;
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
	{
		// refactor
		// fopen("/dev/null", "w");
		FILE* kvStream = fopen("/dev/null", "w");
		FILE* featureStream = kvStream;
		FILE* patternStream = stdout;

		int debugInfo = 1;

		struct Vertex* frequentPatterns = NULL;
		struct ShallowGraph* extensionEdges = NULL;
		struct Graph* extensionEdgesVertexStore = NULL;
		int patternSize = 0;

		struct Vertex* frequentVertices = getVertex(vp);
		struct Vertex* frequentEdges = getVertex(vp);
		struct Graph** db = NULL;
		int nGraphs = 128;
		int tmpResultSetSize = 0;

		/* init data structures */
		nGraphs = getDB(&db);
		destroyFileIterator(); // graphs are in memory now


		if (maxPatternSize > 0) {
			/* get frequent vertices */
			tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, kvStream, gp);
			filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);

			/* output frequent vertices */
			fprintf(patternStream, "patterns size 0\n");
			printStringsInSearchTree(frequentVertices, patternStream, sgp);
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 1: %i\n", frequentVertices->d); fflush(stderr); }
		}

		if (maxPatternSize > 1) {
			/* get frequent edges: first edge id is given by number of frequent vertices */
			offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
			getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, kvStream, gp);
			filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);

			/* output frequent edges */
			fprintf(patternStream, "patterns size 1\n");
			printStringsInSearchTree(frequentEdges, patternStream, sgp);

			/* convert frequentEdges to ShallowGraph */
			extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);	
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 2: %i\n", frequentEdges->d); fflush(stderr); }
		}

		// BFS
		/* start with patterns containing two edges */
		for (frequentPatterns = frequentEdges, patternSize = 3; (frequentPatterns->d > 0) && (patternSize <= maxPatternSize); ++patternSize) {
			struct ShallowGraph* prefix = getShallowGraph(sgp);
			struct Vertex* candidateSet;
			struct Vertex** pointers;
			struct Graph** refinements;
			int refinementSize;

			if (debugInfo) { fprintf(stderr, "starting level %i\n", patternSize); fflush(stderr); }

			// candidateSet = generateCandidateTreeSet(frequentPatterns, extensionEdges, gp, sgp);
			candidateSet = generateCandidateAprioriTreeSet(frequentPatterns, extensionEdges, gp, sgp);

			if (debugInfo) { fprintf(stderr, "Candidates for level %i: %i\n", patternSize, candidateSet->d); fflush(stderr); }

			refinementSize = candidateSet->d;
			pointers = malloc(refinementSize * sizeof(struct Vertex*));
			refinements = malloc(refinementSize * sizeof(struct Graph*));

			makeGraphsAndPointers(candidateSet, candidateSet, refinements, pointers, 0, prefix, gp, sgp); 
			stupidPatternEvaluation(db, nGraphs, refinements, refinementSize, pointers, gp);

			/* threshold + 1 as candidateSet contains each candidate once, already */
			filterSearchTreeP(candidateSet, threshold + 1, candidateSet, featureStream, gp);

			/* output patterns of current level */
			fprintf(patternStream, "patterns size %i\n", patternSize);
			printStringsInSearchTreeWithOffset(candidateSet, -1, patternStream, sgp); 
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level %i: %i\n", patternSize, candidateSet->d); fflush(stderr); }

			/* garbage collection */
			dumpSearchTree(gp, frequentPatterns);
			dumpShallowGraph(sgp, prefix);
			free(pointers);
			for (int i=0; i<refinementSize; ++i) {
				dumpGraph(gp, refinements[i]);
			}
			free(refinements);
			frequentPatterns = candidateSet;

			/* flush the output, close the streams	 */
			fflush(featureStream);
			fflush(kvStream);
			fflush(patternStream);
		}	

		// garbage collection
		dumpShallowGraphCycle(sgp, extensionEdges);
		dumpGraph(gp, extensionEdgesVertexStore);
		dumpSearchTree(gp, frequentPatterns);
		dumpSearchTree(gp, frequentVertices);
		fclose(kvStream);
		dumpCube();

		for (int i=0; i<nGraphs; ++i) {
			dumpGraph(gp, db[i]);
		}
		free(db);
	}

	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	if (1) { fprintf(stderr, "finishing level %i\n", 0); fflush(stderr); }
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}


struct SubtreeIsoDataStoreList* shallowCopySubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* a) {
	struct SubtreeIsoDataStoreList* copy = getSubtreeIsoDataStoreList();
	copy->first = a->first;
	copy->last = a->last;
	copy->patternId = a->patternId;
	copy->size = a->size;
	return copy;
}


char isSortedSubtreeIsoDataStoreListOnPatterns(struct SubtreeIsoDataStoreList* l) {
	if (l->size < 2) {
		return 1;
	}
	for (struct SubtreeIsoDataStoreList* e=l; e->next!=NULL; e=e->next) {
		if (e->first->data.h->number >= e->next->first->data.h->number) {
			return 0;
		}
	}
	return 1;
}


/**
 *
 * input: a list of support sets allSupportSets, a list of pattern ids patternIds
 * output: a list of support sets which is a sublist of allSupportSets, such that each support set corresponds to an id in patternIds
 * (comparison for intersection takes place on allSupportSet->first->data.h->number and patternId->value
 *
 * Basically a sorted list intersection with different type lists, hence assumes sorted lists.
 *
 * Expects both inputs to be not NULL.
 * Guarantees that the output list has as many elements as parentIds, i.e. that all requested support sets were found
 */
struct SubtreeIsoDataStoreList* getSupportSetsOfPatterns(struct SubtreeIsoDataStoreList* allSupportSets, struct IntSet* patternIds) {
	assert(allSupportSets != NULL);
	assert(patternIds != NULL);
	assert(isSortedUniqueIntSet(patternIds));
	assert(isSortedSubtreeIsoDataStoreListOnPatterns(allSupportSets));

	struct SubtreeIsoDataStoreList* selectedSupportSets = NULL;

	struct SubtreeIsoDataStoreList* a = allSupportSets;
	struct IntElement* b = patternIds->first;

	size_t assert_Found = 0;

	/* Once one or the other list runs out -- we're done */
	while (a != NULL && b != NULL)
	{
		if (a->first->data.h->number == b->value) {
			struct SubtreeIsoDataStoreList* supportSet = shallowCopySubtreeIsoDataStoreList(a);
			supportSet->next = selectedSupportSets;
			selectedSupportSets = supportSet;

			a = a->next;
			b = b->next;

			++assert_Found;
		}
		else if (a->first->data.h->number < b->value) /* advance the smaller list */
			a = a->next;
		else
			b = b->next;
	}

	assert(assert_Found == patternIds->size);
	return selectedSupportSets;

}

struct SubtreeIsoDataStoreList* intersectTwoSupportSets(struct SubtreeIsoDataStoreList* l1, struct SubtreeIsoDataStoreList* l2) {
	struct SubtreeIsoDataStoreList* supportList = getSubtreeIsoDataStoreList();

	struct SubtreeIsoDataStoreElement* a = l1->first;
	struct SubtreeIsoDataStoreElement* b = l2->first;

	/* Once one or the other list runs out -- we're done */
	while (a != NULL && b != NULL)
	{
		if (a->data.g->number == b->data.g->number)
		{
			appendSubtreeIsoDataStore(supportList, a->data);
			a = a->next;
			b = b->next;
		}
		else if (a->data.h->number < b->data.g->number) /* advance the smaller list */
			a = a->next;
		else
			b = b->next;
	}
	return supportList;

}


/**
 * Return the intersection of the sets in aprioriList.
 * The function returns a new list, iff aprioriList != NULL
 */
struct SubtreeIsoDataStoreList* intersectSupportSets(struct SubtreeIsoDataStoreList* aprioriList) {
	if (aprioriList == NULL) {
		return NULL;
	}
	if (aprioriList->next == NULL) {
		return intersectTwoSupportSets(aprioriList, aprioriList); // copy
	}

	struct SubtreeIsoDataStoreList* intersection = intersectTwoSupportSets(aprioriList, aprioriList->next);
	for (struct SubtreeIsoDataStoreList* support=aprioriList->next->next; support!=NULL; support=support->next) {
		struct SubtreeIsoDataStoreList* tmp = intersectTwoSupportSets(intersection, support);
		dumpSubtreeIsoDataStoreListCopy(intersection);
		intersection = tmp;
	}

	return intersection;
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
 * Changes the order of the elements in parentSupportSets such that the element that matches parentIdToKeep
 * with element->data->h->number is at the head of the list.
 *
 * expects parentIdToKeep to be in parentSupportSets. (hence also expects  that parentsupportsets->size > 0)
 */
struct SubtreeIsoDataStoreList* subtreeIsoDataStoreChangeHead(struct SubtreeIsoDataStoreList* parentSupportSets, int parentIdToKeep) {
	assert(parentSupportSets != NULL);

	// don't change anything and return fast if list head is already correct
	if (parentSupportSets->first->data.h->number == parentIdToKeep) {
		return parentSupportSets;
	}
	for (struct SubtreeIsoDataStoreList* e=parentSupportSets; e->next!=NULL; e=e->next) {
		if (e->next->first->data.h->number == parentIdToKeep) {
			// remove matching element from its position in the list
			struct SubtreeIsoDataStoreList* match = e->next;
			e->next = e->next->next;
			// push matching element to head of list
			match->next = parentSupportSets;
			parentSupportSets = match;
			break; // stop searching for the list
		}
	}

	// first element is now correct
	assert(parentSupportSets->first->data.h->number == parentIdToKeep);

	return parentSupportSets;
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
	fprintf(stderr, "=====\n");
	for (struct SubtreeIsoDataStoreList* l = parentSupportSets; l!=NULL; l=l->next) {
		fprintf(stderr, "support of pattern %i: ", l->first->data.h->number);
		printSubtreeIsoDataStoreList(l, stderr);
	}

	// move desired parent support set to head of list to keep the correct subtreeIsoDataStores
	parentSupportSets = subtreeIsoDataStoreChangeHead(parentSupportSets, parentIdToKeep);

	// compute a candidate support set that is the intersection of the support sets of the apriori parents of the extension
	struct SubtreeIsoDataStoreList* candidateSupportSuperSet = intersectSupportSets(parentSupportSets);
	fprintf(stderr, "resulting intersection: ");
	printSubtreeIsoDataStoreList(candidateSupportSuperSet, stderr);

	// garbage collection
	while (parentSupportSets) {
		struct SubtreeIsoDataStoreList* tmp = parentSupportSets->next;
		free(parentSupportSets);
		parentSupportSets = tmp;
	}

	return candidateSupportSuperSet;
}

struct Graph* popGraph(struct Graph** list) {
	struct Graph* head = *list;
	if (head != NULL) {
		*list = head->next;
		head->next = NULL;
	}
	return head;
}

void extendPreviousLevelNoPruning(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		struct ShallowGraph* extensionEdges,
		size_t threshold,
		// output
		struct SubtreeIsoDataStoreList** resultCandidateSupportSuperSets,
		struct Graph** resultCandidates,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {
	assert(previousLevelSupportLists != NULL);
	assert(previousLevelSearchTree != NULL);
	assert(extensionEdges != NULL);


	*resultCandidateSupportSuperSets = NULL;
	*resultCandidates = NULL;

	struct Vertex* currentLevelCandidateSearchTree = getVertex(gp->vertexPool);

	int nAllExtensions = 0;
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
			++nAllExtensions;

			struct ShallowGraph* string = canonicalStringOfTree(extension, sgp);

			if (!containsString(currentLevelCandidateSearchTree, string)) {
				// count number of apriori survivors
				 ++nAddedToOutput;

				 addToSearchTree(currentLevelCandidateSearchTree, string, gp, sgp);
				 extension->number = currentLevelCandidateSearchTree->lowPoint;

				// get (hopefully small) superset of the support set of the extension
				struct SubtreeIsoDataStoreList* extensionSupportSuperSet = intersectTwoSupportSets(frequentPatternSupportList, frequentPatternSupportList);

				// add extension and support super set to list of candidates for next level
				extension->next = *resultCandidates;
				*resultCandidates = extension;
				extensionSupportSuperSet->next = *resultCandidateSupportSuperSets;
				*resultCandidateSupportSuperSets = extensionSupportSuperSet;

			} else {
				// dump extension that does not fulfill apriori property
				dumpShallowGraph(sgp, string);
				dumpGraph(gp, extension);
				++nDumped;
			}
		}
	}

	dumpSearchTree(gp, currentLevelCandidateSearchTree);
	fprintf(stderr, "generated extensions: %i\n filtered extensions: %i\n", nAllExtensions, nAddedToOutput);

	assert(nAddedToOutput + nDumped == nAllExtensions);
}

void extendPreviousLevelOnlyAprioriPruning(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		struct ShallowGraph* extensionEdges,
		size_t threshold,
		// output
		struct SubtreeIsoDataStoreList** resultCandidateSupportSuperSets,
		struct Graph** resultCandidates,
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

			struct IntSet* aprioriParentIdSet = aprioriCheckExtensionReturnList(extension, previousLevelSearchTree, currentLevelCandidateSearchTree, gp, sgp);

			if (aprioriParentIdSet) {
				// count number of apriori survivors
				 ++nAllExtensionsPostApriori;

				struct SubtreeIsoDataStoreList* extensionSupportSuperSet = intersectTwoSupportSets(frequentPatternSupportList, frequentPatternSupportList);
				dumpIntSet(aprioriParentIdSet);

				// add extension and support super set to list of candidates for next level
				extension->next = *resultCandidates;
				*resultCandidates = extension;
				extensionSupportSuperSet->next = *resultCandidateSupportSuperSets;
				*resultCandidateSupportSuperSets = extensionSupportSuperSet;

				++nAddedToOutput;

			} else {
				// dump extension that does not fulfill apriori property
				dumpGraph(gp, extension);
				++nDumped;
			}
		}
	}

	dumpSearchTree(gp, currentLevelCandidateSearchTree);
	fprintf(stderr, "generated extensions: %i\napriori filtered extensions: %i\n", nAllExtensionsPreApriori, nAllExtensionsPostApriori);

	assert(nAddedToOutput + nDumped == nAllExtensionsPreApriori);
}

void extendPreviousLevel(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		struct ShallowGraph* extensionEdges,
		size_t threshold,
		// output
		struct SubtreeIsoDataStoreList** resultCandidateSupportSuperSets,
		struct Graph** resultCandidates,
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

			struct IntSet* aprioriParentIdSet = aprioriCheckExtensionReturnList(extension, previousLevelSearchTree, currentLevelCandidateSearchTree, gp, sgp);

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
	fprintf(stderr, "generated extensions: %i\napriori filtered extensions: %i\nintersection filtered extensions: %i\n", nAllExtensionsPreApriori, nAllExtensionsPostApriori, nAllExtensionsPostIntersectionFilter);

	assert(nAddedToOutput + nDumped == nAllExtensionsPreApriori);
}


struct SubtreeIsoDataStoreList* iterativeBFS(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		size_t threshold,
		struct ShallowGraph* frequentEdges,
		// output
		struct Vertex** currentLevelSearchTree,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp) {
	assert(previousLevelSupportLists != NULL);
	assert(previousLevelSearchTree != NULL);
	assert(frequentEdges != NULL);

	struct SubtreeIsoDataStoreList* currentLevelCandidateSupportSets;
	struct Graph* currentLevelCandidates;

	extendPreviousLevel(previousLevelSupportLists, previousLevelSearchTree, frequentEdges, threshold,
			&currentLevelCandidateSupportSets, &currentLevelCandidates,
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
				dumpNewCubeFast(result.S, result.g->n, result.h->n);
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
	fprintf(stderr, "frequent extensions: %i\n", nAllFrequentExtensions);

	return actualSupportLists;
}


/**
 * Assumes that each support set in the list supportSets corresponds to the same pattern but to different base graphs.
 * To correctly free the cubes (iterative subtree iso data structures), we need h->n. Hence, we need to dump h after
 * we dumped the support sets.
 */
void dumpSubtreeIsoDataStoreListWithH(struct SubtreeIsoDataStoreList* supportSets, struct GraphPool* gp) {
	struct Graph* h = supportSets->first->data.h;
	dumpSubtreeIsoDataStoreList(supportSets);
	dumpGraph(gp, h);
}


struct SubtreeIsoDataStoreList* initIterativeBFS(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int edgeId) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = postoderDB[i];
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheckForGraph(base, h);
		data.h->number = edgeId;
		//		printNewCubeCondensed(data.S, data.g->n, data.h->n);
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


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int mainIterativeBFS(int argc, char** argv) {

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* pointer to the current graph which is returned by the input iterator */

	/* user input handling variables */
	int threshold = 1000;
	unsigned int maxPatternSize = 20;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ht:p:u";
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
	{
		// // refactor
		// // fopen("/dev/null", "w");
		FILE* kvStream = fopen("/dev/null", "w");
		FILE* featureStream = kvStream;
		FILE* patternStream = stdout;

		int debugInfo = 1;



		struct Graph** db = NULL;
		int nGraphs = 128;
		int tmpResultSetSize = 0;

		/* init data structures */
		nGraphs = getDB(&db);
		int** postorders = getPostorders(db, nGraphs);

		destroyFileIterator(); // graphs are in memory now

		struct Vertex* frequentVertices = getVertex(vp);
		if (maxPatternSize > 0) {
			/* get frequent vertices */
			tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, kvStream, gp);
			filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);

			/* output frequent vertices */
			fprintf(patternStream, "patterns size 1\n");
			printStringsInSearchTree(frequentVertices, patternStream, sgp);
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 1: %i\n", frequentVertices->d); fflush(stderr); }
		}

		struct Vertex* frequentEdges = getVertex(vp);
		if (maxPatternSize > 1) {
			/* get frequent edges: first edge id is given by number of frequent vertices */
			offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
			getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, kvStream, gp);
			filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);

			// garbage collection
			dumpSearchTree(gp, frequentVertices);

			/* output frequent edges */
			fprintf(patternStream, "patterns size 2\n");
			printStringsInSearchTree(frequentEdges, patternStream, sgp);

		} else {
			dumpSearchTree(gp, frequentVertices);
		}

		if (maxPatternSize > 2) {

			// init levelwise search data structures for patterns with two vertices

			/* convert frequentEdges to ShallowGraph */
			struct Graph* extensionEdgesVertexStore = NULL;
			struct ShallowGraph* extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
			if (debugInfo) { fprintf(stderr, "Frequent patterns in level 2: %i\n", frequentEdges->d); fflush(stderr); }
			dumpSearchTree(gp, frequentEdges);



			// data structures for iterative levelwise search
			struct SubtreeIsoDataStoreList* previousLevelSupportSets = NULL;
			struct SubtreeIsoDataStoreList* previousLevelSupportSetsTail = NULL;
			// recreate search tree for level 2, but with (possibly) different ids.
			struct Vertex* previousLevelSearchTree = getVertex(gp->vertexPool);

			// TODO this whole mess can be avoided, if edgeSearchTree2ShallowGraph() also returns a list of unique edges,
			// or if we get them from somewhere else. then we can avoid building another search tree and conversion etc.
			for (struct VertexList* e=extensionEdges->edges; e!=NULL; e=e->next) {

				struct Graph* candidate = createGraph(2, gp);
				addEdgeBetweenVertices(0, 1, NULL, candidate, gp);
				candidate->vertices[0]->label = e->startPoint->label;
				candidate->vertices[1]->label = e->endPoint->label;
				candidate->vertices[0]->neighborhood->label = e->label;
				candidate->vertices[1]->neighborhood->label = e->label;

				struct ShallowGraph* cString = canonicalStringOfTree(candidate, sgp);

				if (!containsString(previousLevelSearchTree, cString)) {

					addToSearchTree(previousLevelSearchTree, cString, gp, sgp);
					int id = previousLevelSearchTree->lowPoint;

					struct SubtreeIsoDataStoreList* edgeSupport = initIterativeBFS(db, postorders, nGraphs, candidate, id);

					if (previousLevelSupportSetsTail != NULL) {
						previousLevelSupportSetsTail->next = edgeSupport;
						previousLevelSupportSetsTail = edgeSupport;
					} else {
						previousLevelSupportSets = edgeSupport;
						previousLevelSupportSetsTail = edgeSupport;
					}

				} else {
					dumpShallowGraph(sgp, cString);
					dumpGraph(gp, candidate);
				}
			}

			// intermediate garbage collection and cleanup
			previousLevelSupportSetsTail = NULL;


			// levelwise search for patterns with more than two vertices
			struct Vertex* currentLevelSearchTree = NULL;
			struct SubtreeIsoDataStoreList* currentLevelSupportSets = NULL;

			for (size_t p=3; (p<=maxPatternSize) && (previousLevelSearchTree->number>0); ++p) {
				currentLevelSearchTree = getVertex(gp->vertexPool);
				offsetSearchTreeIds(currentLevelSearchTree, previousLevelSearchTree->lowPoint);

				currentLevelSupportSets = iterativeBFS(previousLevelSupportSets, previousLevelSearchTree, threshold, extensionEdges, &currentLevelSearchTree, gp, sgp);

				// printing TODO
				fprintf(stdout, "patterns size %zu\n", p);
				printStringsInSearchTree(currentLevelSearchTree, stdout, sgp);

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

			dumpShallowGraphCycle(sgp, extensionEdges);
			dumpGraph(gp, extensionEdgesVertexStore);
			dumpSearchTree(gp, previousLevelSearchTree);

			while (previousLevelSupportSets) {
				struct SubtreeIsoDataStoreList* tmp = previousLevelSupportSets->next;
				// we also dump the pattern graphs completely, which we can't do in a DFS mining approach.
				dumpSubtreeIsoDataStoreListWithH(previousLevelSupportSets, gp);
				previousLevelSupportSets = tmp;
			}

		} else {
			dumpSearchTree(gp, frequentEdges);
		}

		fclose(kvStream);

		for (int i=0; i<nGraphs; ++i) {
			dumpGraph(gp, db[i]);
			free(postorders[i]);
		}
		free(db);
		free(postorders);
	}

	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
//		return mainIterativeDFS(argc, argv);
	//	return mainDFS(argc, argv);
	//	return mainBFS(argc, argv);
	return mainIterativeBFS(argc, argv);
}
