/*
 * lwm_initAndCollect.c
 *
 *  Created on: Mar 27, 2018
 *      Author: pascal
 */

#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "loading.h"
#include "outerplanar.h"

#include "searchTree.h"
#include "cs_Parsing.h"
#include "cs_Tree.h"

#include "sampleSubtrees.h"

#include "subtreeIsoUtils.h"
#include "localEasySubtreeIsomorphism.h"



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
		if (g->n < 2) {
			h = cloneGraph(g, gp);
		} else {
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
		}

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


static int getAllSpanningTreesOfDB(struct Graph*** db, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		struct Graph* h = NULL;
		if (g->n < 2) {
			h = cloneGraph(g, gp);
		} else {
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
		}

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


static int getNonisomorphicSpanningTreeSamplesOfDB(struct Graph*** db, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFile())) {
		struct Vertex* searchTree = getVertex(gp->vertexPool);
		struct ShallowGraph* sample = NULL;
		if (g->n < 2) {
			// a singleton is a tree. we add it / its unique spanning tree to the search tree
			addToSearchTree(searchTree, canonicalStringOfTree(g, sgp), gp, sgp);
		} else {
			// sample k spanning trees, canonicalize them and add them in a search tree (to avoid duplicates, i.e. isomorphic spanning trees)
			sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingWilson, g, k, k, 1, gp, sgp);
			for (struct ShallowGraph* tree=sample; tree!=NULL; tree=tree->next) {
				if (tree->m != 0) {
					struct Graph* tmp = shallowGraphToGraph(tree, gp);
					addToSearchTree(searchTree, canonicalStringOfTree(tmp, sgp), gp, sgp);
					/* garbage collection */
					dumpGraph(gp, tmp);
				}
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



int getDBfromCanonicalStrings(struct Graph*** db, FILE* stream, size_t bufferSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int dbSize = 0;
	int i = 0;
	int graphId = -1;
	int graphCount = -1;
	char* buffer = malloc(bufferSize * sizeof(char));

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
	free(buffer);
	return i;
}


/**
Find the frequent vertices in a graph db given by an array of graphs.
The frequent vertices are stored in the search tree, the return value of this function is the size of the
temporary data structure for merging search trees.
 */
static int getFrequentVertices(struct Graph** db, int dbSize, struct Vertex* frequentVertices, struct GraphPool* gp) {
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


static void getFrequentEdges(struct Graph** db, int dbSize, int initialResultSetSize, struct Vertex* frequentEdges, struct GraphPool* gp) {
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




static struct SupportSet* getSupportSetsOfVertices(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SupportSet* actualSupport = getSupportSet();
	h->number = patternId;
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore base = {0};
		base.g = db[i];
		base.postorder = postoderDB[i];
		struct SubtreeIsoDataStore data = initIterativeSubtreeCheckForSingleton(base, h);
		if (data.foundIso) {
			appendSupportSetData(actualSupport, data);
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
static char singletonSubgraphCheck(struct Graph* g, struct Graph* h) {
	char* vertexLabel = h->vertices[0]->label;
	for (int v=0; v<g->n; ++v) {
		if (labelCmp(g->vertices[v]->label, vertexLabel) == 0) {
			return 1;
		}
	}
	return 0;
}

static struct SupportSet* initLocalEasyForVertices(struct SpanningtreeTree* spanningTreesDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SupportSet* actualSupport = getSupportSet();
	h->number = patternId;
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore data = {0};
		data.g = spanningTreesDB[i].g;
		data.h = h;
		data.postorder = (int*)&(spanningTreesDB[i]); // we store a spanningtreetree pointer instead of a int
		data.foundIso = singletonSubgraphCheck(data.g, data.h);
		if (data.foundIso) {
			appendSupportSetData(actualSupport, data);
		}
	}
	return actualSupport;
}


static int** getPostorders(struct Graph** db, int nGraphs) {
	int** postorderDB = malloc(nGraphs * sizeof(int*));
	for (int g=0; g<nGraphs; ++g) {
		postorderDB[g] = getPostorder(db[g], 0);
	}
	return postorderDB;
}


static void getFrequentVerticesAndEdges(struct Graph** db, int nGraphs, size_t threshold, struct Vertex** frequentVertices, struct Vertex** frequentEdges, FILE* logStream, struct GraphPool* gp) {
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
static struct SupportSet* createSingletonPatternSupportSetsForForestDB(struct Graph** db, int** postorders, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	// init levelwise search data structures for patterns with one vertex

	// data structures for iterative levelwise search
	struct SupportSet* vertexSupportSets = NULL;
	struct SupportSet* vertexSupportSetsTail = NULL;

	int id = 1;
	for (struct VertexList* e=frequentVertices->neighborhood; e!=NULL; e=e->next) {

		struct Graph* candidate = createGraph(1, gp);
		candidate->vertices[0]->label = e->label;
		e->endPoint->lowPoint = id;

		struct SupportSet* vertexSupport = getSupportSetsOfVertices(db, postorders, nGraphs, candidate, id);

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
static struct SupportSet* createSingletonPatternSupportSetsForLocalEasyDB(struct SpanningtreeTree* sptTrees, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	// init levelwise search data structures for patterns with one vertex

	// data structures for iterative levelwise search
	struct SupportSet* vertexSupportSets = NULL;
	struct SupportSet* vertexSupportSetsTail = NULL;

	int id = 1;
	for (struct VertexList* e=frequentVertices->neighborhood; e!=NULL; e=e->next) {

		struct Graph* candidate = createGraph(1, gp);
		candidate->vertices[0]->label = e->label;
		e->endPoint->lowPoint = id;

		struct SupportSet* vertexSupport = initLocalEasyForVertices(sptTrees, nGraphs, candidate, id);

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
Create a list of single-edge ShallowGraphs from a search tree containing single edge canonical strings.
Note that this method creates a hardcopy of the edges, strings and vertices.
Hence, it requires a pointer to a struct Graph* newVertices variable where it stores a newly created graph that holds all the new vertices.
To avoid memory leaks, this graph needs to be dumped together with the struct ShallowGraph* result of this method.
*/
static struct ShallowGraph* edgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct Graph** newVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* result = getShallowGraph(sgp);
	struct VertexList* e;
	struct Vertex* createdVertices = NULL;
	int nVertices = 0;
	int i;
	struct Vertex* tmp;

	for (e=frequentEdges->neighborhood; e!=NULL; e=e->next) {
		struct Vertex* v = getVertex(gp->vertexPool);
		struct VertexList* f;
		v->label = e->label;
		/* store newly created vertex in a list */
		v->next = createdVertices;
		createdVertices = v;
		++nVertices;

		for (f=e->endPoint->neighborhood->endPoint->neighborhood; f!=NULL; f=f->next) {
			struct VertexList* g;
			for (g=f->endPoint->neighborhood; g!=NULL; g=g->next) {
				struct VertexList* new = getVertexList(sgp->listPool);
				new->startPoint = v;
				new->label = f->label;
				new->endPoint = getVertex(gp->vertexPool);
				new->endPoint->label = g->label;
				pushEdge(result, new);
				/* if the edge has not identical vertex labels at both vertices,
				add the reverse edge to the output */
				if (strcmp(v->label, new->endPoint->label) != 0) {
					pushEdge(result, inverseEdge(new, sgp->listPool));
				}
				/* store newly created vertex in a list */
				new->endPoint->next = createdVertices;
				createdVertices = new->endPoint;
				++nVertices;
			}
		}
	}

	/* to avoid memory leaks, make hardcopies of the labels */
	for (e=result->edges; e!=NULL; e=e->next) {
		if (!e->isStringMaster) {
			e->isStringMaster = 1;
			e->label = copyString(e->label);
		}
		if (!e->startPoint->isStringMaster) {
			e->startPoint->isStringMaster = 1;
			e->startPoint->label = copyString(e->startPoint->label);
		}
		if (!e->endPoint->isStringMaster) {
			e->endPoint->isStringMaster = 1;
			e->endPoint->label = copyString(e->endPoint->label);
		}
	}

	/* to avoid memory leaks, make a graph containing all newly created vertices and return it*/
	*newVertices = getGraph(gp);
	setVertexNumber(*newVertices, nVertices);
	for (i=0, tmp=createdVertices; i<nVertices; ++i, tmp=tmp->next) {
		(*newVertices)->vertices[i] = tmp;
	}

	return result;
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
		struct SupportSet** supportSets,
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
	struct SupportSet* frequentVerticesSupportSets = createSingletonPatternSupportSetsForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSupportSetsSparse(frequentVerticesSupportSets, featureStream);

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
		struct SupportSet** supportSets,
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
	struct SupportSet* frequentVerticesSupportSets = createSingletonPatternSupportSetsForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSupportSetsSparse(frequentVerticesSupportSets, featureStream);

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
		struct SupportSet** supportSets,
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
	struct SupportSet* frequentVerticesSupportSets = createSingletonPatternSupportSetsForForestDB(db, postorders, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSupportSetsSparse(frequentVerticesSupportSets, featureStream);

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
		struct SupportSet** supportSets,
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
	struct SupportSet* frequentVerticesSupportSets = createSingletonPatternSupportSetsForLocalEasyDB(sptTrees, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSupportSetsSparse(frequentVerticesSupportSets, featureStream);

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
		struct SupportSet** supportSets,
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
	struct SupportSet* frequentVerticesSupportSets = createSingletonPatternSupportSetsForLocalEasyDB(sptTrees, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSupportSetsSparse(frequentVerticesSupportSets, featureStream);

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


static struct SupportSet* initPatternEnumerationForVertices(struct Graph** spanningTreesDB, size_t nGraphs, struct Graph* h, int patternId) {
	struct SupportSet* actualSupport = getSupportSet();
	h->number = patternId;
	for (size_t i=0; i<nGraphs; ++i) {
		struct SubtreeIsoDataStore data = {0};
		data.g = spanningTreesDB[i];
		data.h = h;
		data.postorder = NULL;
		data.foundIso = 1;
		appendSupportSetData(actualSupport, data);
	}
	return actualSupport;
}


/**
 * create data structures for levelwise mining for subtree and iterative subtree embedding operator
 * for all frequent vertices in the db.
 * the ids of the frequent vertices in the search tree might be altered to ensure a sorted list of support sets.
 * To avoid leaks, the initial frequentVertices search tree must not be dumped until the end of all times.
 */
static struct SupportSet* getSupportSetsOfVerticesForPatternEnumeration(struct Graph** db, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	// init levelwise search data structures for patterns with one vertex

	// data structures for iterative levelwise search
	struct SupportSet* vertexSupportSets = NULL;
	struct SupportSet* vertexSupportSetsTail = NULL;

	int id = 1;
	for (struct VertexList* e=frequentVertices->neighborhood; e!=NULL; e=e->next) {

		struct Graph* candidate = createGraph(1, gp);
		candidate->vertices[0]->label = e->label;
		e->endPoint->lowPoint = id;

		struct SupportSet* vertexSupport = initPatternEnumerationForVertices(db, nGraphs, candidate, id);

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
		struct SupportSet** supportSets,
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
	struct SupportSet* frequentVerticesSupportSets = getSupportSetsOfVerticesForPatternEnumeration(db, nGraphs, frequentVertices, gp, sgp);
	printStringsInSearchTree(frequentVertices, patternStream, sgp);
	printSupportSetsSparse(frequentVerticesSupportSets, featureStream);

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



void garbageCollectPatternEnumeration(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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
