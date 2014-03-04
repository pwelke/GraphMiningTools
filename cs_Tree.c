#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "cs_Parsing.h"
#include "cs_Compare.h"
#include "cs_Tree.h"


/**
A canonical string in the sense of this module is a ShallowGraph containing a NULL 
terminated list of VertexLists. ->startPoint and ->endPoint of vertex lists are not
considered. Instead, only the ->label is important.
*/ 




/**********************************************************************************
 *************************** canonical strings for trees **************************
 **********************************************************************************/


int canonicalString2GraphRec(struct Graph* g, int current, int parent, struct ShallowGraph* suffix, struct VertexList* initEdge, struct VertexList* termEdge, struct GraphPool* gp) {

	/* for all children of parent */
	while (suffix->edges != NULL) {
		char* edgeLabel;

		if (strcmp(suffix->edges->label, termEdge->label) == 0) {
			suffix->edges = suffix->edges->next;
			return current;
		}
		
		/* first comes edge label */
		suffix->edges = suffix->edges->next;
		edgeLabel = suffix->edges->label;

		/* now comes the vertex label */
		suffix->edges = suffix->edges->next;
		g->vertices[current]->label = suffix->edges->label;

		addEdgeBetweenVertices(parent, current, edgeLabel, g, gp);

		/* recursive call */
		suffix->edges = suffix->edges->next;
		current = canonicalString2GraphRec(g, current + 1, current, suffix, initEdge, termEdge, gp);
	}

	/* return position of next free vertex in g */
	return current;
}


/**
Return a graph that belongs to the isomorphism class that is represented by pattern.
*/
struct Graph* treeCanonicalString2Graph(struct ShallowGraph* pattern, struct GraphPool* gp) {
	struct VertexList* initEdge = getInitialisatorEdge(gp->listPool);
	struct VertexList* termEdge = getTerminatorEdge(gp->listPool);
	struct VertexList* e;
	int n = 0;
	struct Graph* g;
	struct VertexList* head = pattern->edges;

	/* find number of vertices, create graph */
	for (e=pattern->edges; e!=NULL; e=e->next) {
		if (strcmp(e->label, initEdge->label) == 0) {
			++n;
		}
	}
	g = createGraph(n + 1, gp);

	/* create tree from canonical string recursively */
	g->vertices[0]->label = pattern->edges->label;
	pattern->edges = pattern->edges->next;
	canonicalString2GraphRec(g, 1, 0, pattern, initEdge, termEdge, gp);
	pattern->edges = head;

	/* cleanup */
	dumpVertexList(gp->listPool, initEdge);
	dumpVertexList(gp->listPool, termEdge);

	return g;
}


/**
Return a graph that belongs to the isomorphism class that is represented by pattern.
pattern is expected to have at most g->n vertices. g must not contain edges.
*/
void canonicalString2ExistingGraph(struct ShallowGraph* pattern, struct Graph* g, struct GraphPool* gp) {
	struct VertexList* initEdge = getInitialisatorEdge(gp->listPool);
	struct VertexList* termEdge = getTerminatorEdge(gp->listPool);
	struct VertexList* head = pattern->edges;

	/* create tree from canonical string recursively */
	g->vertices[0]->label = pattern->edges->label;
	pattern->edges = pattern->edges->next;
	canonicalString2GraphRec(g, 1, 0, pattern, initEdge, termEdge, gp);
	pattern->edges = head;

	/* cleanup */
	dumpVertexList(gp->listPool, initEdge);
	dumpVertexList(gp->listPool, termEdge);
}


/**
Given a tree $T$ and a vertex $ v \in V(T) $ this function returns
a string $ \pi (T) $ s.t. $ \pi (T) = \pi(T') $ iff $ T $ and $ T' $ are isomorphic.
 */
struct ShallowGraph* canonicalStringOfRootedTree(struct Vertex* vertex, struct Vertex* parent, struct ShallowGraphPool *p) {
	struct ShallowGraph *stringList = NULL;
	struct VertexList *idx;
	int numChildren = 0;

	/* for any neighbor...*/
	for (idx = vertex->neighborhood; idx; idx = idx->next) {
		/* ... that is not the parent (pointer comparison!) */
		if (idx->endPoint != parent) {
			struct ShallowGraph *tmp;

			/* copy edge that connects vertex and its child */
			struct VertexList* e = getVertexList(p->listPool);
			e->label = idx->label;

			/* compute canonical string recursively */
			tmp = canonicalStringOfRootedTree(idx->endPoint, vertex, p);

			/* add the edge connecting the child with vertex */
			tmp->edges = push(tmp->edges, e);


			/* add it to the list of strings for the children */
			tmp->next = stringList;
			stringList = tmp;
			++numChildren;
		}
	}

	/* if the list is empty, vertex is a leaf. this stops recursion */
	if (numChildren == 0) {
		struct ShallowGraph *result = getShallowGraph(p);
		struct VertexList *e = getVertexList(p->listPool);

		/* the canonical string of a single vertex consists of the label of
		that vertex */
		e->label = vertex->label;
		result->edges = push(result->edges, e);

		/* stop recursion */
		return result;
	} else {
		/* otherwise we have to sort the canonical strings in ascending order 
		therefore create array containing pointers to the strings */
		struct ShallowGraph** tempArray;
		if ((tempArray = malloc(numChildren * sizeof(struct ShallowGraph*)))) {
			int i;
			struct ShallowGraph *tmp;

			/* this will be the resulting canonical string */
			struct ShallowGraph* result = getShallowGraph(p);
			/* add an edge representing the label of the current vertex */
			struct VertexList* v = getVertexList(p->listPool);
			v->label = vertex->label;
			result->edges = push(result->edges, v);

			/* stdlib qsort requires an array of elements as input, so we will give it to it */
			tmp = stringList;

			for (i=0; i<numChildren; ++i) {
				tempArray[i] = tmp;
				tmp = tmp->next;
			}

			/*stdlib qsort using my lexicographic comparison of ShallowGraphs */
			qsort(tempArray, numChildren, sizeof(struct ShallowGraph*), &lexComp);

			/* append the edges of the substrings to the result */
			idx = result->edges;
			for (i=0; i<numChildren; ++i) {

				/* add open bracket as a substring begins */
				idx->next = getInitialisatorEdge(p->listPool);
				idx = idx->next;

				/* append canonical string of the current child */ 
				idx->next = tempArray[i]->edges;

				/* the edges do not belong to this element any longer.
				This is important as dumping otherwise would dump the edges, too */
				tempArray[i]->edges = NULL;
				dumpShallowGraph(p, tempArray[i]);

				/* go as long as we are not at the end of the list */

				/*********************************************************
				 ** TODO  Use skip pointers to avoid quadratic efford  **
				 *********************************************************/
				for (; idx->next; idx = idx->next);

				/* add close bracket as substring ends */
				idx->next = getTerminatorEdge(p->listPool);
				idx = idx->next;
			}

			free(tempArray);
			return result;

		} else {
			printf("canonicalStringOfRootedTree: Error allocating memory for array of neighbors");
			return NULL;
		}
	}

}


/**
Given a tree $T$ and a vertex $ v \in V(T) $ this function returns
a string $ \pi (T) $ s.t. $ \pi (T) = \pi(T') $ iff $ T $ and $ T' $ are isomorphic.

This method computes the canonical string of the maximal subtree (beginning at v)
where for each vertex w: 0 < w->visited <= maxDepth.
 */
struct ShallowGraph* canonicalStringOfRootedLevelTree(struct Vertex* vertex, struct Vertex* parent, int maxDepth, struct ShallowGraphPool *p) {
	struct ShallowGraph *stringList = NULL;
	struct VertexList *idx;
	int numChildren = 0;

	/* for any neighbor...*/
	for (idx = vertex->neighborhood; idx; idx = idx->next) {
		/* ... that is not the parent (pointer comparison!) */
		if (idx->endPoint != parent) {
			/* ... and has distance smaller than maxDepth */
			if (idx->endPoint->visited && (idx->endPoint->visited <= maxDepth)) {
				struct ShallowGraph *tmp;

				/* copy edge that connects vertex and its child */
				struct VertexList* e = getVertexList(p->listPool);
				e->label = idx->label;

				/* compute canonical string recursively */
				tmp = canonicalStringOfRootedLevelTree(idx->endPoint, vertex, maxDepth, p);

				/* add the edge connecting the child with vertex */
				pushEdge(tmp, e);

				/* add it to the list of strings for the children */
				tmp->next = stringList;
				stringList = tmp;
				++numChildren;
			}
		}
	}

	/* if the list is empty, vertex is a leaf. this stops recursion */
	if (numChildren == 0) {
		struct ShallowGraph *result = getShallowGraph(p);
		struct VertexList *e = getVertexList(p->listPool);

		/* the canonical string of a single vertex consists of the label of
		that vertex */
		e->label = vertex->label;
		pushEdge(result, e);

		/* stop recursion */
		return result;
	} else {
		/* otherwise we have to sort the canonical strings in ascending order
		therefore create array containing pointers to the strings */
		struct ShallowGraph** tempArray;
		if ((tempArray = malloc(numChildren * sizeof(struct ShallowGraph*)))) {
			int i;
			struct ShallowGraph *tmp;

			/* this will be the resulting canonical string */
			struct ShallowGraph* result = getShallowGraph(p);
			/* add an edge representing the label of the current vertex */
			struct VertexList* v = getVertexList(p->listPool);
			v->label = vertex->label;
			pushEdge(result, v);

			/* stdlib qsort requires an array of elements as input, so we will give it to it */
			tmp = stringList;

			for (i=0; i<numChildren; ++i) {
				tempArray[i] = tmp;
				tmp = tmp->next;
			}

			/*stdlib qsort using my lexicographic comparison of ShallowGraphs */
			qsort(tempArray, numChildren, sizeof(struct ShallowGraph*), &lexComp);

			/* append the edges of the substrings to the result */
			idx = result->edges;
			for (i=0; i<numChildren; ++i) {

				/* add open bracket as a substring begins */
				idx->next = getInitialisatorEdge(p->listPool);
				idx = idx->next;

				/* append canonical string of the current child */
				idx->next = tempArray[i]->edges;

				/* the edges do not belong to this element any longer.
				This is important as dumping otherwise would dump the edges, too */
				tempArray[i]->edges = NULL;
				dumpShallowGraph(p, tempArray[i]);

				/* go as long as we are not at the end of the list */

				/*********************************************************
				 ** TODO  Use skip pointers to avoid quadratic efford  **
				 *********************************************************/
				for (; idx->next; idx = idx->next);

				/* add close bracket as substring ends */
				idx->next = getTerminatorEdge(p->listPool);
				idx = idx->next;
			}

			free(tempArray);
			return result;

		} else {
			fprintf(stderr, "canonicalStringOfRootedTree: Error allocating memory for array of neighbors");
			return NULL;
		}
	}

}


/**
 * Computes the canonical string of the (free) level tree of depth maxDepth rooted at v.
 */
struct ShallowGraph* canonicalStringOfLevelTree(struct ShallowGraph* vertexList, int maxDepth, struct ShallowGraphPool* sgp) {

	struct VertexList* idx;
	struct ShallowGraph* canonicalString = NULL;

	/* compute the canonical string of the unique tree rooted at vertex i and save it at the correct position of the
	canonicalStrings array */
	for (idx=vertexList->edges; idx && (idx->endPoint->visited <= maxDepth); idx=idx->next) {

		/* compute the canonical string */
		struct ShallowGraph *cs = canonicalStringOfRootedLevelTree(idx->endPoint, NULL, maxDepth, sgp);

		/* keep the string if its the first one, computed for this component or lexicograhically smaller
			than the saved string */
		if (canonicalString) {
			if (lexicographicComparison(canonicalString, cs) > 0) {
				dumpShallowGraph(sgp, canonicalString);
				canonicalString = cs;
			} else {
				dumpShallowGraph(sgp, cs);
			}
		} else {
			canonicalString = cs;
		}

	}

	return canonicalString;
}



/**
Traverses a graph and marks all vertices reachable from v with the number given
by the argument component.
@refactor: to dfs.c
 */
void markConnectedComponents(struct Vertex *v, int component) {
	struct VertexList *index;

	/* mark vertex as visited */
	v->visited = component;

	/*recursive call for all neighbors that are not visited so far */
	for (index = v->neighborhood; index; index = index->next) {
		if (!(index->endPoint->visited)) {
			markConnectedComponents(index->endPoint, component);
		}
	}
}


/**
Takes the output of partitionIntoForestAndCycles() and returns all canonical strings and returns a 
ShallowGraph list of canonical strings for the contained trees */
struct ShallowGraph* getTreePatterns(struct Graph* forest, struct ShallowGraphPool *sgp) {
	/* components contains the number of trees in the forest after the first for loop */
	int components = 0;
	int i;
	struct ShallowGraph **canonicalStrings;
	struct ShallowGraph* result = NULL;

	/* mark every tree with a different number */
	for (i=0; i<forest->n; ++i) {
		if (forest->vertices[i]->visited == 0) {
			++components;
			markConnectedComponents(forest->vertices[i], components);
		}
	}

	/* for each component, we will have a list of possible canonical strings */
	if (!(canonicalStrings = malloc(components * sizeof(struct ShallowGraph*)))) {
		printf("getTreePatterns: Error allocating memory for canonical string array for %i components.\n", components);
		return NULL;
	} else {

		/* initialize the thing to NULL */
		for (i=0; i<components; ++i) {
			canonicalStrings[i] = NULL;
		}

		/* compute the canonical string of the unique tree rooted at vertex i and save it at the correct position of the 
		canonicalStrings array */
		for (i=0; i<forest->n; ++i) {
			/* readability: the label of the current tree minus 1 for indexing purposes */
			int currentComponent = forest->vertices[i]->visited - 1;
			/* compute the canonical string */
			struct ShallowGraph *cs = canonicalStringOfRootedTree(forest->vertices[i], NULL, sgp);

			/* keep the string if its the first one, computed for this component or lexicograhically smaller 
			than the saved string */
			if (canonicalStrings[currentComponent]) {
				if (lexicographicComparison(canonicalStrings[currentComponent], cs) > 0) {
					dumpShallowGraph(sgp, canonicalStrings[currentComponent]);
					canonicalStrings[currentComponent] = cs;
				} else {
					dumpShallowGraph(sgp, cs);
				}
			} else {
				canonicalStrings[currentComponent] = cs;
			}
		}


		/* concatenate the elements referred to in the array to a list, free the array */
		for (i=0; i<components-1; ++i) {
			canonicalStrings[i]->next = canonicalStrings[i+1];
		}
		canonicalStrings[components-1]->next = NULL;
		result = canonicalStrings[0];
		free(canonicalStrings);
	}
	return result;
}
