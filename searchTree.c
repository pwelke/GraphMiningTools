#include <stddef.h>
#include <string.h>
#include <malloc.h>

#include "graph.h"
#include "canonicalString.h"
#include "searchTree.h"


/**************************************************************************************
 ******************************** Search Tree *****************************************
 **************************************************************************************/



/**
 * Recursively add a string represented by a list of edges to a search tree given by its root.
 * The string is consumed
 * This method DOES NOT INCREASE the current number of strings contained in the search tree.
 * You have to maintain the correct number of strings in the search tree yourself.
 * However, it returns 1, if the string was not contained in the trie before and 0 otherwise.
 */
char addStringToSearchTree(struct Vertex* root, struct VertexList* edge, struct GraphPool* p) {

	/* if edge == NULL, stop recursion, remember, that some string ends here */
	if (edge == NULL) {
		root->visited += 1;
		return 0;
	} else {
		struct VertexList* idx;
		for (idx=root->neighborhood; idx; idx=idx->next) {
			/* if the next label is already in the tree, continue recursively */
			if (strcmp(idx->label, edge->label) == 0) {
				char isNew = addStringToSearchTree(idx->endPoint, edge->next, p);
				/* edges dangling at edge are consumed or dumped by the following recursion steps */
				edge->next = NULL;
				dumpVertexList(p->listPool, edge);
				return isNew;
			}
		}

		/* otherwise add the current edge to the tree and continue recursively */
		idx = edge->next;
		edge->startPoint = root;
		edge->endPoint = getVertex(p->vertexPool);
		addEdge(root, edge);
		addStringToSearchTree(edge->endPoint, idx, p);
		return 1;
	}
}


char containsStringRec(struct Vertex* root, struct VertexList* edge) {

	/* if edge == NULL, we are done. check if visited > 0.  */
	if (edge == NULL) {
		return root->visited;
	} else {
		struct VertexList* idx;
		for (idx=root->neighborhood; idx; idx=idx->next) {
			/* if the next label is already in the tree, continue recursively */
			if (strcmp(idx->label, edge->label) == 0) {
				char found = containsStringRec(idx->endPoint, edge->next);
				return found;
			}
		}

		/* otherwise add the current edge to the tree and continue recursively */
		return 0;
	}
}


/**
If string is contained in searchTree given by root, return its multiplicity (->visited),
which is bound to be larger than zero.
Otherwise, return zero.
*/
char containsString(struct Vertex* root, struct ShallowGraph* string) {
	return containsStringRec(root, string->edges);
}


int getIDRec(struct Vertex* root, struct VertexList* edge) {

	/* if edge == NULL, we are done. check if visited > 0.  */
	if (edge == NULL) {
		return root->lowPoint;
	} else {
		struct VertexList* idx;
		for (idx=root->neighborhood; idx; idx=idx->next) {
			/* if the next label is already in the tree, continue recursively */
			if (strcmp(idx->label, edge->label) == 0) {
				return = getIDRec(idx->endPoint, edge->next);
				
			}
		}

		/* otherwise add the current edge to the tree and continue recursively */
		return -1;
	}
}


/**
If string is contained in search tree given by root, return its id (->lowPoint).
Otherwise return -1
*/
int getID(struct Vertex* root, struct ShallowGraph* string) {
	return getIDRec(root, string->edges);
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
This function adds a NULL terminated list of ShallowGraphs to a search tree likeish structure 
given by its root. The list is consumed and dumped.

The resulting structure is a tree rooted at the return value where a path from the root
to some vertex v in the tree represents v->visited many strings.

root->number gives the size of strings, root->d gives the number of unique elements in strings. 

 */
struct Vertex* addToSearchTree(struct Vertex* root, struct ShallowGraph* strings, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph *idx;

	for (idx=strings; idx; idx=idx->next) {
		root->d += addStringToSearchTree(root, idx->edges, gp);
		root->number += 1;

		idx->edges = NULL;
	}

	dumpShallowGraphCycle(sgp, strings);

	return root;
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
		if (globalTree->visited == 0) {
			++trueRoot->d;
		}

		/* add current number of elements (divided by divisor) to global number */
		globalTree->visited += localTree->visited / divisor;
		trueRoot->number += localTree->visited / divisor;

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

/**
Given a search tree, omit any multiplicity of strings contained. 
that is: if current->visited > 0, set visited = 1 for current != root.
root visited will be updated to store number of unique strings */ 
void resetToUniqueRec(struct Vertex* root, struct Vertex* current) {
	struct VertexList* e;
	if (current != root) {
		if (current->visited > 0) {
			current->visited = 1;
			++root->d;
			++root->number;
		}
	}

	for (e=current->neighborhood; e!=NULL; e=e->next) {
		resetToUniqueRec(root, e->endPoint);
	}
}

void resetToUnique(struct Vertex* root) {
	root->d = 0;
	root->number = 0;
	resetToUniqueRec(root, root);
}


/**
Given a search tree, omit any multiplicity of strings contained. 
that is: if current->visited > 0, set visited = 1 for current != root.
root visited will be updated to store number of unique strings */ 
void setLowPointsRec(struct Vertex* root, struct Vertex* current) {
	struct VertexList* e;
	if (current != root) {
		if (current->visited > 0) {
			++root->lowPoint;
			current->lowPoint = root->lowPoint;
		}
	}

	for (e=current->neighborhood; e!=NULL; e=e->next) {
		setLowPointsRec(root, e->endPoint);
	}
}

void setLowPoints(struct Vertex* root) {
	setLowPointsRec(root, root);
}


/**
Remove all strings from search tree that occur less than threshold times.
(this is indicated by the ->visited field of the last vertex).
*/
char filterSearchTree(struct Vertex* current, int threshold, struct Vertex* root, struct GraphPool* gp) {
	struct VertexList* e;
	struct VertexList* dump = NULL;
	struct VertexList* good = NULL;

	for (e=current->neighborhood; e!=NULL; e=e->next) {
		e->used = filterSearchTree(e->endPoint, threshold, root, gp);
	}

	for (e=current->neighborhood; e!=NULL; ) {
		struct VertexList* next = e->next;
		if (e->used) {
			e->next = good;
			good = e;
		} else {
			e->next = dump;
			dump = e;
		}
		e = next;
	}

	for (e=good, current->neighborhood=NULL; e!=NULL; ) {
		struct VertexList* next = e->next;
		e->next = current->neighborhood;
		current->neighborhood = e;
		e = next;
	}
	dumpVertexListRecursively(gp->listPool, dump);

		/* if all children of current were deleted and current is not end of frequent
	string, delete current and notify caller that we can delete the edge */
	if (current->visited < threshold) {
		if (current->visited > 0) {
			/* "remove" string from search tree */ 
			root->number -= current->visited;
			--root->d;
		}

		/* if current is not an internal vertex, we dump it and tell the caller
		that the edge is unused. otherwise, the vertex stays and the edge is marked
		as used */
		if (current->neighborhood == NULL) {
			/* an empty search tree still consists of the root */
			if (current != root) {
				dumpVertex(gp->vertexPool, current);
			}
			return 0;
		} else {
			return 1;
		}

	} else {
		/* string is frequent. */
		return 1;
	}
}


/**
Remove all strings from search tree that occur less than threshold times.
(this is indicated by the ->visited field of the last vertex).

Additionally, write the ids of the strings (stored in ->lowPoint) to the lowPoints stream.
*/
char filterSearchTreeP(struct Vertex* current, int threshold, struct Vertex* root, FILE* lowPoints, struct GraphPool* gp) {
	struct VertexList* e;
	struct VertexList* dump = NULL;
	struct VertexList* good = NULL;

	for (e=current->neighborhood; e!=NULL; e=e->next) {
		e->used = filterSearchTreeP(e->endPoint, threshold, root, lowPoints, gp);
	}

	for (e=current->neighborhood; e!=NULL; ) {
		struct VertexList* next = e->next;
		if (e->used) {
			e->next = good;
			good = e;
		} else {
			e->next = dump;
			dump = e;
		}
		e = next;
	}

	for (e=good, current->neighborhood=NULL; e!=NULL; ) {
		struct VertexList* next = e->next;
		e->next = current->neighborhood;
		current->neighborhood = e;
		e = next;
	}
	dumpVertexListRecursively(gp->listPool, dump);

	/* if all children of current were deleted and current is not end of frequent
	string, delete current and notify caller that we can delete the edge */
	if (current->visited < threshold) {
		if (current->visited > 0) {
			/* "remove" string from search tree */ 
			root->number -= current->visited;
			--root->d;
		}

		/* if current is not an internal vertex, we dump it and tell the caller
		that the edge is unused. otherwise, the vertex stays and the edge is marked
		as used */
		if (current->neighborhood == NULL) {
			/* an empty search tree still consists of the root */
			if (current != root) {
				dumpVertex(gp->vertexPool, current);
			}
			return 0;
		} else {
			return 1;
		}

	} else {
		/* string is frequent. */
		fprintf(lowPoints, "%i\n", current->lowPoint);
		return 1;
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
		trueRoot->d += localTree->visited / divisor;

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


/**
This function builds a search tree likeish structure given a NULL terminated
list of ShallowGraphs. The list is consumed and dumped.

The resulting structure is a tree rooted at the return value where a path from the root
to some vertex v in the tree represents v->visited many strings.

root->number gives the size of strings, root->d gives the number of unique elements in strings. 

 */
struct Vertex* buildSearchTree(struct ShallowGraph* strings, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Vertex* root = getVertex(gp->vertexPool);
	return addToSearchTree(root, strings, gp, sgp);
}


void recPrint(struct Vertex* root, struct Vertex* trueRoot, struct ShallowGraph* prefix, FILE* stream, struct ShallowGraphPool* sgp) {
	struct VertexList* e;

	if (root != trueRoot) {
		if (root->visited != 0) {
			fprintf(stream, "%i\t", root->visited);
			printCanonicalString(prefix, stream);
		}
	}																																																																								

	for (e=root->neighborhood; e!=NULL; e=e->next) {
		
		/* after finishing this block, we want prefix to be as before, thus we have
		   to do some list magic */																																																								
		struct VertexList* lastEdge = prefix->lastEdge;
		appendEdge(prefix, shallowCopyEdge(e, sgp->listPool));

		recPrint(e->endPoint, root, prefix, stream, sgp);

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


void printStringsInSearchTree(struct Vertex* root, FILE* stream, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* prefix = getShallowGraph(sgp);
	recPrint(root, root, prefix, stream, sgp);
	dumpShallowGraph(sgp, prefix);
}


int streamBuildSearchTree(FILE* stream, struct Vertex* root, int bufferSize, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int number;
	int nPatterns;
	int i;
	char* buffer = malloc(bufferSize * sizeof(char));
	int head = fscanf(stream, "# %i %i\n", &number, &nPatterns);
	if (head != 2) {
		return 0;
	}

	for (i=0; i<nPatterns; ++i) {
		int multiplicity;
		struct ShallowGraph* string;
		fscanf(stream, "%i\t", &multiplicity);
		string = parseCString(stream, buffer, sgp);
		addToSearchTree(root, string, gp, sgp);
	}	
	free(buffer);
	return 1;
}

struct ShallowGraph* streamReadPatterns(FILE* stream, int bufferSize, int* number, struct ShallowGraphPool* sgp) {
	int nPatterns;
	int i;
	struct ShallowGraph* patterns = NULL;

	char* buffer = malloc(bufferSize * sizeof(char));
	int head = fscanf(stream, " # %i %i\n", number, &nPatterns);

	//fprintf(stderr, " # %i %i\n", *number, nPatterns);

	if (head != 2) {
		free(buffer);
		return NULL;
	}

	for (i=0; i<nPatterns; ++i) {
		int multiplicity;
		struct ShallowGraph* string;
		fscanf(stream, "%i\t", &multiplicity);
		string = parseCString(stream, buffer, sgp);
		string->next = patterns;
		patterns = string;
	}	
	
	free(buffer);
	return patterns;
}

struct Vertex* loadSearchTree(FILE* stream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Vertex* root = getVertex(gp->vertexPool);
	int bufferSize = 100;
	streamBuildSearchTree(stream, root, bufferSize, gp, sgp);
	return root;
}
