#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../searchTree.h"
#include "../loading.h"
#include "../outerplanar.h"
#include "../bloomFilter.h"
#include "../cs_Parsing.h"
#include "../treeEnumeration.h"
#include "../levelwiseTreePatternMining.h"
#include "../cactustree.h"

char DEBUG_INFO = 1;

typedef enum {
		wilson,
		listing,
		mix,
		cactus,
		bridgeForest,
		listOrSample
	} SamplingMethod;	



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
			initialAddToPruningSet(results[v].id, i);
		}
	}
}


void stupidPatternEvaluation(struct Graph** db, int nGraphs, struct Graph** patterns, int nPatterns, struct Vertex** pointers, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int i;
	for (i=0; i<nGraphs; ++i) {
		int j;
		for (j=0; j<nPatterns; ++j) {
			if (cactusTreeSubIso(db[i], patterns[j], gp, sgp)) {
				++pointers[j]->visited;
			}
		}
	}
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
	//struct Graph* g = NULL;

	/* user input handling variables */
	int threshold = 1000;
	//char unsafe = 0;

	/* i counts the number of graphs read */
	//int i = 0;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ht:u";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
			break;
		// case 'u':
		// 	unsafe = 1;
		// 	break;
		case 't':
			if (sscanf(optarg, "%i", &threshold) != 1) {
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
		FILE* featureStream = fopen("/dev/null", "w");
		FILE* patternStream = stdout;

		int debugInfo = 1;
		int i;

		struct Vertex* frequentPatterns;
		struct ShallowGraph* extensionEdges;
		int patternSize;

		struct Vertex* frequentVertices = getVertex(vp);
		struct Vertex* frequentEdges = getVertex(vp);
		struct Graph** db = NULL;
		int nGraphs = 128;
		int tmpResultSetSize;

		/* init data structures */
		initPruning(nGraphs);
		nGraphs = getDB(&db);
		destroyFileIterator(); // graphs are in memory now

		/* get frequent vertices */
		tmpResultSetSize = getFrequentVertices(db, nGraphs, frequentVertices, kvStream, gp);
		filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureStream, gp);
		fprintf(patternStream, "patterns size 0\n");
		printStringsInSearchTree(frequentVertices, patternStream, sgp);

		/* get frequent edges: first edge id is given by number of frequent vertices */
		offsetSearchTreeIds(frequentEdges, frequentVertices->lowPoint);
		getFrequentEdges(db, nGraphs, tmpResultSetSize, frequentEdges, kvStream, gp);
		filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureStream, gp);
		fprintf(patternStream, "patterns size 1\n");
		printStringsInSearchTree(frequentEdges, patternStream, sgp);

		/* convert frequentEdges to ShallowGraph */
		extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, gp, sgp);	

		/* start with patterns containing two edges */
		for (frequentPatterns = frequentEdges, patternSize = 2; (frequentPatterns->d > 0) && (/* TODO */ patternSize < 4); ++patternSize) {
			struct ShallowGraph* prefix = getShallowGraph(sgp);
			struct Vertex* candidateSet;
			struct Vertex** pointers;
			struct Graph** refinements;
			int refinementSize;
			
			candidateSet = generateCandidateSet(frequentPatterns, extensionEdges, gp, sgp);
			
			fprintf(stdout, "level %i candidates\n", patternSize);
			printStringsInSearchTree(candidateSet, stdout, sgp);

			refinementSize = candidateSet->d;
			pointers = malloc(refinementSize * sizeof(struct Vertex*));
			refinements = malloc(refinementSize * sizeof(struct Graph*));

			makeGraphsAndPointers(candidateSet, candidateSet, refinements, pointers, 0, prefix, gp, sgp); 
			stupidPatternEvaluation(db, nGraphs, refinements, refinementSize, pointers, gp, sgp);

			/* threshold + 1 as candidateSet contains each candidate once, already */
			filterSearchTreeP(candidateSet, threshold + 1, candidateSet, featureStream, gp);

			fprintf(patternStream, "patterns size %i\n", patternSize);
			printStringsInSearchTree(candidateSet, patternStream, sgp); 

			if (debugInfo) { fprintf(stderr, "Computation of level %i done\n", patternSize); }

			/* garbage collection */
			dumpSearchTree(gp, frequentPatterns);
			dumpShallowGraph(sgp, prefix);
			free(pointers);
			for (i=0; i<refinementSize; ++i) {
				dumpGraph(gp, refinements[i]);
			}
			free(refinements);
			frequentPatterns = candidateSet;

			/* flush the output */
			fflush(featureStream);
			fflush(kvStream);
			fflush(patternStream);
		}

		// garbage collection
		freePruning();
		dumpSearchTree(gp, frequentVertices);
		dumpSearchTree(gp, frequentEdges);

		for (i=0; i<nGraphs; ++i) {
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
