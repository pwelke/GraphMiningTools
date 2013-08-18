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
traverses the db once.

the db is expected to have the format outputted by printStringsInSearchTree().
both output parameters (frequent* ) should be initialized to "empty" structs.
fileName specifies the file the db is contained in.
mingraph and maxgraph specify a range in which to read patterns.
*/
void getVertexAndEdgeHistograms(char* fileName, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int bufferSize = 100;
	int i = 0;
	FILE* stream = fopen(fileName, "r");
	struct ShallowGraph* patterns = NULL;
	
	/* iterate over all graphs in the database */
	while (((i < maxGraph) || (maxGraph == -1)) && (patterns = streamReadPatterns(stream, bufferSize, sgp))) {
		if (i >= minGraph) {
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
			int v;
			for (v=0; v<patternGraph->n; ++v) {
				/* See commented out how it would look if done by the book.
				However, this has to be fast and treeCenterCanonicalString has
				too much overhead!
				    struct ShallowGraph* cString;
				    auxiliary->vertices[0]->label = patternGraph->vertices[v]->label;
				    cString = treeCenterCanonicalString(auxiliary, sgp);
				    addToSearchTree(containedVertices, cString, gp, sgp); */
				struct VertexList* cString = getVertexList(sgp->listPool);
				cString->label = patternGraph->vertices[v]->label;
				containedVertices->d += addStringToSearchTree(containedVertices, cString, gp);
				containedVertices->number += 1;
			}
			/* set multiplicity of patterns to 1 and add to global vertex pattern set */
			resetToUnique(containedVertices);
			mergeSearchTrees(frequentVertices, containedVertices, 1, NULL, NULL, frequentVertices, 0, gp);
			dumpSearchTree(gp, containedVertices);
			
			/* get frequent Edges */
			for ( ; pattern!=NULL; pattern=pattern->next) {
				patternGraph = canonicalString2Graph(pattern, gp);
				for (v=0; v<patternGraph->n; ++v) {
					struct VertexList* e;
					for (e=patternGraph->vertices[v]->neighborhood; e!=NULL; e=e->next) {
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
				dumpGraph(gp, patternGraph);
			}
			/* set multiplicity of patterns to 1 and add to global edge pattern set */
			resetToUnique(containedEdges);
			mergeSearchTrees(frequentEdges, containedEdges, 1, NULL, NULL, frequentEdges, 0, gp);
			dumpSearchTree(gp, containedEdges);
		}

		/* counting of read graphs and garbage collection */
		++i;
		dumpShallowGraphCycle(sgp, patterns);
	}
	fclose(stream);
}


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


/**
check if the treeCenterCanonicalString of pattern is already contained in search tree
*/
char alreadyEnumerated(struct Graph* pattern, struct Vertex* searchTree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* string = treeCenterCanonicalString(pattern, sgp);
	char alreadyFound = containsString(searchTree, string);
	dumpShallowGraph(sgp, string);
	return alreadyFound;
}


/**
 * Removes vertex w from the adjacency list of vertex v and returns the edge
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


/**
if the treeCenterCanonicalString of any subtree of pattern (that is obtained by removing a leaf of pattern) 
is not contained in searchTree, this method returns false
*/
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
		string = treeCenterCanonicalString(current, sgp);
		if (containsString(currentLevel, string)) {
			dumpShallowGraph(sgp, string);
 			dumpGraph(gp, current);
		} else {
			/* filter out patterns where a subtree is not frequent */
			if (subtreeIsInfrequent(current, lowerLevel, gp, sgp)) {
				dumpShallowGraph(sgp, string);
				dumpGraph(gp, current);
			} else {
				current->next = filteredExtension;
				filteredExtension = current;
				addToSearchTree(currentLevel, string, gp, sgp);
			}
		}
	}
	return filteredExtension;
}


void recAccess(struct Vertex* lowerLevel, struct Vertex* currentLevel, struct ShallowGraph* frequentEdges, struct Vertex* root, struct Vertex* parent, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if (root->visited != 0) {
		/* at this point, we have found a pattern, we want to make a tree from it, get the refinements,
		filter interesting candidates and then scan the db of patterns  */
		struct Graph* pattern = canonicalString2Graph(prefix, gp);
		struct Graph* refinements = extendPattern(pattern, frequentEdges, gp);
		refinements = filterExtension(pattern, lowerLevel, currentLevel, gp, sgp);

	}

	/* recursively access the subtree dangling from root */
	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint != parent) {
			/* after finishing this block, we want prefix to be as before, thus we have
			   to do some list magic */
			struct VertexList* lastEdge = prefix->lastEdge;
			appendEdge(prefix, shallowCopyEdge(e, sgp->listPool));

			recAccess(lowerLevel, currentLevel, frequentEdges, e->endPoint, root, prefix, gp, sgp);

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