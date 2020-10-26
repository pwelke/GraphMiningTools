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

#include "lwm_initAndCollect.h"
#include "lwmr_initAndCollect.h"


/**
 * Load the graph database as directed graphs!
 */
int getDirectedDB(struct Graph*** db) {
	struct Graph* g = NULL;
	int dbSize = 0;
	int i = 0;

	while ((g = iterateFileDirected())) {
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


static void getFrequentDirectedEdges(struct Graph** db, int dbSize, int initialResultSetSize, struct Vertex* frequentEdges, struct GraphPool* gp) {
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

				/* as for vertices, I use specialized code to generate
				the canonical string of a single directed edge */

				/* cString = v (e w) */
				struct VertexList* cString;
				cString = getTerminatorEdge(gp->listPool);

				struct VertexList* tmp = getVertexList(gp->listPool);
				tmp->label = e->endPoint->label;
				tmp->next = cString;

				cString = getVertexList(gp->listPool);
				cString->label = e->label;
				cString->next = tmp;

				tmp = getInitialisatorEdge(gp->listPool);
				tmp->next = cString;

				cString = getVertexList(gp->listPool);
				cString->label = e->startPoint->label;
				cString->next = tmp;

				/* add the string to the search tree */
				containedEdges->d += addStringToSearchTree(containedEdges, cString, gp);
				containedEdges->number += 1;
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
Create a list of single-edge ShallowGraphs from a search tree containing single edge canonical strings.
Note that this method creates a hardcopy of the edges, strings and vertices.
Hence, it requires a pointer to a struct Graph* newVertices variable where it stores a newly created graph that holds all the new vertices.
To avoid memory leaks, this graph needs to be dumped together with the struct ShallowGraph* result of this method.
*/
static struct ShallowGraph* directedEdgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct Graph** newVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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


static void getFrequentVerticesAndDirectedEdges(struct Graph** db, int nGraphs, size_t threshold, struct Vertex** frequentVertices, struct Vertex** frequentEdges, FILE* logStream, struct GraphPool* gp) {
	*frequentVertices = getVertex(gp->vertexPool);
	*frequentEdges = getVertex(gp->vertexPool);

	/* get frequent vertices */
	int tmpResultSetSize = getFrequentVertices(db, nGraphs, *frequentVertices, gp);
	filterSearchTree(*frequentVertices, threshold, *frequentVertices, gp);
	fprintf(logStream, "Number of frequent vertices: %i\n", (*frequentVertices)->d); fflush(logStream);

	/* get frequent edges: first edge id is given by number of frequent vertices */
	offsetSearchTreeIds(*frequentEdges, (*frequentVertices)->lowPoint);
	getFrequentDirectedEdges(db, nGraphs, tmpResultSetSize, *frequentEdges, gp);
	filterSearchTree(*frequentEdges, threshold, *frequentEdges, gp);
	fprintf(logStream, "Number of frequent edges: %i\n", (*frequentEdges)->d); fflush(logStream);

}


size_t initDirectedPatternEnumeration(// input
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
	int nGraphs = getDirectedDB(&db);

	printGraph(db[0]);

	struct Vertex* frequentVertices;
	struct Vertex* frequentEdges;
	getFrequentVerticesAndDirectedEdges(db, nGraphs, threshold, &frequentVertices, &frequentEdges, logStream, gp);

	printf("\n\nfrequent Edges\n");
	printStringsInSearchTree(frequentEdges, stdout, sgp);


	/* convert frequentEdges to ShallowGraph of extension edges */
	struct Graph* extensionEdgesVertexStore = NULL;
	struct ShallowGraph* extensionEdges = directedEdgeSearchTree2ShallowGraph(frequentEdges, &extensionEdgesVertexStore, gp, sgp);
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

void garbageCollectDirectedPatternEnumeration(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	garbageCollectPatternEnumeration(y, gp, sgp);
}
