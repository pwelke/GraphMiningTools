#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "treeCenter.h"
#include "cachedGraph.h"
#include "subtreeIsomorphism.h"
#include "cs_Parsing.h"
#include "cs_Tree.h"
#include "searchTree.h"
#include "bloomFilter.h"
#include "levelwiseTreePatternMining.h"

static struct VertexList* getCanonicalStringOfEdge(struct VertexList* e, struct ListPool* lp) {
	struct VertexList* cString;
	if (strcmp(e->startPoint->label, e->endPoint->label) < 0) {
		/* cString = v e (w) */
		struct VertexList* tmp = getVertexList(lp);
		tmp->label = e->endPoint->label;

		cString = getTerminatorEdge(lp);
		tmp->next = cString;

		cString = getVertexList(lp);
		cString->label = e->label;
		cString->next = tmp;

		tmp = getInitialisatorEdge(lp);
		tmp->next = cString;

		cString = getVertexList(lp);
		cString->label = e->startPoint->label;
		cString->next = tmp;
	} else {
		/* cString = w e (v) */
		struct VertexList* tmp = getVertexList(lp);
		tmp->label = e->startPoint->label;

		cString = getTerminatorEdge(lp);
		tmp->next = cString;

		cString = getVertexList(lp);
		cString->label = e->label;
		cString->next = tmp;

		tmp = getInitialisatorEdge(lp);
		tmp->next = cString;

		cString = getVertexList(lp);
		cString->label = e->endPoint->label;
		cString->next = tmp;
	}
	return cString;
}


/**
return the histogram of vertices and edges in a db in a search tree.
traverses the db once.

the db is expected to have the format outputted by printStringsInSearchTree().
both output parameters (frequent* ) should be initialized to "empty" structs.
fileName specifies the file the db is contained in.

It uses a resultSet struct speed up some stuff and fills up the pruning data structure.

The method expects that initPruning was called with a positive argument before and returns 
the number of graphs in the database.
*/
int getVertexAndEdgeHistograms(char* fileName, double importance, struct Vertex* frequentVertices, struct Vertex* frequentEdges, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int bufferSize = 100;
	int i = 0;
	FILE* stream = fopen(fileName, "r");
	struct ShallowGraph* patterns = NULL;
	int number;
	struct compInfo* results = NULL;
	int resultSize = 0;
	
	/* iterate over all graphs in the database */
	while ((patterns = streamReadPatterns(stream, bufferSize, &number, sgp))) {

		struct ShallowGraph* pattern = patterns;
		struct Graph* patternGraph;
		int numberOfPatterns = 0;
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
		patternGraph = treeCanonicalString2Graph(pattern, gp);
		for (v=0; v<patternGraph->n; ++v) {
			/* See commented out how it would look if done by the book.
			However, this has to be fast and canonicalStringOfTree has
			too much overhead!
			    struct ShallowGraph* cString;
			    auxiliary->vertices[0]->label = patternGraph->vertices[v]->label;
			    cString = canonicalStringOfTree(auxiliary, sgp);
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
			++numberOfPatterns;
			if (patternGraph == NULL) {
				patternGraph = treeCanonicalString2Graph(pattern, gp);
			}
			for (v=0; v<patternGraph->n; ++v) {
				struct VertexList* e;
				for (e=patternGraph->vertices[v]->neighborhood; e!=NULL; e=e->next) {
					int w = e->endPoint->number;
					/* edges occur twice in patternGraph. just add them once to the search tree */
					if (w > v) {
						/* as for vertices, I use specialized code to generate 
						the canonical string of a single edge */
						struct VertexList* cString = getCanonicalStringOfEdge(e, gp->listPool);
						/* add the string to the search tree */
						containedEdges->d += addStringToSearchTree(containedEdges, cString, gp);
						containedEdges->number += 1;
					} 
				}
			}
			dumpGraph(gp, patternGraph);
			patternGraph = NULL;
		}
		// TODO filtering of patterns
		if (importance < 1) {
			int threshold = (int)(importance * numberOfPatterns);
			filterSearchTreeLEQ(frequentEdges, threshold, frequentEdges, gp);
		} else {
			filterSearchTree(frequentEdges, numberOfPatterns, frequentEdges, gp);
		}
		
		/* set multiplicity of patterns to 1 and add to global edge pattern set */
		resetToUnique(containedEdges);
		mergeSearchTrees(frequentEdges, containedEdges, 1, results, &resultPos, frequentEdges, 0, gp);
		dumpSearchTree(gp, containedEdges);

		/* write (graph->number, pattern id) pairs to stream, add the patterns to the bloom
		filter of the graph (i) for pruning */
		for (v=0; v<resultPos; ++v) {
			fprintf(keyValueStream, "%i %i\n", number, results[v].id);
			initialAddToPruningSet(results[v].id, i);
		}
		

		/* counting of read graphs and garbage collection */
		++i;
		dumpShallowGraphCycle(sgp, patterns);
	}
	if (results != NULL) {
		free(results);
	}
	fclose(stream);
	return i;
}


/**
Create a list of single-edge ShallowGraphs from a search tree containing single edge canonical strings.
Note that this method creates a hardcopy of the edges, strings and vertices. 
Hence, it requires a pointer to a struct Graph* newVertices variable where it stores a newly created graph that holds all the new vertices. 
To avoid memory leaks, this graph needs to be dumbed together with the struct ShallowGraph* result of this method.
*/
struct ShallowGraph* edgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct Graph** newVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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


/** Check for each pattern tree in patternTrees if there is at least one tree in transactionTrees such that 
the pattern is subgraph isomorphic to the transaction tree */
int checkIfSubIso(struct ShallowGraph* transactionTrees, struct Graph** patternTrees, 
					int i, int n, int** features, struct Vertex** pointers, struct GraphPool* gp) {
	struct ShallowGraph* spanningTreeString;
	int currentLevelNumber = 0;
	// struct CachedGraph* subtreeCache = initCachedGraph(gp, 200);
	
	/* if there is no frequent pattern from lower level contained in i, dont even start searching */
	if (!isEmpty(i)) {
		/* set d to one, meaning that all patternTrees have not yet been recognized to be subtree
		of current graph */
		/* for each spanning tree */
		for (spanningTreeString=transactionTrees; spanningTreeString!=NULL; spanningTreeString=spanningTreeString->next) {
			/* convert streamed spanning tree string to graph */
			struct Graph* spanningTree = treeCanonicalString2Graph(spanningTreeString, gp);
			
			/* for each pattern */
			int pattern;
			for (pattern=0; pattern<n; ++pattern) {
				if (isSubset(pointers[pattern]->d, i)) {
					/* if pattern is not already found to be subtree of current graph */
					if (!features[i][pattern]) {
						/* if pattern is contained in spanning tree */
						// if (subtreeCheckCached(spanningTree, patternTrees[pattern], gp, subtreeCache)) {
						if (subtreeCheck3(spanningTree, patternTrees[pattern], gp)) {
							/* currentLevel patternstring visited +1 and continue with next pattern */
							features[i][pattern] = 1;
							++pointers[pattern]->visited;
							++currentLevelNumber;
						} else {
							features[i][pattern] = 0;
						}
					}
				}
			}
			dumpGraph(gp, spanningTree);
		}
	}
	// dumpCachedGraph(subtreeCache);
	return currentLevelNumber;
}

/** Make checkIfSubIso compatible to the signature required by scanDBNoCache */
int checkIfSubIsoCompatible(struct ShallowGraph* transactionTrees, struct Graph** patternTrees, double fraction, 
							int i, int n, int** features, struct Vertex** pointers, struct GraphPool* gp) {
	if (fraction) {
		/* forget about it and hide warning */
	}
	return checkIfSubIso(transactionTrees, patternTrees, i, n, features, pointers, gp);
}


/**
 * Given some transaction trees (spanningTreeStrings), and some pattern trees (refinements), check for each 
 * refinement if it is a subtree of at least fraction of the transaction trees. 
 */
int checkIfImportantSubIso(struct ShallowGraph* transactionTrees, struct Graph** patternTrees, double fraction, 
							int i, int n, int** features, struct Vertex** pointers, struct GraphPool* gp) {
	struct ShallowGraph* spanningTreeString;
	int currentLevelNumber = 0;
	// struct CachedGraph* subtreeCache = initCachedGraph(gp, 200);

	
	/* if there is no frequent pattern from lower level contained in i, dont even start searching */
	if (!isEmpty(i)) {
		int pattern;

		/* convert streamed spanning tree strings to graph */
		struct Graph* spanningTrees = NULL;
		int weightedNTransactions = 0;
		for (spanningTreeString=transactionTrees; spanningTreeString!=NULL; spanningTreeString=spanningTreeString->next) {
			struct Graph* tmp = treeCanonicalString2Graph(spanningTreeString, gp);
			tmp->number = spanningTreeString->data; /* streamReadPatterns..() stores the multiplicity of the pattern there */
			weightedNTransactions += spanningTreeString->data;
			tmp->next = spanningTrees;
			spanningTrees = tmp;
		}

		for (pattern=0; pattern<n; ++pattern) {
			int count = 0;
			struct Graph* spanningTree;
			for (spanningTree=spanningTrees; spanningTree!=NULL; spanningTree=spanningTree->next) {
				if (isSubset(pointers[pattern]->d, i)) {
					/* if pattern is contained in spanning tree */
					// if (subtreeCheckLF(spanningTree, patternTrees[pattern], gp, sgp)) {
					// if (subtreeCheckCached(spanningTree, patternTrees[pattern], gp, subtreeCache)) {
					if (subtreeCheck3(spanningTree, patternTrees[pattern], gp)) {
						/* weight the found match with the multiplicity 
						of the (spanning) tree in the original graph */
						if (spanningTree->number == 0) {
							/* should not happen */
							++count;
							fprintf(stderr, "Multiplicity of spanning tree is zero. Not good!\n");
						} else {
							count += spanningTree->number;
							// fprintf(stderr, "multiplicity %i\n", spanningTree->number);
						}	
					}
				}	 
			}
			/* if we cross the frequency threshold, we mark the pattern as matched and update 
			 * bookkeeping information accordingly. This must be done exactly once */ 
			// fprintf(stderr, "fraction %lf nTransactions %i count %i c>=f*n %i\n", fraction, weightedNTransactions, count, (count >= fraction * weightedNTransactions));
			if ((count > fraction * weightedNTransactions) || ((fraction == 1.0) && (count == weightedNTransactions))) { 
				features[i][pattern] = 1;
				++pointers[pattern]->visited;
				++currentLevelNumber;
			}
		}
		dumpGraphList(gp, spanningTrees);
	} 
	// dumpCachedGraph(subtreeCache);
	return currentLevelNumber;
}


/**
Walk through the db, checking for each graph g \in db which refinements are subtrees of at least one of its spanning 
trees. for all these refinements, the visited counter of its cString in currentLevel is increased. 
*/
void scanDBNoCache(char* fileName, struct Vertex* currentLevel, struct Graph** refinements, 
					struct Vertex** pointers, int n, int threshold, int nGraphs,
					double fraction, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp,
					int (*embeddingOperator)(struct ShallowGraph*, struct Graph**, double, int, int, int**, struct Vertex**, struct GraphPool*)) {
	int bufferSize = 100;
	int i = 0;
	FILE* stream = fopen(fileName, "r");
	struct ShallowGraph* spanningTreeStrings = NULL;
	int number;
	int spanningTreeCount;

	int** features = malloc(nGraphs * sizeof(int*));
	for (i=0; i<nGraphs; ++i) {
		features[i] = malloc((n + 1) * sizeof(int));
	}
	i = 0;
	
	/* iterate over all graphs in the database */
	while ((spanningTreeStrings = streamReadPatternsAndTheirNumber(stream, bufferSize, &number, &spanningTreeCount, sgp))) {
		int refinement;

		/* reinit the feature array */
		features[i][n] = number;
		if (features[i][n] == 0) {
			fprintf(stderr, "Reading error. 0 was read\n");
		}
		for (refinement=0; refinement<n; ++refinement) {
			features[i][refinement] = 0;
		}

		/* the pattern matching operator */
		currentLevel->number += embeddingOperator(spanningTreeStrings, refinements, fraction, i, n, features, pointers, gp);

		/* counting of read graphs and garbage collection */
		++i;
		dumpShallowGraphCycle(sgp, spanningTreeStrings);
	}
	fclose(stream);

	/* pruning and output of frequent patterns */ 
	// processedGraphs = i; /* only loop through graphs that were processed */
	// fprintf(stderr, "mingraph=%i nGraphs=%i\n", minGraph, nGraphs);
	for (i=0; i<nGraphs; ++i) {
		int refinement;
		resetPruningSet(i);
		for (refinement=0; refinement<n; ++refinement) {
			if ((features[i][refinement] == 1) && (pointers[refinement]->visited >= threshold)) {
				fprintf(keyValueStream, "%i %i\n", features[i][n], pointers[refinement]->lowPoint);
				addToPruningSet(pointers[refinement]->lowPoint, i);
			}
		}
	}

	for (i=0; i<nGraphs; ++i) {
		free(features[i]);
	}
	free(features);
}


/**
 * Init patterns and pointers data structures for constant time access to the strings in a searchtree.
 * Input: root = current = root of search tree containing canonical strings of trees.
 * Output:
 *   patterns is an array of Graph structs corresponding to the canonical strings stored in the search tree
 *       ordered by dfs in the search tree.
 *   pointers is an array of the same size of vertex structs holding pointers to the vertices in the search
 *       tree where the canonical strings terminate. This yields constant time access to the information that
 *       is/needs to be stored there. Please refer to the documentation in searchTree.c for further information.
 */
int makeGraphsAndPointers(struct Vertex* root, struct Vertex* current, struct Graph** patterns, struct Vertex** pointers, int i, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if ((current->visited != 0) && (current != root))  {
		/* TODO could also reset lowpoints here for speedup */
		patterns[i] = treeCanonicalString2Graph(prefix, gp);
		pointers[i] = current;
		++i; // for search trees that contain only strings of the same length, we could return i+1 here. 
		     // In general, we need to check if there are children at the current vertex
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
