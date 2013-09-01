#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "treeCenter.h"
#include "cachedGraph.h"
#include "subtreeIsomorphismLabeled.h"
#include "canonicalString.h"
#include "searchTree.h"
#include "levelwiseMining.h"

int* pruning;

void initPruning(int nGraphs) {
	if ((pruning = malloc(nGraphs * sizeof(int)))) {
		int i;
		for (i=0; i<nGraphs; ++i) {
			pruning[i] = 0;
		}
	}
}

void freePruning() {
	free(pruning);
}

inline int hashID(const int elementID) {
	return 1<<(elementID % sizeof(int));
}

inline void addToPruningSet(const int elementID, const int index) {
	pruning[index] |= hashID(elementID);
}

inline char containedInPruningSet(const int elementID, const int index) {
	return (pruning[index] & hashID(elementID)) != 0;
}


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
	int number;
	
	/* iterate over all graphs in the database */
	while (((i < maxGraph) || (maxGraph == -1)) && (patterns = streamReadPatterns(stream, bufferSize, &number, sgp))) {
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
			/* set multiplicity of patterns to 1 and add to global vertex pattern set, print to file */
			resetToUnique(containedVertices);
			mergeSearchTrees(frequentVertices, containedVertices, 1, NULL, NULL, frequentVertices, 0, gp);
			dumpSearchTree(gp, containedVertices);
			
			/* get frequent Edges */
			/* set the lowest id given to any edge to the highest id of any frequent vertex */
			frequentEdges->lowPoint = frequentVertices->lowPoint;
			for ( ; pattern!=NULL; pattern=pattern->next) {
				if (patternGraph == NULL) {
					patternGraph = canonicalString2Graph(pattern, gp);
				}
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
				patternGraph = NULL;
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
return the histogram of vertices and edges in a db in a search tree.
traverses the db once.

the db is expected to have the format outputted by printStringsInSearchTree().
both output parameters (frequent* ) should be initialized to "empty" structs.
fileName specifies the file the db is contained in.
mingraph and maxgraph specify a range in which to read patterns.
*/
void getVertexAndEdgeHistogramsP(char* fileName, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int bufferSize = 100;
	int i = 0;
	FILE* stream = fopen(fileName, "r");
	struct ShallowGraph* patterns = NULL;
	int number;
	struct compInfo* results = NULL;
	int resultSize = 0;
	
	/* iterate over all graphs in the database */
	while (((i < maxGraph) || (maxGraph == -1)) && (patterns = streamReadPatterns(stream, bufferSize, &number, sgp))) {
		if (i >= minGraph) {
			struct ShallowGraph* pattern = patterns;
			struct Graph* patternGraph;
			int v;

			/* frequency of an edge increases by one if there exists a pattern for the current graph (a spanning tree) 
			that contains the edge. Thus we need to find all edges contained in any spanning tree and then add them 
			to frequentEdges once omitting multiplicity */
			struct Vertex* containedEdges = getVertex(gp->vertexPool);

			/* the vertices contained in g can be obtained from a single spanning tree, as all spanning trees contain
			the same vertex set. However, to omit multiplicity, we again resort to a temporary searchTree */
			struct Vertex* containedVertices = getVertex(gp->vertexPool);

			/* init temporary result storage if necessary */
			int neededResultSize = patterns->m + 1;
			int resultPos = 0;
			if (neededResultSize > resultSize) {
				if (results) {
					free(results);
				}

				results = getResultVector(neededResultSize);
				resultSize = neededResultSize;
			}

			/* get frequent vertices */
			patternGraph = canonicalString2Graph(pattern, gp);
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
			/* set multiplicity of patterns to 1 and add to global vertex pattern set, print to file */
			resetToUnique(containedVertices);
			mergeSearchTrees(frequentVertices, containedVertices, 1, results, &resultPos, frequentVertices, 0, gp);
			dumpSearchTree(gp, containedVertices);

			/* write (graph->number, pattern id) pairs to stream */
			for (v=0; v<resultPos; ++v) {
				fprintf(keyValueStream, "%i %i\n", number, results[v].id);
			}
			
			/* get frequent Edges */
			resultPos = 0;
			for ( ; pattern!=NULL; pattern=pattern->next) {
				if (patternGraph == NULL) {
					patternGraph = canonicalString2Graph(pattern, gp);
				}
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
				patternGraph = NULL;
			}
			/* set multiplicity of patterns to 1 and add to global edge pattern set */
			resetToUnique(containedEdges);
			mergeSearchTrees(frequentEdges, containedEdges, 1, results, &resultPos, frequentEdges, 0, gp);
			dumpSearchTree(gp, containedEdges);
			
			/* write (graph->number, pattern id) pairs to stream, add the patterns to the bloom
			filter of the graph (i) for pruning */
			for (v=0; v<resultPos; ++v) {
				fprintf(keyValueStream, "%i %i\n", number, results[v].id);
				addToPruningSet(results[v].id, i);
			}
		}

		/* counting of read graphs and garbage collection */
		++i;
		dumpShallowGraphCycle(sgp, patterns);
	}
	if (results != NULL) {
		free(results);
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
			string = treeCenterCanonicalString(subPattern, sgp);
			pattern->vertices[v] = leaf;
			addEdge(e->startPoint, e);

			/* check if infrequent, if so return 1 otherwise continue */
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
		string = treeCenterCanonicalString(current, sgp);
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


void generateCandidateSetRec(struct Vertex* lowerLevel, struct Vertex* currentLevel, struct ShallowGraph* frequentEdges, struct Vertex* root, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if ((root->visited != 0) && (root != lowerLevel)) {
		/* at this point, we have found a pattern, we want to make a tree from it, get the refinements,
		filter interesting candidates and then scan the db of patterns  */
		struct Graph* pattern = canonicalString2Graph(prefix, gp);
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

struct ShallowGraph* edgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* result = getShallowGraph(sgp);
	struct VertexList* e;

	for (e=frequentEdges->neighborhood; e!=NULL; e=e->next) {
		struct Vertex* v = getVertex(gp->vertexPool);
		struct VertexList* f;
		v->label = e->label;
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
	return result;
}

/**
frequentEdgeShallowGraph is a list of VertexLists that point to Vertices
that are only accessible from there. These vertices have to be dumped, 
but there may be more than one vertexlist referencing a Vertex.
thus the algorithm has to find and store the first occurrence of each Vertex, 
dump them and just then dump the shallowgraph.
*/
void freeFrequentEdgeShallowGraph(struct GraphPool* gp, struct ShallowGraphPool* sgp, struct ShallowGraph* edges) {
	struct VertexList* e = edges->edges;
	struct Vertex* list = NULL;

	for (e=edges->edges; e!=NULL; e=e->next) {
		e->startPoint->d = e->endPoint->d = 1;
	}
	for (e=edges->edges; e!=NULL; e=e->next) {
		if (e->startPoint->d == 1) {
			e->startPoint->d = 0;
			e->startPoint->next = list;
			list = e->startPoint;
		}
		if (e->endPoint->d == 1) {
			e->endPoint->d = 0;
			e->endPoint->next = list;
			list = e->endPoint;
		}
	}
	while (list != NULL) {
		struct Vertex* next = list->next;
		dumpVertex(gp->vertexPool, list);
		list = next;
	}
	dumpShallowGraph(sgp, edges);
}


/**
Walk through the db, checking for each graph g \in db which refinements are subtrees of at least one of its spanning 
trees. for all these refinements, the visited counter of its cString in currentLevel is increased. 
*/
void scanDB(char* fileName, struct Vertex* currentLevel, struct Graph** refinements, struct Vertex** pointers, int n, int minGraph, int maxGraph, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int bufferSize = 100;
	int i = 0;
	FILE* stream = fopen(fileName, "r");
	struct ShallowGraph* spanningTreeStrings = NULL;
	int number;
	struct CachedGraph* subtreeCache = initCachedGraph(gp, 200);
	char* found = malloc(n * sizeof(char));
	
	/* iterate over all graphs in the database */
	while (((i < maxGraph) || (maxGraph == -1)) && (spanningTreeStrings = streamReadPatterns(stream, bufferSize, &number, sgp))) {
		if (i >= minGraph) {
			struct ShallowGraph* spanningTreeString;
			struct Graph* spanningTree = NULL;
			int refinement;
			int newBloomFilter = 0;

			/* if there is no frequent pattern from lower level contained in i, dont even start searching */
			if (pruning[i] != 0) {
				/* set d to one, meaning that all refinements have not yet been recognized to be subtree
				of current graph */
				for (refinement=0; refinement<n; ++refinement) {
					found[refinement] = 0;
				}
				/* for each spanning tree */
				for (spanningTreeString=spanningTreeStrings; spanningTreeString!=NULL; spanningTreeString=spanningTreeString->next) {
					/* convert streamed spanning tree string to graph */
					if (spanningTree != NULL) {
						int v;
						for (v=0; v<spanningTree->n; ++v) {
							dumpVertexListRecursively(gp->listPool, spanningTree->vertices[v]->neighborhood);
							spanningTree->vertices[v]->neighborhood = NULL;
						}
						canonicalString2ExistingGraph(spanningTreeString, spanningTree, gp);
					} else {
						spanningTree = canonicalString2Graph(spanningTreeString, gp);
					}
				
					/* for each refinement */
					for (refinement=0; refinement<n; ++refinement) {
						//if (containedInPruningSet(pointers[refinement]->d, i)) {
							/* if refinement is not already found to be subtree of current graph */
							if (!found[refinement]) {
								/* if refinement is contained in spanning tree */
								if (subtreeCheckCached(spanningTree, refinements[refinement], gp, subtreeCache)) {
									/* currentLevel refinementstring visited +1 and continue with next refinement */
									found[refinement] = 1;
									++pointers[refinement]->visited;
									++currentLevel->number;
									fprintf(keyValueStream, "%i %i\n", number, pointers[refinement]->lowPoint);
									newBloomFilter |= hashID(pointers[refinement]->lowPoint);
								}
							}
						//}
					}
				}
				pruning[i] = newBloomFilter;
				dumpGraph(gp, spanningTree);
			}
		}

		/* counting of read graphs and garbage collection */
		++i;
		dumpShallowGraphCycle(sgp, spanningTreeStrings);
	}
	dumpCachedGraph(subtreeCache);
	free(found);
	fclose(stream);
}


int makeGraphsAndPointers(struct Vertex* root, struct Vertex* current, struct Graph** patterns, struct Vertex** pointers, int i, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if ((current->visited != 0) && (current != root))  {
		/* TODO could also reset lowpoints here for speedup */
		patterns[i] = canonicalString2Graph(prefix, gp);
		pointers[i] = current;
		return i+1;
	}

	/* recursively access the subtree dangling from current */
	for (e=current->neighborhood; e!=NULL; e=e->next) {	
		/* after finishing this block, we want prefix to be as before, thus we have
			to do some list magic */
		struct VertexList* lastEdge = prefix->lastEdge;
		appendEdge(prefix, shallowCopyEdge(e, sgp->listPool));

		i = makeGraphsAndPointers(root, e->endPoint, patterns, pointers, i, prefix, gp, sgp);

		dumpVertexList(sgp->listPool, prefix->lastEdge);
		prefix->lastEdge = lastEdge;
		--prefix->m;

		if (prefix->m == 0) {
			prefix->edges = NULL;
		} else {
			lastEdge->next = NULL;
		}
	}
	return i;
}
