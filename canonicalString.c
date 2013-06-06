#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "graph.h"
#include "canonicalString.h"


/** canonicalString.c */


/** Maybe I can implement the whole string things without strings and just with the search tree idea. 
Let's try that. Every function with something like canonical string in its name returns a path. */



/**********************************************************************************
 *************************** canonical strings for trees **************************
 **********************************************************************************/


/**
 * Wrapper method for use in qsort
 */
int lexComp(const void* e1, const void* e2) {

	struct ShallowGraph* g = *((struct ShallowGraph**)e1);
	struct ShallowGraph* h = *((struct ShallowGraph**)e2);

	return lexicographicComparison(g, h);
}


/**
Compare two strings represented by ShallowGraphsto each other. Return behaviour should match that
of string.h's strcmp() function */
int lexicographicComparison(const struct ShallowGraph *g1, const struct ShallowGraph *g2) {
	return compareVertexLists(g1->edges, g2->edges);
}


/**
Computes the lexicographic order of two rooted paths given by their roots \verb+ r1+ and \verb+ r2+ recursively. 
This function returns
\[ -1 if P1 < P2 \]
\[ 0 if P1 = P2 \]
\[ 1 if P1 > P2 \]
and uses the comparison of label strings as total ordering.
The two paths are assumed to have edge labels and vertex labels are not taken into account.
 */
int compareVertexLists(const struct VertexList* e1, const struct VertexList* e2) {

	/* if this value is larger than 0 the first label is lex. larger than the second etc. */
	int returnValue = strcmp(e1->label, e2->label);

	/* if the two paths are identical so far wrt. labels check the next vertex on each path */
	if (returnValue == 0) {

		if ((e1->next) && (e2->next)) {
			/* if both lists have a next, proceed recursively */
			return compareVertexLists(e1->next, e2->next);
		} else {
			/* if the first list is shorter, its lex. smaller */
			if ((e1->next == NULL) && (e2->next)){
				return -1;
			}
			/* if the second is shorter, this one is lex.smaller */
			if ((e2->next == NULL) && (e1->next)){
				return 1;
			}
		}

		/* if none of the cases above was chosen, both paths end here and we have to return 0 as both strings
		represented by the lists are identical */
		return 0;

	} else {
		/* if the paths differ in the current vertices, we return the return value of the comparison function */
		return returnValue;
	} 
}


/**
Returns a VertexList object that represents the end of a canonical String.
Currently the ) sign is used for that.
 */
struct VertexList* getTerminatorEdge(struct ListPool *p) {
	struct VertexList* e = getVertexList(p);
	e->label = malloc(2*sizeof(char));
	sprintf(e->label, ")");
	e->isStringMaster = 1;
	return e;
}


/**
Returns a VertexList object that represents the beginning of a canonical String.
Currently the ( sign is used for that.
 */
struct VertexList* getInitialisatorEdge(struct ListPool *p) {
	struct VertexList* e = getVertexList(p);
	e->label = malloc(2*sizeof(char));
	sprintf(e->label, "(");
	e->isStringMaster = 1;
	return e;
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



/**********************************************************************************
 *************************** canonical strings for cycles *************************
 **********************************************************************************/


/**
 * If you want to compare two cycles by yourself, use compareShallowGraphCycles() instead.
 *
 * Compare two cycles of equal length. Caution, the input parameters point to the edges
 * BEFORE the edges starting the cycle.
 * This is due to the usage of this method in permutateCycle().
 */
struct VertexList* compareCycles(struct VertexList* c1, struct VertexList* c2) {
	struct VertexList* idx1 = c1;
	struct VertexList* idx2 = c2;

	/* check for differences in the following edges */
	for (idx1=idx1->next, idx2=idx2->next; idx1!=c1; idx1=idx1->next, idx2=idx2->next) {

		/* check for differences in the edge label */
		if (strcmp(idx1->label, idx2->label) < 0) {
			return c1;
		}
		if (strcmp(idx1->label, idx2->label) > 0) {
			return c2;
		}
		if (strcmp(idx1->label, idx2->label) == 0) {
			/* if edge labels do not differ, check for differences in endPoint label */
			if (strcmp(idx1->endPoint->label, idx2->endPoint->label) < 0) {
				return c1;
			}
			if (strcmp(idx1->endPoint->label, idx2->endPoint->label) > 0) {
				return c2;
			}
		}
	}

	/* check for differences in the edge label of the first edge*/
	if (strcmp(idx1->label, idx2->label) < 0) {
		return c1;
	}
	if (strcmp(idx1->label, idx2->label) > 0) {
		return c2;
	}
	if (strcmp(idx1->label, idx2->label) == 0) {
		/* if edge labels do not differ, check for differences in endPoint label */
		if (strcmp(idx1->endPoint->label, idx2->endPoint->label) < 0) {
			return c1;
		}
		if (strcmp(idx1->endPoint->label, idx2->endPoint->label) > 0) {
			return c2;
		}
	}

	/* if no difference is found, return c1 */
	return c1;
}


/**
 * Compares two ShallowGraphs representing cycles of the same length and returns the one
 * resulting in the lexicographically smaller canonical string
 */
struct ShallowGraph* compareShallowGraphCycles(struct ShallowGraph* c1, struct ShallowGraph* c2) {
	struct VertexList* smallest;

	c1->lastEdge->next = c1->edges;
	c2->lastEdge->next = c2->edges;

	/* see doc of compareCycles for explanation of this call */
	smallest = compareCycles(c1->lastEdge, c2->lastEdge);

	c1->lastEdge->next = NULL;
	c2->lastEdge->next = NULL;

	return (smallest == c1->lastEdge) ? c1 : c2;

}


/**
 * Finds the cyclic permutation that results in a lexicographically smallest
 * representation of the input cycle, if one considers the canonical string beginning
 * with the edge label of the first edge in the ->edge list followed by the vertex label
 * of its endpoint etc.
 */
struct ShallowGraph* permutateCycle(struct ShallowGraph* cycle) {
	struct VertexList* start = cycle->edges;
	struct VertexList* end = assertLastPointer(cycle);
	/* best points to the edge before the starting edge of the lex smallest cycle */
	struct VertexList* best = cycle->edges;
	struct VertexList* idx;

	/* make actual cycle */
	end->next = start;

	for (idx = start->next; idx != start; idx=idx->next) {
		best = compareCycles(best, idx);
	}

	/* turn cycle s.t. the shallow graph is lex. smallest */
	cycle->edges = best->next;
	cycle->lastEdge = best;
	best->next = NULL;

	return cycle;
}

/**
 * Returns the canonical string of a cycle. This method consumes the cycle.
 */
void cycleToString(struct ShallowGraph* cycle, struct ShallowGraphPool* sgp) {
	struct VertexList* idx, *tmp;
	struct ShallowGraph* tmpString = getShallowGraph(sgp);

	appendEdge(tmpString, getInitialisatorEdge(sgp->listPool));

	for (idx=cycle->edges; idx; idx=tmp) {
		struct VertexList* endPoint = getVertexList(sgp->listPool);
		endPoint->label = idx->endPoint->label;

		tmp = idx->next;

		appendEdge(tmpString, idx);
		appendEdge(tmpString, endPoint);
	}

	appendEdge(tmpString, getTerminatorEdge(sgp->listPool));

	/* move all to cycle */
	cycle->edges = tmpString->edges;
	cycle->lastEdge = tmpString->lastEdge;
	cycle->m = tmpString->m;
	/* ->next and ->prev remain */

	/* dump tmpString */
	tmpString->edges = NULL;
	dumpShallowGraph(sgp, tmpString);
}


/**
 *  returns the canonical string of the input cycle without consuming the cycle
 */
struct ShallowGraph* getCStringOfCycle(struct ShallowGraph* cycle, struct ShallowGraphPool* sgp) {
	struct VertexList* idx;
	struct ShallowGraph* tmpString = getShallowGraph(sgp);

	appendEdge(tmpString, getInitialisatorEdge(sgp->listPool));

	for (idx=cycle->edges; idx; idx=idx->next) {
		struct VertexList* endPoint = getVertexList(sgp->listPool);
		endPoint->label = idx->endPoint->label;

		appendEdge(tmpString, shallowCopyEdge(idx, sgp->listPool));
		appendEdge(tmpString, endPoint);
	}

	appendEdge(tmpString, getTerminatorEdge(sgp->listPool));

	return tmpString;
}


/**
 * Process a list of ShallowGraphs representing cycles and returns a list of canonical strings
 * for these cycles. The input is consumed.
 */
struct ShallowGraph* getCyclePatterns(struct ShallowGraph* cycles, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* idx;
	struct ShallowGraph* result = NULL;
	struct ShallowGraph* next;

	for (idx = cycles; idx; idx=next) {
		struct ShallowGraph* tmpResult1;
		struct ShallowGraph* rev;

		next = idx->next;

		tmpResult1 = result;
		idx = permutateCycle(idx);
		rev = permutateCycle(inverseCycle(idx, sgp));

		result = compareShallowGraphCycles(idx, rev);
		result->next = tmpResult1;

		/* dump the larger of the two cycles */
		(result == idx) ? dumpShallowGraph(sgp, rev) : dumpShallowGraph(sgp, idx);

		/* convert the cycle to its canonical string */
		cycleToString(result, sgp);

	}

	return result;
}



/**************************************************************************************
 ******************************** Search Tree *****************************************
 **************************************************************************************/



/**
 * Recursively add a string represented by a list of edges to a search tree given by its root.
 * The string is consumed
 * This method DOES NOT INCREASE the current number of strings contained in the search tree.
 * You have to maintain the correct number of strings in the search tree yourself.
 */
void addStringToSearchTree(struct Vertex* root, struct VertexList* edge, struct GraphPool* p) {

	/* if edge == NULL, stop recursion, remember, that some string ends here */
	if (edge == NULL) {
		root->visited += 1;
	} else {
		struct VertexList* idx;
		for (idx=root->neighborhood; idx; idx=idx->next) {
			/* if the next label is already in the tree, continue recursively */
			if (strcmp(idx->label, edge->label) == 0) {
				addStringToSearchTree(idx->endPoint, edge->next, p);
				/* edges dangling at edge are consumed or dumped by the following recursion steps */
				edge->next = NULL;
				dumpVertexList(p->listPool, edge);
				return;
			}
		}

		/* otherwise add the current edge to the tree and continue recursively */
		idx = edge->next;
		edge->startPoint = root;
		edge->endPoint = getVertex(p->vertexPool);
		addEdge(root, edge);
		addStringToSearchTree(edge->endPoint, idx, p);
	}
}


/**
This function builds a search tree likeish structure given a NULL terminated
list of ShallowGraphs. The list is consumed and dumped.

The resulting structure is a tree rooted at the return value where a path from the root
to some vertex v in the tree represents v->visited many strings.

 */
struct Vertex* buildSearchTree(struct ShallowGraph* strings, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph *idx;
	struct Vertex* root = getVertex(gp->vertexPool);

	for (idx=strings; idx; idx=idx->next) {
		addStringToSearchTree(root, idx->edges, gp);
		root->number += 1;
		idx->edges = NULL;
	}

	dumpShallowGraphCycle(sgp, strings);

	return root;
}


/**
 * returns an uninitialized result vector of size n
 * used solely for storing results of mergeSearchTree
 * TODO some error, if there is not enough memory
 */
struct compInfo* getResultVector(int n) {

	return malloc(n * sizeof(struct compInfo));
}


/**
 * Wrapper method for use in qsort. compares the ids of two compInfo
 * structs
 *
 * This function returns the difference e1->id - e2->id thus fulfilling
 * the requirements of qsort.
 */
int compInfoComparison(const void* e1, const void* e2) {
	/* TODO move into function call */
	struct compInfo a = *(struct compInfo*)e1;
	struct compInfo b = *(struct compInfo*)e2;

	return a.id - b.id;
}


/*
 * updates search tree structure.
 * labels of strings are stored in edges, the labels are hardcopied to allow global Index construction
 * the endPoint vertex of an edge stores:
 * - possible extensions of the string in ->neighborhood
 * - the number of strings that are represented by the path from the root of the tree
 * 		to this vertex in ->visited, localTree->visited elements are divided by divisor
 * 		(use e.g. 2 for including cycles, as they are listed twice)
 * - the tree-wide unique id of the string, or 0 if no particular string ends there
 * 		in ->lowPoint
 */
void mergeSearchTrees(struct Vertex* globalTree, struct Vertex* localTree, int divisor, struct compInfo* results, int* pos, struct Vertex* trueRoot, int depth, struct GraphPool* p) {


	/* process current vertex */

	/* if some string ends here, write it to the output array and update data structure */
	if (localTree->visited) {

		/* add current number of elements (divided by divisor) to global number */
		globalTree->visited += localTree->visited / divisor;

		/* if the current string has no global id yet, get the next one */
		if (globalTree->lowPoint == 0) {
			trueRoot->lowPoint++;
			globalTree->lowPoint = trueRoot->lowPoint;
		}

		/* if we have a results array, store the (id, count)-pair in the array */
		if (results) {

			results[*pos].id = globalTree->lowPoint;
			results[*pos].count = localTree->visited / divisor;
			results[*pos].depth = depth;
			*pos += 1;

		}

	}

	/* if the current vertex is no leaf, merge search trees recursively */
	if (localTree->neighborhood) {

		struct VertexList* locNb;

		/* TODO this is the ugliest, most inefficient stuff */

		/* process neighbors */
		for (locNb=localTree->neighborhood; locNb; locNb=locNb->next) {
			struct VertexList* globNb;
			char found = 0;

			for (globNb=globalTree->neighborhood; globNb; globNb=globNb->next) {

				/* if the next label is already in the tree, continue recursively */
				if (strcmp(globNb->label, locNb->label) == 0) {
					mergeSearchTrees(globNb->endPoint, locNb->endPoint, divisor, results, pos, trueRoot, depth+1, p);
					found = 1;
					break;
				}
			}

			if (!found) {

				/* we reach this point, iff locNb->label is not found in the labels of the global neigbors
				 * thus we have to add the edge and the vertex to the globalTree. This copying has to be a real
				 * copy, no shallow, as any label or vertex that the local tree refers to is dumped when
				 * processing of the current vertex is done.
				 */
				globNb = getVertexList(p->listPool);
				globNb->label = copyString(locNb->label);
				globNb->isStringMaster = 1;
				globNb->endPoint = getVertex(p->vertexPool);
				globNb->startPoint = globalTree;

				/* add this edge at the head of globalTree->neighborhood */
				globNb->next = globalTree->neighborhood;
				globalTree->neighborhood = globNb;

				/* recursive call to merge search trees from this point on */
				mergeSearchTrees(globNb->endPoint, locNb->endPoint, divisor, results, pos, trueRoot, depth+1, p);
			}
		}

		/* at this point, any subtrees (i.e. vertices) dangling at the localTree->neighborhood edges are dumped
		 * so dump the vertex list.
		 * TODO think about this. dumping of the search tree can be handled by dumpSearchTree with no
		 *  additional asymptotic expenses. */
	}
}

/*
 * updates search tree structure.
 * labels of strings are stored in edges,
 * the endPoint vertex of an edge stores:
 * - possible extensions of the string in ->neighborhood
 * - the number of strings that are represented by the path from the root of the tree
 * 		to this vertex in ->visited, localTree->visited elements are divided by divisor
 * 		(use e.g. 2 for including cycles, as they are listed twice)
 * - the tree-wide unique id of the string, or 0 if no particular string ends there
 * 		in ->lowPoint
 */
void shallowMergeSearchTrees(struct Vertex* globalTree, struct Vertex* localTree, int divisor, struct compInfo* results, int* pos, struct Vertex* trueRoot, int depth, struct GraphPool* p) {


	/* process current vertex */

	/* if some string ends here, write it to the output array and update data structure */
	if (localTree->visited) {

		/* add current number of elements (divided by divisor) to global number */
		globalTree->visited += localTree->visited / divisor;

		/* if the current string has no global id yet, get the next one */
		if (globalTree->lowPoint == 0) {
			trueRoot->lowPoint++;
			globalTree->lowPoint = trueRoot->lowPoint;
		}

		/* if we have a results array, store the (id, count)-pair in the array */
		if (results) {

			results[*pos].id = globalTree->lowPoint;
			results[*pos].count = localTree->visited / divisor;
			results[*pos].depth = depth;
			*pos += 1;

		}

	}

	/* if the current vertex is no leaf, merge search trees recursively */
	if (localTree->neighborhood) {

		struct VertexList* locNb;

		/* TODO this is the ugliest, most inefficient stuff */

		/* process neighbors */
		for (locNb=localTree->neighborhood; locNb; locNb=locNb->next) {
			struct VertexList* globNb;
			char found = 0;

			for (globNb=globalTree->neighborhood; globNb; globNb=globNb->next) {

				/* if the next label is already in the tree, continue recursively */
				if (strcmp(globNb->label, locNb->label) == 0) {
					mergeSearchTrees(globNb->endPoint, locNb->endPoint, divisor, results, pos, trueRoot, depth+1, p);
					found = 1;
					break;
				}
			}

			if (!found) {

				/* we reach this point, iff locNb->label is not found in the labels of the global neigbors
				 * thus we have to add the edge and the vertex to the globalTree. This copying has to be a real
				 * copy, no shallow, as any label or vertex that the local tree refers to is dumped when
				 * processing of the current vertex is done.
				 */
				globNb = getVertexList(p->listPool);
				globNb->label = locNb->label;
				globNb->isStringMaster = 0;
				globNb->endPoint = getVertex(p->vertexPool);
				globNb->startPoint = globalTree;

				/* add this edge at the head of globalTree->neighborhood */
				globNb->next = globalTree->neighborhood;
				globalTree->neighborhood = globNb;

				/* recursive call to merge search trees from this point on */
				mergeSearchTrees(globNb->endPoint, locNb->endPoint, divisor, results, pos, trueRoot, depth+1, p);
			}
		}

		/* at this point, any subtrees (i.e. vertices) dangling at the localTree->neighborhood edges are dumped
		 * so dump the vertex list.
		 * TODO think about this. dumping of the search tree can be handled by dumpSearchTree with no
		 *  additional asymptotic expenses. */
	}
}



/**
 * Print the search tree to the screen. The output is not very user friendly
 */
void printSearchTree(struct Vertex* root, int level) {
	struct VertexList* index;

	for (index=root->neighborhood; index; index=index->next) {
		printf("l%i ", level);
		printf("(%i, %i) %s\n", index->endPoint->lowPoint, index->endPoint->visited, index->label);
		printSearchTree(index->endPoint, level+1);
	}
}


/**
 * DFD: depth first destruction of a tree
 */
void dumpSearchTree(struct GraphPool* p, struct Vertex* root) {
	struct VertexList *idx, *tmp;

	idx=root->neighborhood;
	while(idx) {
		dumpSearchTree(p, idx->endPoint);
		tmp = idx;
		idx = idx->next;
		dumpVertexList(p->listPool, tmp);
	}
	dumpVertex(p->vertexPool, root);
}



/*****************************************************************************************
 ************************************ Outerplanar Blocks *********************************
 *****************************************************************************************/


/*
 * returns the "correct" modulo of two numbers. b is assumed to be positive,
 * a is an arbitrary integer.
 */
int mod(int a, int b) {
	if (a % b >= 0) {
		return a % b;
	} else {
		return b + (a % b);
	}
}


/**
 * Comparator that compared two directed edges wrt the lexicographic order of (flag, used)
 */
int compareDiagonals(const void* e1, const void* e2) {

	struct VertexList* e = *((struct VertexList**)e1);
	struct VertexList* f = *((struct VertexList**)e2);

	if (e->flag - f->flag == 0) {
		return e->used - f->used;
	} else {
		return e->flag - f->flag;
	}
}


/**
 * !Be careful with this method! See compareCycles for more details
 *
 * Compare two cycles of equal length. Caution, the input parameters point to the edges
 * BEFORE the edges starting the cycle.
 * This is due to the usage of this method in permutateCycle().
 */
struct VertexList* compareBlockRepresentations(struct VertexList* c1, struct VertexList* c2, int m, int o1, int o2) {
	struct VertexList* idx1 = c1;
	struct VertexList* idx2 = c2;


	/* check for differences in the following edges */
	for (idx1=idx1->next, idx2=idx2->next;
			(idx1!=c1);
			idx1=idx1->next, idx2=idx2->next) {

		/* check for differences in the edge label */
		if (strcmp(idx1->label, idx2->label) < 0) {
			return c1;
		}
		if (strcmp(idx1->label, idx2->label) > 0) {
			return c2;
		}
		if (strcmp(idx1->label, idx2->label) == 0) {
			/* if edge labels do not differ, check for differences in endPoint label */
			if (strcmp(idx1->endPoint->label, idx2->endPoint->label) < 0) {
				return c1;
			}
			if (strcmp(idx1->endPoint->label, idx2->endPoint->label) > 0) {
				return c2;
			}
		}
	}

	/* check for differences in the edge label of the first edge*/
	if (strcmp(idx1->label, idx2->label) < 0) {
		return c1;
	}
	if (strcmp(idx1->label, idx2->label) > 0) {
		return c2;
	}
	if (strcmp(idx1->label, idx2->label) == 0) {
		/* if edge labels do not differ, check for differences in endPoint label */
		if (strcmp(idx1->endPoint->label, idx2->endPoint->label) < 0) {
			return c1;
		}
		if (strcmp(idx1->endPoint->label, idx2->endPoint->label) > 0) {
			return c2;
		}
	}


	/* if no difference is found in the hamiltonian cycle, check the diagonals */
	for (idx1=idx1->next, idx2=idx2->next; idx1!=c1; idx1=idx1->next, idx2=idx2->next) {
		struct VertexList* e;
		int i;

		int neighbors1 = 0;
		int neighbors2 = 0;

		struct VertexList** narray1;
		struct VertexList** narray2;

		/* count number of diagonals that go from the current vertex to a vertex with a higher
		 * index wrt the current permutation */
		for (e=idx1->endPoint->neighborhood; e; e=e->next) {
			if (mod(e->startPoint->d - o1, m + 1) < mod(e->endPoint->d - o1, m + 1)) {
				++neighbors1;
			}
		}
		for (e=idx2->endPoint->neighborhood; e; e=e->next) {
			if (mod(e->startPoint->d - o2, m + 1) < mod(e->endPoint->d - o2, m + 1)) {
				++neighbors2;
			}
		}



		/* if there are more edges with a low start vertex index, the resulting cstring is smaller */
		if (neighbors1 < neighbors2) {
			return c2;
		}
		if (neighbors2 < neighbors1) {
			return c1;
		}

		/* special cases where current vertices have no or just one incident diagonal */
		if (neighbors1 == 0) {
			continue;
		}
		if (neighbors1 == 1) {
			if (mod(idx1->endPoint->neighborhood->endPoint->d - o1, m + 1) > mod(idx2->endPoint->neighborhood->endPoint->d - o2, m + 1)) {
				return c2;
			} else {
				return c1;
			}
		}

		/* otherwise we have to sort the edges wrt their endpoints modulo the permutation */

		/* sort the edges according to their cycle indices modulo the permutation */
		narray1 = malloc(neighbors1 * sizeof(struct VertexList*));
		narray2 = malloc(neighbors2 * sizeof(struct VertexList*));
		for (i=0, e=idx1->endPoint->neighborhood; i<neighbors1; e=e->next) {
			e->flag = mod(e->startPoint->d - o1, m + 1);
			e->used = mod(e->endPoint->d - o1, m + 1);
			if (e->flag < e->used) {
				narray1[i] = e;
				++i;
			}
		}
		for (i=0, e=idx2->endPoint->neighborhood; i<neighbors2; e=e->next) {
			e->flag = mod(e->startPoint->d - o2, m + 1);
			e->used = mod(e->endPoint->d - o2, m + 1);
			if (e->flag < e->used) {
				narray2[i] = e;
				++i;
			}
		}

		qsort(narray1, neighbors1, sizeof(struct VertexList*), compareDiagonals);
		qsort(narray2, neighbors2, sizeof(struct VertexList*), compareDiagonals);

		/* check if the indices of the endpoints of the diagonals differ */
		for (i=0; i<neighbors1; ++i) {
			if (compareDiagonals(&(narray1[i]), &(narray2[i])) < 0) {
				free(narray1);
				free(narray2);
				return c1;
			}
			if (compareDiagonals(&(narray1[i]), &(narray2[i])) > 0) {
				free(narray1);
				free(narray2);
				return c2;
			}
			/* otherwise continue with the next loop iteration */
		}
		free(narray1);
		free(narray2);

	}

	/* if neither checking the hamiltonian cycle nor checking the diagonals resulted
	 * in any difference, return c1 */
	return c1;

}


/**
 * This method creates the string representation of a diagonal in an outerplanar block.
 * It returns "( start end label )" where whitespaces stand for "next vertex list".
 */
void appendDiagonal(struct ShallowGraph* diags, int start, int end, char* label, struct ListPool* p) {
	appendEdge(diags, getInitialisatorEdge(p));

	appendEdge(diags, getVertexList(p));
	diags->lastEdge->label = malloc(12*sizeof(char));
	diags->lastEdge->isStringMaster = 1;
	sprintf(diags->lastEdge->label, "%i", start);

	appendEdge(diags, getVertexList(p));
	diags->lastEdge->label = malloc(12*sizeof(char));
	diags->lastEdge->isStringMaster = 1;
	sprintf(diags->lastEdge->label, "%i", end);

	appendEdge(diags, getVertexList(p));
	diags->lastEdge->label = label;

	appendEdge(diags, getTerminatorEdge(p));
}


/**
 * Finds the cyclic permutation that results in a lexicographically smallest
 * representation of the input cycle, if one considers the canonical string beginning
 * with the edge label of the first edge in the ->edge list followed by the vertex label
 * of its endpoint etc.
 */
struct ShallowGraph* permutateBlock(struct ShallowGraph* cycle, struct ShallowGraphPool* sgp) {
	struct VertexList* start = cycle->edges;
	struct VertexList* end = assertLastPointer(cycle);
	/* best points to the edge before the starting edge of the lex smallest cycle */
	struct VertexList* best = cycle->edges;
	int bestOffset = 2;
	/* return */
	struct ShallowGraph* cString;
	struct ShallowGraph* secondPart = getShallowGraph(sgp);
	/* indices */
	struct VertexList* idx;
	int i;

	/* make actual cycle */
	end->next = start;

	/* give each vertex a number according to its position in cycle */
	for (idx=start->next, i=1; idx!=start; idx=idx->next, ++i) {
		idx->startPoint->d = i;
	}
	start->endPoint->d = 0;

	/* offset=3 as in the first step the permutation starting at edge after the second vertex
	 * in cycle is compared to the permutation starting after the first */
	for (idx = start->next, i=3; idx != start; idx=idx->next, i=(i+1)%(cycle->m + 1)) {
		best = compareBlockRepresentations(best, idx, cycle->m, bestOffset, i);
		if (best == idx) {
			bestOffset = i;
		}
	}

	/* turn cycle s.t. the shallow graph is lex. smallest */
	cycle->edges = best->next;
	cycle->lastEdge = best;
	best->next = NULL;

	/* compute canonical string of the component */
	for (idx=cycle->edges; idx; idx=idx->next) {
		struct VertexList* e;

		int neighbors = 0;

		struct VertexList** narray1;

		/* count number of diagonals that go from the current vertex to a vertex with a higher
		 * index wrt the current permutation */
		for (e=idx->endPoint->neighborhood; e; e=e->next) {
			if (mod(e->startPoint->d - bestOffset, cycle->m + 1) < mod(e->endPoint->d - bestOffset, cycle->m + 1)) {
				++neighbors;
			}
		}

		/* special cases where current vertices have no or just one incident diagonal */
		if (neighbors == 0) {
			continue; /* the idx loop */
		}

		if (neighbors == 1) {
			/* add edge representation to secondPart. There may be more than one incident diagonals, but just one that
			 * satisfies startpoint < endpoint. This is the edge we are searching. */
			for (e=idx->endPoint->neighborhood; e; e=e->next) {
				if (mod(e->startPoint->d - bestOffset, cycle->m + 1) < mod(e->endPoint->d - bestOffset, cycle->m + 1)) {
					appendDiagonal(secondPart,
							mod(e->startPoint->d - bestOffset, cycle->m + 1),
							mod(e->endPoint->d - bestOffset, cycle->m + 1),
							e->label,
							sgp->listPool);

					break; /* inner for, there is just one edge */
				}
			}
			continue; /* the idx loop */
		}

		/* otherwise: sort the edges according to their cycle indices modulo the permutation */
		narray1 = malloc(neighbors * sizeof(struct VertexList*));

		for (i=0, e=idx->endPoint->neighborhood; i<neighbors; e=e->next) {
			e->flag = mod(e->startPoint->d - bestOffset, cycle->m + 1);
			e->used = mod(e->endPoint->d - bestOffset, cycle->m + 1);
			if (e->flag < e->used) {
				narray1[i] = e;
				++i;
			}
		}

		qsort(narray1, neighbors, sizeof(struct VertexList*), compareDiagonals);

		/* append string representation of diagonal */
		for (i=0; i<neighbors; ++i) {
			appendDiagonal(secondPart, narray1[i]->flag, narray1[i]->used, narray1[i]->label, sgp->listPool);
		}

		/* garbage collection */
		free(narray1);
	}

	cString = getCStringOfCycle(cycle, sgp);
	cString->lastEdge->next = secondPart->edges;
	cString->lastEdge = secondPart->lastEdge;
	cString->m = cString->m + secondPart->m;
	pushEdge(cString, getInitialisatorEdge(sgp->listPool));
	appendEdge(cString, getTerminatorEdge(sgp->listPool));

	/* garbage collection */
	secondPart->edges = NULL;
	dumpShallowGraph(sgp, secondPart);

	return cString;
}


/**
 * Compute the canonical string of an outerplanar block given as hamiltonian cycle and list of diagonals.
 * This method assumes that graph, the start- and endpoints of the edges point to, contains no edges.
 *
 * If you want to use this method in a context, where the underlying graph is not empty, you either
 * have to delete all edges from him or you have to grate an empty new graph and change the start-
 * and endpoints of edges in hamiltonianCycle and diagonals accordingly.
 */
struct ShallowGraph* getCanonicalStringOfOuterplanarBlock(struct ShallowGraph* hamiltonianCycle, struct ShallowGraph* diagonals, struct ShallowGraphPool* sgp) {
	struct VertexList* idx;
	struct ShallowGraph* result1;
	struct ShallowGraph* result2;
	struct ShallowGraph* inverse = inverseCycle(hamiltonianCycle, sgp);

	/* special case. if block is a cycle, return the canonical string of that cycle */
	if (diagonals->m == 0) {
		dumpShallowGraph(sgp, diagonals);
		dumpShallowGraph(sgp, inverse);
		return getCyclePatterns(hamiltonianCycle, sgp);
	}

	/* add diagonals to the graph underlying the shallowgraphs */
	for (idx=diagonals->edges; idx; idx=idx->next) {
		struct VertexList* e = shallowCopyEdge(idx, sgp->listPool);
		e->next = idx->startPoint->neighborhood;
		idx->startPoint->neighborhood = e;

		e = inverseEdge(idx, sgp->listPool);
		e->next = idx->endPoint->neighborhood;
		idx->endPoint->neighborhood = e;
	}

	/* now find the permutation and orientation of the hamiltonian cycle that results in the lex.
	 * smallest string */
	result1 = permutateBlock(hamiltonianCycle, sgp);
	result2 = permutateBlock(inverse, sgp);


	/* garbage collection */
	dumpShallowGraph(sgp, inverse);
	dumpShallowGraph(sgp, hamiltonianCycle);
	dumpShallowGraph(sgp, diagonals);

	/* compare cStrings and return the better one */
	if (compareVertexLists(result1->edges, result2->edges) < 0) {
		dumpShallowGraph(sgp, result2);
		return result1;
	} else {
		dumpShallowGraph(sgp, result1);
		return result2;
	}

}



/*****************************************************************************************
 ************************************ User Output ****************************************
 *****************************************************************************************/


/**
 * Conversion method to convert a string stored as a labeled path to a char array string
 */
char* canonicalStringToChar(struct ShallowGraph* string) {
	char* out;
	struct VertexList* e;
	int size = 0;
	int i;

	/* count the space needed for the string (+1 for a whitespace separating two labels) */
	for (e=string->edges; e; e=e->next) {
		size += strlen(e->label) + 1;
	}
	out = malloc((size + 1) * sizeof(char));


	for (e=string->edges, i=0; e; e=e->next) {
		sprintf(&(out[i]), "%s ", e->label);
		i += strlen(e->label) + 1;
	}

	return out;
}


/**
Print the canonical string, represented by the ShallowGraph s to 
the screen.
 */
void printCanonicalString(struct ShallowGraph *s) {
	struct VertexList *i;
	for (i=s->edges; i; i = i->next) {
		printf("%s", i->label);
	}
	printf("\n");
}


/**
Print the canonical strings represented by the list of
ShallowGraphs s 
 */
void printCanonicalStrings(struct ShallowGraph *s) {
	struct ShallowGraph* i;
	for (i=s; i; i=i->next) {
		printCanonicalString(i);
	}
}
