#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

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


void stupidPatternEvaluation(struct Graph** db, int nGraphs, struct Graph** patterns, int nPatterns, struct Vertex** pointers, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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
	size_t size;
};

void appendSubtreeIsoDataStoreElement(struct SubtreeIsoDataStoreList* l, struct SubtreeIsoDataStoreElement* e) {
	l->size++;
	e->next = l->first;
	l->first = e;
}

void appendSubtreeIsoDataStore(struct SubtreeIsoDataStoreList* l, struct SubtreeIsoDataStore data) {
	struct SubtreeIsoDataStoreElement* e = calloc(1, sizeof(struct SubtreeIsoDataStoreElement));
	e->data = data;
	e->next = l->first;

	l->first = e;
	l->size++;
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

void dumpSubtreeIsoDataStoreElements(struct SubtreeIsoDataStoreElement* e, struct GraphPool* gp) {
	if (e->next != NULL) {
		dumpSubtreeIsoDataStoreElements(e->next, gp);
	}

	dumpNewCube(e->data.S, e->data.g->n, e->data.h->n);
//	dumpGraph(gp, e->data.h);
	free(e);
}

void dumpSubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* s, struct GraphPool* gp) {
	if (s->size > 0) {
		dumpSubtreeIsoDataStoreElements(s->first, gp);
	}
	free(s);
}


void dumpSubtreeIsoDataStoreElementsWithPostorder(struct SubtreeIsoDataStoreElement* e, struct GraphPool* gp) {
	if (e->next != NULL) {
		dumpSubtreeIsoDataStoreElementsWithPostorder(e->next, gp);
	}

	dumpNewCube(e->data.S, e->data.g->n, e->data.h->n);
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

struct SubtreeIsoDataStoreList* initIterativeDFS(struct Graph** db, size_t nGraphs, struct VertexList* e, struct GraphPool* gp) {
	struct SubtreeIsoDataStoreList* actualSupport = getSubtreeIsoDataStoreList();
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = getPostorder(base.g, 0);
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheck(base, e, gp);
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
		refinements = basicFilter(refinements, processedPatterns, gp, sgp);
	}

	// for each refinement recursively call DFS
	for (struct Graph* refinement=refinements; refinement!=NULL; refinement=refinement->next) {
		struct ShallowGraph* cString = canonicalStringOfTree(refinement, sgp);
		// add canonical string of pattern to the set of processed patterns for filtering out candidates that were already tested.
		addToSearchTree(processedPatterns, cloneShallowGraph(cString, sgp), gp, sgp);

		// test if candidate is frequent
		struct SubtreeIsoDataStoreList* refinementSupport = getSubtreeIsoDataStoreList();
		for (struct SubtreeIsoDataStoreElement* i=candidateSupport->first; i!=NULL; i=i->next) {
			struct SubtreeIsoDataStore result = iterativeSubtreeCheck(i->data, refinement, gp);
			//			printf("xxxxxxxxxxxxxxx\n");
			//			printNewCubeCondensed(result.S, result.g->n, result.h->n);
			if (result.foundIso) {
				appendSubtreeIsoDataStore(refinementSupport, result);
			} else {
				dumpNewCube(result.S, result.g->n, result.h->n);
			}
		}
		// if so, print and recurse
		if (refinementSupport->size >= threshold) {
			fprintf(stdout, "============\nNEW PATTERN:\n");
			printCanonicalString(cString, stdout);
			printSubtreeIsoDataStoreList(refinementSupport, stdout);

			iterativeDFS(refinementSupport, threshold, maxPatternSize, frequentEdges, processedPatterns, gp, sgp);
		}
		// clean up
		dumpSubtreeIsoDataStoreList(refinementSupport, gp);
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
					struct SubtreeIsoDataStoreList* edgeSupport = initIterativeDFS(db, nGraphs, e, gp);
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
			stupidPatternEvaluation(db, nGraphs, refinements, refinementSize, pointers, gp, sgp);

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

int main(int argc, char** argv) {
	return mainIterativeDFS(argc, argv);
	//	return mainDFS(argc, argv);
	//	return mainBFS(argc, argv);
}
