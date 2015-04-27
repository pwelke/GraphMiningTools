#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "cs_Tree.h"
#include "searchTree.h"
#include "bloomFilter.h"
#include "treeEnumeration.h"


/**
 Creates a hard copy of g that has a new vertex that is a copy of newEdge->endPoint
 and a new edge from the vertex v in g to the new vertex. with label newEdge->label.

 Does not check if newEdge->startPoint->label == g->vertices[v]->label!
 */
struct Graph* refinementGraph(struct Graph* g, int v, struct VertexList* newEdge, struct GraphPool* gp) {
	struct Graph* copy = getGraph(gp);
	int i;

	copy->activity = g->activity;
	copy->m = g->m + 1;
	copy->n = g->n + 1;
	copy->number = g->number;

	copy->vertices = malloc(copy->n * sizeof(struct Vertex*));

	/* copy vertices */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			copy->vertices[i] = shallowCopyVertex(g->vertices[i], gp->vertexPool);
		}
	}

	/* copy new vertex and edge */
	copy->vertices[copy->n - 1] = shallowCopyVertex(newEdge->endPoint, gp->vertexPool);
	copy->vertices[copy->n - 1]->number = copy->n - 1;
	addEdgeBetweenVertices(v, copy->n - 1, newEdge->label, copy, gp);

	/* copy edges */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			struct VertexList* e;
			for (e=g->vertices[i]->neighborhood; e; e=e->next) {
				struct VertexList* tmp = getVertexList(gp->listPool);
				tmp->endPoint = copy->vertices[e->endPoint->number];
				tmp->startPoint = copy->vertices[e->startPoint->number];
				tmp->label = e->label;

				/* add the shallow copy to the new graph */
				tmp->next = copy->vertices[e->startPoint->number]->neighborhood;
				copy->vertices[e->startPoint->number]->neighborhood = tmp;
			}
		}
	}
	return copy;
}


/** 
Take a tree and add any edge in the candidate set to each vertex in the graph in turn.
candidateEdges is expected to contain edges that have a nonNULL ->startPoint and ->endPoint.
The label of startPoint determines, which vertex can be used for appending the edge, 
the label of the endpoint defines the label of the ne vertex added

TODO there is speedup to gain here. The appending at the moment runs in o(#candidates * n).
*/
struct Graph* extendPattern(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {
	struct Graph* rho = NULL;
	struct VertexList* e;
	for (e=candidateEdges->edges; e!=NULL; e=e->next) {
		int v;
		for (v=0; v<g->n; ++v) {
			/* if the labels are compatible */
			if (strcmp(g->vertices[v]->label, e->startPoint->label) == 0) {
				struct Graph* h = refinementGraph(g, v, e, gp);
				h->next = rho;
				rho = h;
			}
		}	
	}
	return rho;
}


/**
check if the canonicalStringOfTree of pattern is already contained in search tree
*/
char alreadyEnumerated(struct Graph* pattern, struct Vertex* searchTree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* string = canonicalStringOfTree(pattern, sgp);
	char alreadyFound = containsString(searchTree, string);
	dumpShallowGraph(sgp, string);
	return alreadyFound;
}


/**
return the fingerprint of pattern (being the bitwise or of the hashes of the subgraphs)
OR 0 if there exists a subgraph of pattern that is not frequent.
*/
int getPatternFingerPrint(struct Graph* pattern, struct Vertex* searchTree, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int v;
	int fingerPrint = 0;

	/* mark leaves */
	/* TODO small speedup, if we go to n-1, as last vertex is the newest leaf */
	for (v=0; v<pattern->n; ++v) {
		if (isLeaf(pattern->vertices[v])) {
			struct Vertex* leaf;
			struct VertexList* e;
			struct Graph* subPattern;
			struct ShallowGraph* string;
			int stringID;

			/* remove vertex and edge from graph */
			leaf = pattern->vertices[v];
			pattern->vertices[v] = NULL;
			e = snatchEdge(leaf->neighborhood->endPoint, leaf);

			/* get induced subgraph, add vertex and leaf to graph again */
			/* TODO speedup by reusing inducedSubgraph */
			subPattern = cloneInducedGraph(pattern, gp);
			string = canonicalStringOfTree(subPattern, sgp);
			pattern->vertices[v] = leaf;
			addEdge(e->startPoint, e);

			/* check if infrequent, if so return 0 otherwise continue */
			stringID = getID(searchTree, string);
			if (stringID == -1) {
				dumpShallowGraph(sgp, string);
				dumpGraph(gp, subPattern);
				return 0;
			} else {
				dumpShallowGraph(sgp, string);
				dumpGraph(gp, subPattern);
				fingerPrint |= hashID(stringID);
			}
		}
	}
	return fingerPrint;
}


/**
for each graph g in extension (that is the list starting at extension and continuing at extension->next), 
two tests are run: g itself must not be contained in currentLevel and any subtree of g must be contained
in lowerLevel. 
if g fulfills both conditions, it is added to the output and to currentLevel.
*/ 
struct Graph* filterExtension(struct Graph* extension, struct Vertex* lowerLevel, struct Vertex* currentLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* idx = extension; 
	struct Graph* filteredExtension = NULL;
	
	while (idx != NULL) {
		struct Graph* current = idx;
		struct ShallowGraph* string;
		idx = idx->next;
		current->next = NULL;

		/* filter out patterns that were already enumerated as the extension of some other pattern
		and are in the search tree */
		string = canonicalStringOfTree(current, sgp);
		if (containsString(currentLevel, string)) {
			dumpShallowGraph(sgp, string);
 			dumpGraph(gp, current);
		} else {
			/* filter out patterns where a subtree is not frequent */
			int fingerPrint = getPatternFingerPrint(current, lowerLevel, gp, sgp);
			if (fingerPrint == 0) {
				dumpShallowGraph(sgp, string);
				dumpGraph(gp, current);
			} else {
				struct VertexList* e;
				for (e=string->edges; e!=NULL; e=e->next) {
					if (!e->isStringMaster) {
						e->isStringMaster = 1;
						e->label = copyString(e->label);
					}
				}
				current->next = filteredExtension;
				filteredExtension = current;

				/* add string to current level, update relevant bookkeeping info of search tree
				and then dump the empty shell of string */
				currentLevel->d += addStringToSearchTreeSetD(currentLevel, string->edges, fingerPrint, gp);
				++currentLevel->number;
				string->edges = NULL;
				dumpShallowGraph(sgp, string);
			}
		}
	}
	return filteredExtension;
}


static void generateCandidateSetRec(struct Vertex* lowerLevel, struct Vertex* currentLevel, struct ShallowGraph* frequentEdges, struct Vertex* root, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if ((root->visited != 0) && (root != lowerLevel)) {
		/* at this point, we have found a pattern, we want to make a tree from it, get the refinements,
		filter interesting candidates and then scan the db of patterns  */
		struct Graph* pattern = treeCanonicalString2Graph(prefix, gp);
		struct Graph* refinements = extendPattern(pattern, frequentEdges, gp);
		refinements = filterExtension(refinements, lowerLevel, currentLevel, gp, sgp);

		/* to just generate the search tree of candidates, we do not need the graphs any more */
		dumpGraph(gp, pattern);
		while (refinements != NULL) {
			struct Graph* next = refinements->next;
			refinements->next = NULL;
			dumpGraph(gp, refinements);
			refinements = next;
		}
	}

	/* recursively access the subtree dangling from root */
	for (e=root->neighborhood; e!=NULL; e=e->next) {	
		/* after finishing this block, we want prefix to be as before, thus we have
			to do some list magic */
		struct VertexList* lastEdge = prefix->lastEdge;
		appendEdge(prefix, shallowCopyEdge(e, sgp->listPool));

		generateCandidateSetRec(lowerLevel, currentLevel, frequentEdges, e->endPoint, prefix, gp, sgp);

		dumpVertexList(sgp->listPool, prefix->lastEdge);
		prefix->lastEdge = lastEdge;
		--prefix->m;

		if (prefix->m == 0) {
			prefix->edges = NULL;
		} else {
			lastEdge->next = NULL;
		}
	}
}


struct Vertex* generateCandidateSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Vertex* currentLevel = getVertex(gp->vertexPool);
	struct ShallowGraph* prefix = getShallowGraph(sgp);
	/* set smallest id of pattern in current level to be largest id of any pattern in lower level plus 1 */
	currentLevel->lowPoint = lowerLevel->lowPoint;
	generateCandidateSetRec(lowerLevel, currentLevel, extensionEdges, lowerLevel, prefix, gp, sgp);
	dumpShallowGraph(sgp, prefix);
	return currentLevel;
}
