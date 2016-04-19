#include <malloc.h>
#include <string.h>
#include <assert.h>

#include "graph.h"
#include "cs_Tree.h"
#include "searchTree.h"
#include "bloomFilter.h"
#include "subtreeIsoUtils.h"
#include "treeEnumeration.h"
#include "outerplanar.h"
#include "intSet.h"


/**
 Creates a hard copy of g with g->n + 1 vertices and g->m + 1 edges that has a new vertex that is a
 copy of newEdge->endPoint and a new edge from the vertex v in g to the new vertex. with label newEdge->label.

 Does not check if newEdge->startPoint->label == g->vertices[v]->label!

 The newly added vertex v has v->number = g->n, the vertex numbers of the vertices of g do not change.
 */
struct Graph* refinementGraph(struct Graph* g, int v, struct VertexList* newEdge, struct GraphPool* gp) {
	struct Graph* copy = getGraph(gp);
	int i;

	copy->activity = g->activity;
	copy->m = g->m; // addEdgebetweenVertices adds plus 1 itself
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


/** Return a list of vertices that are on a path from v to the vertex with ->number == 0.
 * The implementation assumes that the tree that v belongs to was created by an extension process
 * that adds new leaves to an existing graph and hence that there is at most one neighbor for
 * each vertex that has a lower ->number.
 */
static struct VertexList* getMostRecentPath(struct Vertex* v, struct ListPool* lp) {
	if (v->number == 0) {
		struct VertexList* goal = getVertexList(lp);
		goal->startPoint = v;
		return goal;
	}
	for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->number < v->number) {
			struct VertexList* path = getVertexList(lp);
			path->startPoint = v;
			path->next = getMostRecentPath(e->endPoint, lp);
			return path;
		}
	}
	fprintf(stderr, "Nastiness occurred.\n");
	return NULL; // should never happen
}

/**
CAUTION: NOT A COMPLETE ENUMERATOR AT THE MOMENT

Take a tree and add any edge in the candidate set to each vertex in the graph that is on the path
from vertex 0 to vertex n-1.
candidateEdges is expected to contain edges that have a nonNULL ->startPoint and ->endPoint.
The label of startPoint determines, which vertex can be used for appending the edge,
the label of the endpoint defines the label of the new vertex added

TODO make this thing a complete enumeration, if possible.
 */
struct Graph* extendPatternOnMostRecentPath(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {
	struct Graph* rho = NULL;
	struct VertexList* mostRecentPath = getMostRecentPath(g->vertices[g->n-1], gp->listPool);
	for (struct VertexList* e=candidateEdges->edges; e!=NULL; e=e->next) {
		for (struct VertexList* v=mostRecentPath; v!=NULL; v=v->next) {
			/* if the labels are compatible */
			if (strcmp(v->startPoint->label, e->startPoint->label) == 0) {
				struct Graph* h = refinementGraph(g, v->startPoint->number, e, gp);
				h->next = rho;
				rho = h;
			}
		}
	}
	return rho;
}

static char hasDegreeLarger2(struct Vertex* v) {
	return ((v->neighborhood) && (v->neighborhood->next) && (v->neighborhood->next->next));
}


static char isReverseIsomorphicEdge(struct VertexList* e, struct VertexList* candidate) {
	return (labelCmp(e->startPoint->label, candidate->endPoint->label) == 0) &&
		   (labelCmp(e->endPoint->label, candidate->startPoint->label) == 0) &&
		   (labelCmp(e->label, candidate->label) == 0);
}


/**
 * CAUTION: NOT A COMPLETE ENUMERATOR AT THE MOMENT
 *
 * Not complete. However, much more complete than the most recent path idea.
 *
 * Idea: only extend frequent graph with edge that is larger than the largest leaf in some total order on extension edges.
 * However, problems occur, if a leaf's removal creates a new leaf.
 */
static struct VertexList* filterCandidateEdgesByOrder(struct Graph* g, struct ShallowGraph* candidateEdges, struct ListPool* lp) {
	struct VertexList* filteredEdges = NULL;
	for (struct VertexList* e=candidateEdges->edges; e!=NULL; e=e->next) {
		filteredEdges = push(filteredEdges, shallowCopyEdge(e, lp));
	}
	for (int v=0; v<g->n; ++v) {
		/* If the removal of v from g results in a new leaf edge, this leaf (in g-v) may have a higher order than the leaf to v, hence we cannot do the thing */
		if (isLeaf(g->vertices[v]) && hasDegreeLarger2(g->vertices[v]->neighborhood->endPoint)) {
			// compare edge leading to v to extension edges, remove the ones that come after it, if still there.
			struct VertexList* e = g->vertices[v]->neighborhood;
			for (struct VertexList* candidate=filteredEdges; candidate!=NULL; candidate=candidate->next) {
				// extension edges end at the leaf, e is the other way round, hence change positions of startPoint and endPoint
				if (isReverseIsomorphicEdge(e, candidate)) {
					dumpVertexListRecursively(lp, candidate->next);
					candidate->next = NULL;
					break;
				}
			}
		}
	}
	return filteredEdges;
}


///**
// * CAUTION: NOT A COMPLETE ENUMERATOR AT THE MOMENT
// *
// * Not complete. However, much more complete than the most recent path idea.
// *
// * Idea: only extend frequent graph with edge that is larger than the largest leaf in some total order on extension edges.
// * However, problems occur, if a leaf's removal creates a new leaf.
// */
//static struct VertexList* filterCandidateEdgesByOrder2(struct Graph* g, struct ShallowGraph* candidateEdges, struct ListPool* lp) {
//	struct VertexList* filteredEdges = NULL;
//	for (struct VertexList* e=candidateEdges->edges; e!=NULL; e=e->next) {
//		filteredEdges = push(filteredEdges, shallowCopyEdge(e, lp));
//	}
//	for (int v=0; v<g->n; ++v) {
//		if (isLeaf(g->vertices[v])) {
//			struct VertexList* e = g->vertices[v]->neighborhood;
//			struct VertexList* candidate = filteredEdges;
//
//			// special case where removal of e results in a leaf
//			if (hasDegreeLarger2(e->endPoint)) {
//				// find the leaf in g-e
//				struct VertexList* f = e->endPoint->neighborhood;
//				if (f->endPoint->number == v) {
//					f = f->next; // we want the edge from the neighbor of v to the other incident vertex, not to v
//				}
//				// fast-forward to the new leaf
//				for (/*candidate was set before*/; candidate!=NULL; candidate=candidate->next) {
//					// extension edges end at the leaf, e is the other way round, hence change positions of startPoint and endPoint
//					if (isReverseIsomorphicEdge(f, candidate)) {
//						break;
//					}
//				}
//
//			}
//
//			// compare edge leading to v to extension edges, remove the ones that come after it, if there are any
//			for (/*candidate was set before*/; candidate!=NULL; candidate=candidate->next) {
//				// extension edges end at the leaf, e is the other way round, hence change positions of startPoint and endPoint
//				if (isReverseIsomorphicEdge(e, candidate)) {
//					dumpVertexListRecursively(lp, candidate->next);
//					candidate->next = NULL;
//					break;
//				}
//			}
//		}
//	}
//	return filteredEdges;
//}


/**
 * CAUTION: NOT A COMPLETE ENUMERATOR AT THE MOMENT
 */
struct Graph* extendPatternByLargerEdges(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {
	struct Graph* rho = NULL;
	struct VertexList* filteredCandidates = filterCandidateEdgesByOrder(g, candidateEdges, gp->listPool);
	for (struct VertexList* e=filteredCandidates; e!=NULL; e=e->next) {
		for (int v=0; v<g->n; ++v) {
			/* if the labels are compatible */
			if (strcmp(g->vertices[v]->label, e->startPoint->label) == 0) {
				struct Graph* h = refinementGraph(g, v, e, gp);
				h->next = rho;
				rho = h;
			}
		}
	}
	dumpVertexListRecursively(gp->listPool, filteredCandidates);
	return rho;
}


/**
 * CAUTION: NOT A COMPLETE ENUMERATOR AT THE MOMENT
 */
struct Graph* extendPatternByLargerEdgesTMP(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {
	if (g->n < 3) {
		return extendPattern(g, candidateEdges, gp);
	} else {
		return extendPatternByLargerEdges(g, candidateEdges, gp);
	}
}

///**
//check if the canonicalStringOfTree of pattern is already contained in search tree
// */
//static char alreadyEnumerated(struct Graph* pattern, struct Vertex* searchTree, struct ShallowGraphPool* sgp) {
//	struct ShallowGraph* string = canonicalStringOfTree(pattern, sgp);
//	char alreadyFound = containsString(searchTree, string);
//	dumpShallowGraph(sgp, string);
//	return alreadyFound;
//}


/**
Return the list of all graphs in extension that are not contained in listOfGraphs (given as searchTree).
extension is consumed. after the call, canonical strings of all elements in extension will be added to listOfGraphs */
struct Graph* basicFilter(struct Graph* extension, struct Vertex* listOfGraphs, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* result = NULL;
	struct Graph* g = extension;
	while (g != NULL) {
		// store second element, detach head
		struct Graph* tmp = g->next;
		g->next = NULL;

		struct ShallowGraph* string = canonicalStringOfTree(g, sgp);
		char alreadyFound = containsString(listOfGraphs, string);
		if (!alreadyFound) {
			addToSearchTree(listOfGraphs, string, gp, sgp);
			// add to result
			g->next = result;
			result = g;
		} else {
			dumpShallowGraph(sgp, string);
			dumpGraph(gp, g);
		}
		// move on
		g = tmp;
	}
	return result;
}


/**
return the fingerprint of pattern (being the bitwise or of the hashes of the subgraphs)
OR 0 if there exists a subgraph of pattern that is not frequent.
 */
static int getPatternFingerPrint(struct Graph* pattern, struct Vertex* searchTree, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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
static struct Graph* filterExtension(struct Graph* extension, struct Vertex* lowerLevel, struct Vertex* currentLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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


/**
for each graph g in extension (that is the list starting at extension and continuing at extension->next), 
two tests are run: g itself must not be contained in currentLevel and any subtree of g with n-1 vertices must be contained
in lowerLevel (this ensures the apriori property that all subtrees are frequent). 
if g fulfills both conditions, it is added to the output and to currentLevel.

In contrast to the above filterExtension, this methos does not use any hashing.
 */
static struct Graph* aprioriFilterExtension(struct Graph* extension, struct Vertex* lowerLevel, struct Vertex* currentLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int v;
	struct Graph* idx = extension; 
	struct Graph* filteredExtension = NULL;

	// create graph that will hold subgraphs of size n-1 (this assumes that all extension trees have the same size)
	struct Graph* subgraph = getGraph(gp);
	setVertexNumber(subgraph, extension->n - 1);
	subgraph->m = subgraph->n - 1;

	while (idx != NULL) {
		int notEnumeratedYet;

		struct Graph* current = idx;
		struct ShallowGraph* string;
		idx = idx->next;
		current->next = NULL;

		/* filter out patterns that were already enumerated as the extension of some other pattern
		and are in the search tree */
		string = canonicalStringOfTree(current, sgp);
		notEnumeratedYet = containsString(currentLevel, string);

		if (notEnumeratedYet) {
			dumpShallowGraph(sgp, string);
			dumpGraph(gp, current);
			continue; // with next refinement
		} else {
			char aprioriProperty = 1;
			struct ShallowGraph* subString;

			for (v=0; v<current->n; ++v) {
				// if the removed vertex is a leaf, we test if the resulting subtree is contained in the lower level
				if (isLeaf(current->vertices[v]) == 1) {
					// we invalidate current by removing the edge to v from its neighbor, which makes subgraph a valid tree
					struct VertexList* edge = snatchEdge(current->vertices[v]->neighborhood->endPoint, current->vertices[v]);

					int i = 0;
					int j = 0;
					for (i=0; i<current->n; ++i) {
						if (i == v) {
							continue;
						} else {
							subgraph->vertices[j] = current->vertices[i];
							subgraph->vertices[j]->number = j;
							++j;
						}
					}
					subString = canonicalStringOfTree(subgraph, sgp);
					// restore law and order in current
					addEdge(edge->startPoint, edge);
					// test apriori property
					aprioriProperty = containsString(lowerLevel, subString);
					dumpShallowGraph(sgp, subString);
					if (!aprioriProperty) {
						break; // looping through the vertices of current and continue with next refinement
					}
				}
			}

			// clean up and garbage collection
			for (v=0; v<current->n; ++v) {
				current->vertices[v]->number = v;
			}

			if (aprioriProperty) {
				// add current to filtered extension
				current->next = filteredExtension;
				filteredExtension = current;
				addToSearchTree(currentLevel, string, gp, sgp);
			} else {
				dumpGraph(gp, current);
			}
		}
	}

	// garbage collection
	for (v=0; v<subgraph->n; ++v) {
		subgraph->vertices[v] = 0;
	}
	dumpGraph(gp, subgraph);
	// fprintf(stderr, "return\n");
	return filteredExtension;
}


/**
for a given extension graph g several tests are run:

- any subtree of g with n-1 vertices (called an apriori parent) must be contained in lowerLevel
  (this ensures the apriori property that all subtrees are frequent).

if g fulfills this conditions, the method returns a list of the ids of the apriori parents in resultSetStore.
if g does not, NULL is returned.

Differences to other extension filter methods in this module:
This method only processes a single extension graph at a time. It does not do anything to it.
In contrast to the above filterExtension, this method does not use any hashing.
In contrast to aprioriFilterExtension, this method also returns a list of IntSets, that contain the ids of
the relevant (n-1)-vertex subtrees and sets h->number to its id (obtained by getID(currentLevel, cString(h)).
 */
struct IntSet* aprioriCheckExtensionReturnList(struct Graph* extension, struct Vertex* lowerLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	// returning NULL will indicate that apriori property does not hold for current
	struct IntSet* aprioriTreesOfExtension = getIntSet();

	// create graph that will hold subgraphs of size n-1 (this assumes that all extension trees have the same size)
	struct Graph* subgraph = getGraph(gp);
	setVertexNumber(subgraph, extension->n - 1);
	subgraph->m = subgraph->n - 1;

	for (int v=0; v<extension->n; ++v) {
		// if the removed vertex is a leaf, we test if the resulting subtree is contained in the lower level
		if (isLeaf(extension->vertices[v]) == 1) {
			// we invalidate current by removing the edge to v from its neighbor, which makes subgraph a valid tree
			struct VertexList* edge = snatchEdge(extension->vertices[v]->neighborhood->endPoint, extension->vertices[v]);
			// now copy pointers of all vertices \neq v to subgraph, this results in a tree of size current->n - 1
			int j = 0;
			for (int i=0; i<extension->n; ++i) {
				if (i == v) {
					continue; // ...with next vertex vertices
				} else {
					subgraph->vertices[j] = extension->vertices[i];
					subgraph->vertices[j]->number = j;
					++j;
				}
			}

			// test apriori property
			struct ShallowGraph* subString = canonicalStringOfTree(subgraph, sgp);
			int aprioriTreeID = getID(lowerLevel, subString);
			dumpShallowGraph(sgp, subString);

			// restore law and order in current (and invalidate subgraph)
			addEdge(edge->startPoint, edge);

			if (aprioriTreeID == -1) {
				dumpIntSet(aprioriTreesOfExtension);
				aprioriTreesOfExtension = NULL;
				break; // looping through the vertices of current and continue with next refinement
			} else {
				addIntSortedNoDuplicates(aprioriTreesOfExtension, aprioriTreeID);
			}
		}
	}

	// clean up
	for (int v=0; v<extension->n; ++v) {
		extension->vertices[v]->number = v;
	}
	// garbage collection
	for (int v=0; v<subgraph->n; ++v) {
		subgraph->vertices[v] = NULL;
	}
	dumpGraph(gp, subgraph);

	if (aprioriTreesOfExtension != NULL) {
		assert(isSortedUniqueIntSet(aprioriTreesOfExtension));
		return aprioriTreesOfExtension;
	} else {
		return NULL;
	}
}





/**
for each graph g in extension (that is the list starting at extension and continuing at extension->next), 
two tests are run: g itself must not be contained in currentLevel and any subtree of g must be contained
in lowerLevel. 
if g fulfills both conditions, it is added to the output and to currentLevel.
 */
static struct Graph* filterExtensionForPaths(struct Graph* extension, struct Vertex* lowerLevel, struct Vertex* currentLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* idx = extension; 
	struct Graph* filteredExtension = NULL;

	while (idx != NULL) {
		struct VertexList* e;
		int fingerPrint;

		struct Graph* current = idx;
		struct ShallowGraph* string;
		idx = idx->next;
		current->next = NULL;

		/* filter out patterns that are not paths */
		if (!isPath(current)) {
			dumpGraph(gp, current);
			continue;
		}

		/* filter out patterns that were already enumerated as the extension of some other pattern
		and are in the search tree */
		string = canonicalStringOfTree(current, sgp);
		if (containsString(currentLevel, string)) {
			dumpShallowGraph(sgp, string);
			dumpGraph(gp, current);
			continue;
		} 

		/* filter out patterns where a subtree is not frequent */
		fingerPrint = getPatternFingerPrint(current, lowerLevel, gp, sgp);
		if (fingerPrint == 0) {
			dumpShallowGraph(sgp, string);
			dumpGraph(gp, current);
			continue;
		} 

		/* make shallow copies of labels to real copies 
		TODO: Might not be necessary any more, as addStringToSearchTree should handle this. */
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
	return filteredExtension;
}


static void generateCandidateSetRec(struct Vertex* lowerLevel, struct Vertex* currentLevel, struct ShallowGraph* frequentEdges, struct Graph* (*filter)(struct Graph*, struct Vertex*, struct Vertex*, struct GraphPool*, struct ShallowGraphPool*), struct Vertex* root, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if ((root->visited != 0) && (root != lowerLevel)) {
		/* at this point, we have found a pattern, we want to make a tree from it, get the refinements,
		filter interesting candidates and then scan the db of patterns  */
		// fprintf(stderr, "pattern to extend: ");
		// printCanonicalString(prefix, stderr);
		struct Graph* pattern = treeCanonicalString2Graph(prefix, gp);
		struct Graph* refinements = extendPattern(pattern, frequentEdges, gp);
		refinements = filter(refinements, lowerLevel, currentLevel, gp, sgp);

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

		generateCandidateSetRec(lowerLevel, currentLevel, frequentEdges, filter, e->endPoint, prefix, gp, sgp);

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


struct Vertex* generateCandidateTreeSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Vertex* currentLevel = getVertex(gp->vertexPool);
	struct ShallowGraph* prefix = getShallowGraph(sgp);
	/* set smallest id of pattern in current level to be largest id of any pattern in lower level plus 1 */
	currentLevel->lowPoint = lowerLevel->lowPoint;
	generateCandidateSetRec(lowerLevel, currentLevel, extensionEdges, &filterExtension, lowerLevel, prefix, gp, sgp);
	dumpShallowGraph(sgp, prefix);
	return currentLevel;
}


struct Vertex* generateCandidateAprioriTreeSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Vertex* currentLevel = getVertex(gp->vertexPool);
	struct ShallowGraph* prefix = getShallowGraph(sgp);
	/* set smallest id of pattern in current level to be largest id of any pattern in lower level plus 1 */
	currentLevel->lowPoint = lowerLevel->lowPoint;
	generateCandidateSetRec(lowerLevel, currentLevel, extensionEdges, &aprioriFilterExtension, lowerLevel, prefix, gp, sgp);
	dumpShallowGraph(sgp, prefix);
	return currentLevel;
}


struct Vertex* generateCandidatePathSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Vertex* currentLevel = getVertex(gp->vertexPool);
	struct ShallowGraph* prefix = getShallowGraph(sgp);
	/* set smallest id of pattern in current level to be largest id of any pattern in lower level plus 1 */
	currentLevel->lowPoint = lowerLevel->lowPoint;
	generateCandidateSetRec(lowerLevel, currentLevel, extensionEdges, &filterExtensionForPaths, lowerLevel, prefix, gp, sgp);
	dumpShallowGraph(sgp, prefix);
	return currentLevel;
}
