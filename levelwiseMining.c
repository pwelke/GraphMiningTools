#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "treeCenter.h"
#include "canonicalString.h"
#include "searchTree.h"
#include "levelwiseMining.h"

/**
levelwise algorithm 

input: the spanning tree pattern db of a graph db, some constraining parameters?, threshold 
output: frequent subtrees of the db

questions:
how to represent the spanning trees? 
how to extend a pattern?
how to store results - check each pattern against db separately. just count how many times it is contained. 

*/

// void iteratePatternDB(char* fileName, int minGraph, int maxGraph, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

// 	FILE* stream = fopen(fileName, "r");
// 	struct ShallowGraph* patterns = NULL;
// 	int i = 0;
	
// 	// /* iterate over all graphs in the database */
// 	while (((i < maxGraph) || (maxGraph == -1)) && (patterns = streamReadPatterns(stream, 100, sgp))) {
// 		// TODO what needs to be done

// 		/* do not alter */
// 		++i;
// 		dumpShallowGraphCycle(sgp, patterns);
// 	}
// 	fclose(stream);
// }

/**
return the histogram of vertices and edges in a db in a search tree.

the db is expected to have the format outputted by printStringsInSearchTree().
both output parameters (frequent* ) should be initialized to "empty" structs.
fileName specifies the file the db is contained in.
mingraph and maxgraph specify a range in which to read patterns.
*/
void getFrequentVerticesAndEdges(char* fileName, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int bufferSize = 100;
	int i = 0;
	FILE* stream = fopen(fileName, "r");
	struct ShallowGraph* patterns = NULL;
	
	/* iterate over all graphs in the database */
	while (((i < maxGraph) || (maxGraph == -1)) && (patterns = streamReadPatterns(stream, bufferSize, sgp))) {
		struct ShallowGraph* pattern = patterns;

		/* frequency of an edge increases by one if there exists a pattern for the current graph (a spanning tree) 
		that contains the edge. Thus we need to find all edges contained in any spanning tree and then add them 
		to frequentEdges once omitting multiplicity */
		struct Vertex* containedEdges = getVertex(gp->vertexPool);

		/* the vertices contained in g can be obtained from a single spanning tree, as all spanning trees contain
		the same vertex set. However, to omit multiplicity, we again resort to a temporary searchTree */
		struct Vertex* containedVertices = getVertex(gp->vertexPool);

		/* get frequent vertices */
		struct Graph* patternGraph = canonicalString2Graph(pattern, gp);

		/* auxiliary contains a single vertex */
		struct Graph* auxiliary = createGraph(1, gp);
		int v;
		for (v=0; v<patternGraph->n; ++v) {
			struct ShallowGraph* cString;
			auxiliary->vertices[0]->label = patternGraph->vertices[v]->label;
			cString = treeCenterCanonicalString(auxiliary, sgp);
			addToSearchTree(containedVertices, cString, gp, sgp);
		}

		/* set multiplicity of patterns to 1 and add to global vertex pattern set */
		resetToUnique(containedVertices);
		mergeSearchTrees(frequentVertices, containedVertices, 1, NULL, NULL, frequentVertices, 0, gp);

		/* garbage collection */
		dumpGraph(gp, auxiliary);
		dumpSearchTree(gp, containedVertices);
		
		/* get frequent Edges */
		auxiliary = createGraph(2, gp);
		addEdgeBetweenVertices(0, 1, NULL, auxiliary, gp);

		for (pattern=pattern->next; pattern!=NULL; pattern=pattern->next) {
			for (v=0; v<patternGraph->n; ++v) {
				struct VertexList* e;
				for (e=patternGraph->vertices[v]->neighborhood; e!=NULL; e=e->next) {
					int w = e->endPoint->number;
					if (w > v) {
						struct ShallowGraph* cString;
						auxiliary->vertices[0]->neighborhood->label = e->label;
						auxiliary->vertices[1]->neighborhood->label = e->label;
						auxiliary->vertices[0]->label = e->startPoint->label;
						auxiliary->vertices[1]->label = e->endPoint->label;

						cString = treeCenterCanonicalString(auxiliary, sgp);
						addToSearchTree(containedEdges, cString, gp, sgp);
					} 
				}
			}
			dumpGraph(gp, patternGraph);
			if (pattern->next != NULL) {
				patternGraph = canonicalString2Graph(pattern, gp);
			}
		}
		/* set multiplicity of patterns to 1 and add to global edge pattern set */
		resetToUnique(containedEdges);
		mergeSearchTrees(frequentEdges, containedEdges, 1, NULL, NULL, frequentEdges, 0, gp);

		/* garbage collection */
		dumpSearchTree(gp, containedEdges);
		dumpGraph(gp, auxiliary);

		/* do not alter */
		++i;
		dumpShallowGraphCycle(sgp, patterns);
	}
	fclose(stream);
}


/**
 * Creates a hard copy of g. Vertices and edges have the same numbers and
 * labels but are different objects in memory. Thus altering (deleting, adding etc.)
 * anything in the copy does not have any impact on g.
 *
 * Deleting a vertex or an edge in g, however, results in a memory leak, as vertices
 * and edges of the copy reference the label strings of the original structures.
 *
 * TODO a similar method that can handle induced subgraphs properly.
 */
struct Graph* refinementGraph(struct Graph* g, int currentVertex, struct VertexList* newEdge, struct GraphPool* gp) {
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
	addEdgeBetweenVertices(currentVertex, copy->n - 1, newEdge->label, copy, gp);

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
the label of the endpoint defins the label of the ne vertex added

TODO there is speedup to gain here. The appending at the moment runs in o(#candidates * n).
*/
struct Graph* extendPattern(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {
	struct Graph* rho = NULL;
	struct VertexList* e;
	for (e=candidateEdges->edges; e!=NULL; e=e->next) {
		int v;
		for (v=0; v<g->n; ++v) {
			/* if the labels are compatible */
			if (strcmp(g->vertices[v]->label, e->label) == 0) {
				struct Graph* h = refinementGraph(g, v, e, gp);
				h->next = rho;
				rho = h;
			}
		}	
	}
	return rho;
}

char alreadyEnumerated(struct Graph* pattern, struct Vertex* searchTree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* string = treeCenterCanonicalString(pattern, sgp);
	char alreadyFound = containsString(searchTree, string);
	dumpShallowGraph(sgp, string);
	return alreadyFound;
}


/**
 * Removes vertex w from the adjacency list of vertex v
 */
struct VertexList* snatchEdge(struct Vertex* v, struct Vertex* w) {
	struct VertexList* idx, *tmp;

	if (v->neighborhood->endPoint == w) {
		tmp = v->neighborhood;
		v->neighborhood = v->neighborhood->next;	
		tmp->next = NULL;
		return tmp;
	}

	tmp = v->neighborhood;
	for (idx=tmp->next; idx; idx=idx->next, tmp=tmp->next) {
		if (idx->endPoint == w) {
			tmp->next = idx->next;
			idx->next = NULL;
			return idx;
		}
	}
	return NULL;
}

char subtreeIsInfrequent(struct Graph* pattern, struct Vertex* searchTree, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int v;

	/* mark leaves */
	// TODO small speedup, if we go to n-1, as last vertex is the newest leaf
	for (v=0; v<pattern->n; ++v) {
		if (isLeaf(pattern->vertices[v])) {
			struct Vertex* leaf;
			struct VertexList* e;
			struct Graph* subPattern;
			struct ShallowGraph* string;

			/* remove vertex and edge from graph */
			leaf = pattern->vertices[v];
			pattern->vertices[v] = NULL;
			e = snatchEdge(leaf->neighborhood->endPoint, leaf);

			/* get induced subgraph, add vertex and leaf to graph again */
			// TODO speedup by reusing inducedSubgraph
			subPattern = cloneInducedGraph(pattern, gp);
			string = treeCenterCanonicalString(subPattern, sgp);
			pattern->vertices[v] = leaf;
			addEdge(e->startPoint, e);

			/* check if infrequent, if so return 1 otherwise continue */
			if (!containsString(searchTree, string)) {
				dumpShallowGraph(sgp, string);
				dumpGraph(gp, subPattern);
				return 1;
			} else {
				dumpShallowGraph(sgp, string);
				dumpGraph(gp, subPattern);
			}
		}
	}
	return 0;
}

/**
better filter: remove each leaf and check if the subtree is contained in the frequent patterns
*/
struct Graph* filterExtension(struct Graph* extension, struct Vertex* patternSearchTree, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* idx = extension; 
	struct Graph* filteredExtension = NULL;
	
	while (idx != NULL) {
		struct Graph* current = idx;
		idx = idx->next;
		current->next = NULL;

		/* filter out patterns that were already enumerated as the extension of some other pattern
		and are in the search tree */
		if (alreadyEnumerated(current, patternSearchTree, sgp)) {
 			dumpGraph(gp, current);
		} else {
			/* filter out patterns where a subtree is not frequent */
			if (subtreeIsInfrequent(current, patternSearchTree, gp, sgp)) {
				dumpGraph(gp, current);
			} else {
				current->next = filteredExtension;
				filteredExtension = current;
			}
		}
	}
	return filteredExtension;
}


void recAccess(struct Vertex* frequentPatterns, struct ShallowGraph* frequentEdges, struct Vertex* root, struct Vertex* parent, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if (root->visited != 0) {
		/* at this point, we have found a pattern, we want to make a tree from it, get the refinements,
		filter interesting candidates and then scan the db of patterns  */
		struct Graph* pattern = canonicalString2Graph(prefix, gp);
		struct Graph* refinements = extendPattern(pattern, frequentEdges, gp);
		refinements = filterExtension(pattern, frequentPatterns, gp, sgp);

	}

	/* recursively access the subtree dangling from root */
	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint != parent) {
			/* after finishing this block, we want prefix to be as before, thus we have
			   to do some list magic */
			struct VertexList* lastEdge = prefix->lastEdge;
			appendEdge(prefix, shallowCopyEdge(e, sgp->listPool));

			recAccess(frequentPatterns, frequentEdges, e->endPoint, root, prefix, gp, sgp);

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
}


void accessSearchTree(struct Vertex* root, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* prefix = getShallowGraph(sgp);
	//recAccess(root, root, prefix, stream, sgp);
	dumpShallowGraph(sgp, prefix);
}