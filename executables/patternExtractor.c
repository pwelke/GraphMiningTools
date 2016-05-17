#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "../loading.h"
#include "../intSet.h"
#include "../levelwiseGraphMining.h"
#include "../subtreeIsomorphism.h"
#include "../minhashing.h"
#include "patternExtractor.h"


/**
 * Print --help message
 */
int printHelp() {
#include "patternExtractorHelp.help"
	unsigned char* help = executables_patternExtractorHelp_txt;
	int len = executables_patternExtractorHelp_txt_len;
	if (help != NULL) {
		int i=0;
		for (i=0; i<len; ++i) {
			fputc(help[i], stdout);
		}
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Could not read help file\n");
		return EXIT_FAILURE;
	}
}


/**
 * jenkinsOneAtATimeHash_step and jenkinsOneAtATimeHash_finalize() together implement
 * the Jenkins one at a time hash function for strings.
 *
 * I split them for hashing 'string's that are actually sequences of char* strings.
 *
 * Hence, for a given sequence of char* c strings, call jenkinsOneAtATimeHash_step()
 * for each char* in the sequence and finally use jenkinsOneAtATimeHash_finalize on
 * the result, as is done in pathHashCode().
 *
 * Source:
 * https://en.wikipedia.org/wiki/Jenkins_hash_function
 */
static uint32_t jenkinsOneAtATimeHash_step(char *key, uint32_t hash) {

	for(uint32_t i = 0; key[i] != '\0'; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	return hash;
}


/**
 * jenkinsOneAtATimeHash_step and jenkinsOneAtATimeHash_finalize() together implement
 * the Jenkins one at a time hash function for strings.
 *
 * I split them for hashing 'string's that are actually sequences of char* strings.
 *
 * Hence, for a given sequence of char* c strings, call jenkinsOneAtATimeHash_step()
 * for each char* in the sequence and finally use jenkinsOneAtATimeHash_finalize on
 * the result, as is done in pathHashCode().
 *
 * Source:
 * https://en.wikipedia.org/wiki/Jenkins_hash_function
 */
static uint32_t jenkinsOneAtATimeHash_finalize(uint32_t hash) {
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}


/**
 * compute a 32bit hash code of path based on the labels of the vertices and edges in the input.
 * The input is assumed to be a path, i.e. the endpoint of an edge is equal to the startpoint of the subsequent edge.
 */
static uint32_t pathHashCode(struct ShallowGraph* path) {
	uint32_t hash = 0;
	char* space = " ";
	for (struct VertexList* e=path->edges; e!=NULL; e=e->next) {
		hash = jenkinsOneAtATimeHash_step(e->startPoint->label, hash);
		hash = jenkinsOneAtATimeHash_step(space, hash);
		hash = jenkinsOneAtATimeHash_step(e->label, hash);
		hash = jenkinsOneAtATimeHash_step(space, hash);
	}
	return jenkinsOneAtATimeHash_finalize(hash);
}


/**
 * Check if an edge with endpoint w exists in the neighborhood of v and return a pointer to it.
 * Careful: the returned edge is no copy, but the original edge, still residing in the neighborhood list.
 */
static struct VertexList* hasEdge(struct Vertex* v, struct Vertex* w) {
	for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint == w) {
			return e;
		}
	}
	return NULL;
}


/**
 * Compute a 32bit hash for a 'cycle' of length 3, where the edges might be NULL, indicating that no edge exists.
 *
 * The hash value is canonical, i.e. the return value will be identical for any two isomorphic 'cycles'.
 */
uint32_t fingerprintTriple(struct Vertex* u, struct Vertex* v, struct Vertex* w, struct VertexList* uv, struct VertexList* vw, struct VertexList* wu, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* path = getShallowGraph(sgp);

	if (uv != NULL) {
		appendEdge(path, shallowCopyEdge(uv, sgp->listPool));
	} else {
		struct VertexList* e = getVertexList(sgp->listPool);
		e->startPoint = u;
		e->endPoint = v;
		e->label = "NULL";
		appendEdge(path, e);
	}

	if (vw != NULL) {
		appendEdge(path, shallowCopyEdge(vw, sgp->listPool));
	} else {
		struct VertexList* e = getVertexList(sgp->listPool);
		e->startPoint = v;
		e->endPoint = w;
		e->label = "NULL";
		appendEdge(path, e);
	}

	if (wu != NULL) {
		appendEdge(path, shallowCopyEdge(wu, sgp->listPool));
	} else {
		struct VertexList* e = getVertexList(sgp->listPool);
		e->startPoint = w;
		e->endPoint = u;
		e->label = "NULL";
		appendEdge(path, e);
	}

	struct ShallowGraph* reverse = getShallowGraph(sgp);
	for (struct VertexList* e=path->edges; e!=NULL; e=e->next) {
		pushEdge(reverse, inverseEdge(e, sgp->listPool));
	}

	uint32_t fingerprint = UINT32_MAX;
	for (int i=0; i<3; ++i) {
		uint32_t h1 = pathHashCode(path);
		uint32_t h2 = pathHashCode(reverse);
		if (fingerprint > h1) {
			fingerprint = h1;
		}
		if (fingerprint > h2) {
			fingerprint = h2;
		}
		// put next vertex up front
		appendEdge(path, popEdge(path));
		appendEdge(reverse, popEdge(reverse));
	}
	dumpShallowGraph(sgp, path);
	dumpShallowGraph(sgp, reverse);
	return fingerprint;
}


/**
 * Compute a set of fingerprints based on the set of all induced subgraphs of size 3.
 *
 * This implementation enumerates all possible combinations of three vertices and collects the (possibly missing)
 * edges between the three vertices. A fingerprint is computed and added to the output for each such graph using
 * fingerprintTriple().
 */
struct IntSet* getTripletFingerprintsBruteForce(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct IntSet* fingerprints = getIntSet();
	int n = g->n;
	// enumerate triplets of vertices
	for (int i = 0; i<n-2; ++i) {
		for (int j=i+1; j<n-1; ++j) {
			for (int k=j+1; k<n; ++k) {
				struct Vertex* u = g->vertices[i];
				struct Vertex* v = g->vertices[j];
				struct Vertex* w = g->vertices[k];
				struct VertexList* uv = hasEdge(u, v);
				struct VertexList* vw = hasEdge(v, w);
				struct VertexList* wu = hasEdge(w, u);
				addIntSortedNoDuplicates(fingerprints, fingerprintTriple(u, v, w, uv, vw, wu, sgp));
			}
		}
	}

	return fingerprints;
}


struct IntSet* computeSubtreeIsomorphisms(struct Graph* g, struct Graph** patterns, int nPatterns, struct GraphPool* gp) {
	struct IntSet* features = getIntSet();

	for (int i=0; i<nPatterns; ++i) {
		if (isSubtree(g, patterns[i], gp)) {
//			addIntSortedNoDuplicates(features, patterns[i]->number);
			addIntSortedNoDuplicates(features, i);
		}
	}
	return features;
}



/**
* Compute a set of fingerprints that is based on the triangles in a graph that contain at least two edges that are present.
* For sparse graphs, this is much faster than brute force enumeration of all induced subgraphs of size 3.
*
* For each vertex v of the graph, this implementation looks at all paths of length whose central vertex is v.
* For each such path, it checks if there is an edge connecting the endpoints, or not.
* These (possibly open) triangles are then hashed to numbers by fingerprintTriple().
*/
struct IntSet* getTripletFingerprintsTriangulation(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct IntSet* fingerprints = getIntSet();
	int n = g->n;
	// enumerate triplets of vertices
	for (int i=0; i<n; ++i) {
		struct Vertex* u = g->vertices[i];
		for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
			struct Vertex* v = e->endPoint;
			struct VertexList* vu = inverseEdge(e, sgp->listPool);
			for (struct VertexList* uw=e->next; uw!=NULL; uw=uw->next) {
				struct Vertex* w = uw->endPoint;
				struct VertexList* wv = hasEdge(w, v);
				addIntSortedNoDuplicates(fingerprints, fingerprintTriple(u, w, v, uw, wv, vu, sgp));
			}
			vu->next = NULL;
			dumpVertexList(sgp->listPool, vu);
		}
	}

	return fingerprints;
}

void printIntArrayNoId(int* a, int n) {
	for (int i=0; i<n; ++i) {
		printf("%i ", a[i]);
	}
	printf("\n");
}

void printIntArray(int* a, int n, int id) {
	printf("%i: ", id);
	printIntArrayNoId(a, n);
}

int main(int argc, char** argv) {

	typedef enum {triangles, bruteForceTriples, treePatterns, treePatternsFast, minHash} ExtractionMethod;
	typedef enum {CANONICALSTRING_INPUT, AIDS99_INPUT} InputMethod;

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* pointer to the current graph which is returned by the input iterator  */
	struct Graph* g = NULL;

	// handle user input
	ExtractionMethod method = triangles;
	char* patternFile = NULL;
	struct Graph** patterns = NULL;
	size_t nPatterns = 0;
	size_t hashSize = 5;
	InputMethod inputMethod = AIDS99_INPUT;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hm:f:c:k:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 'f':
			patternFile = optarg;
			inputMethod = AIDS99_INPUT;
			break;
		case 'c':
			patternFile = optarg;
			inputMethod = CANONICALSTRING_INPUT;
			break;
		case 'k':
			if (sscanf(optarg, "%zu", &hashSize) != 1) {
				fprintf(stderr, "Hash size argument must be unsigned integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'm':
			if (strcmp(optarg, "triangles") == 0) {
				method = triangles;
				break;
			}
			if (strcmp(optarg, "triples") == 0) {
				method = bruteForceTriples;
				break;
			}
			if (strcmp(optarg, "treePatterns") == 0) {
				method = treePatterns;
				break;
			}
			if (strcmp(optarg, "treePatternsFast") == 0) {
				method = treePatternsFast;
				break;
			}
			if (strcmp(optarg, "minHash") == 0) {
				method = minHash;
				break;
			}
			fprintf(stderr, "Unknown extraction method: %s\n", optarg);
			return EXIT_FAILURE;
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

	if ((method == treePatterns) || (method == minHash) || (method == treePatternsFast)) {
		if (patternFile == NULL) {
			fprintf(stderr, "No pattern file specified! Please do so using -a or -c\n");
			return EXIT_FAILURE;
		} else {
			FILE* patternStream = NULL;
			switch (inputMethod) {
			case AIDS99_INPUT:
				createFileIterator(patternFile, gp);
				nPatterns = getDB(&patterns);
				destroyFileIterator();
				break;
			case CANONICALSTRING_INPUT:
				patternStream = fopen(patternFile, "r");
				nPatterns = getDBfromCanonicalStrings(&patterns, patternStream, 20, gp, sgp);
				fclose(patternStream);
				break;
			}
		}
	}


	struct EvaluationPlan evaluationPlan = {0};
	if ((method == minHash) || (method == treePatternsFast)) {
		struct Graph* patternPoset = NULL;
		int** permutations = malloc(hashSize * sizeof(int*));
		size_t* permutationSizes = malloc(hashSize * sizeof(size_t));
		patternPoset = buildTreePosetFromGraphDB(patterns, nPatterns, gp, sgp);
		free(patterns); // we do not need this array any more. the graphs are accessible from patternPoset
		for (size_t i=0; i<hashSize; ++i) {
			permutations[i] = getRandomPermutation(nPatterns);
			permutationSizes[i] = posetPermutationMark(permutations[i], nPatterns, patternPoset);
			permutations[i] = posetPermutationShrink(permutations[i], nPatterns, permutationSizes[i]);
		}
		evaluationPlan = buildEvaluationPlan(permutations, permutationSizes, hashSize, patternPoset);
	}

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

	/* iterate over all graphs in the database */
	while ((g = iterateFile())) {
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n >= 0) {
			struct IntSet* fingerprints = NULL;
			switch (method) {
			case triangles:
				fingerprints = getTripletFingerprintsTriangulation(g, sgp);
				break;
			case bruteForceTriples:
				fingerprints = getTripletFingerprintsBruteForce(g, sgp);
				break;
			case treePatterns:
				fingerprints = computeSubtreeIsomorphisms(g, patterns, nPatterns, gp);
				break;
			case minHash:
				fingerprints = (struct IntSet*)fastMinHashForTrees(g, evaluationPlan, gp, sgp);
				break;
			case treePatternsFast:
				fingerprints = (struct IntSet*)explicitEmbeddingForTrees(g, evaluationPlan.F, gp, sgp);
				break;
			}
			// output
			switch (method) {
			case minHash:
				printIntArrayNoId((int*)fingerprints, evaluationPlan.sketchSize);
				free(fingerprints);
				break;
			default:
				printIntSetAsLibSvm(fingerprints, g->activity, stdout);
				dumpIntSet(fingerprints);
				break;
			}
		}


		/* garbage collection */
		dumpGraph(gp, g);
	}

	/* global garbage collection */
	if (method == minHash) {
		evaluationPlan = dumpEvaluationPlan(evaluationPlan, gp);
	}
	if (method == treePatterns) {
		for (size_t i=0; i<nPatterns; ++i) {
			dumpGraph(gp, patterns[i]);
		}
		free(patterns);
	}
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}
