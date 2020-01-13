#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include "subtreeIsomorphismSampling.h"
#include "subtreeIsoUtils.h"
#include "sampleSubtrees.h"
#include "bipartiteMatching.h"
#include "intMath.h"

/*
 * subtreeIsomorphismSampling.c
 *
 *  Created on: Oct 12, 2018
 *      Author: pascal
 */

/**
 * create a shuffled array of the elements of v->neighbors.
 */
struct VertexList** shuffleNeighbors(struct Vertex* v, int degV) {
	if (degV > 0) {
		struct VertexList** edgeArray = malloc(degV * sizeof(struct VertexList*));
		edgeArray[0] = v->neighborhood;
		for (int i=1; i < degV; ++i) {
			edgeArray[i] = edgeArray[i-1]->next;
		}
		shuffle(edgeArray, degV);
		return edgeArray;
	}
	return NULL;
}


/**
 * mixed bfs/dfs strategy. embed all children of a vertex, then call recursively.
 * similar to gaston dfs strategy in the pattern space, but for a single tree in a fixed graph.
 */
static char recursiveSubtreeIsomorphismSampler(struct Vertex* parent, struct Graph* g) {

	struct Vertex* currentImage = g->vertices[parent->visited - 1];

	int nNeighbors = degree(parent);
	struct VertexList** shuffledNeighbors = shuffleNeighbors(parent, nNeighbors);

	// TODO need to shuffle the neighbors of the image, as well
	int unassignedNeighborsOfRoot = 0;
	int processedNeighborsOfRoot = 0;

	for (int i=0; i<nNeighbors; ++i) {
		struct VertexList* child = shuffledNeighbors[i];
		if (child->endPoint->visited == 0) {
			++unassignedNeighborsOfRoot;
			++processedNeighborsOfRoot;
			for (struct VertexList* embeddingEdge=currentImage->neighborhood; embeddingEdge!=NULL; embeddingEdge=embeddingEdge->next) {
				if ((embeddingEdge->endPoint->visited == 0)
						&& (labelCmp(child->label, embeddingEdge->label) == 0)
						&& (labelCmp(child->endPoint->label, embeddingEdge->endPoint->label) == 0)) {

					// if the child vertex and edge labels fit, map the vertices to each other, mark it as a novel assignment
					child->endPoint->visited = embeddingEdge->endPoint->number + 1;
					child->flag = 1;
					embeddingEdge->endPoint->visited = 1;

					// and remember that the child vertex was assigned
					--unassignedNeighborsOfRoot;
					break; //...looping over the neighbors of the image of parent in g
				}
			}
		}
	}

	// check, if we could assign all neighbors of the currentRoot to some neighbors of its image.
	if (unassignedNeighborsOfRoot > 0) {
		free(shuffledNeighbors);
		return 0;
	}

	// if none of the neighbors needs to be newly asigned, we have embedded this branch.
	if (processedNeighborsOfRoot == 0) {
		free(shuffledNeighbors);
		return 1;
	}

	// if new neighbors were assigned, recurse to the newly assigned neighbors
	char embeddingWorked = 1;
	for (int i=0; i<nNeighbors; ++i) {
		struct VertexList* child = shuffledNeighbors[i];
		if (child->flag) {
			child->flag = 0;
			embeddingWorked = recursiveSubtreeIsomorphismSampler(child->endPoint, g);
		}
		if (!embeddingWorked) {
			break;
		}
	}
	free(shuffledNeighbors);
	return embeddingWorked;
}


/**
 * mixed bfs/dfs strategy. embed all children of a vertex, then call recursively.
 * similar to gaston dfs strategy in the pattern space, but for a single tree pattern
 * in a fixed graph transaction.
 */
static char recursiveSubtreeIsomorphismSamplerWithShuffledImage(struct Vertex* parent, struct Graph* g) {

	struct Vertex* currentImage = g->vertices[parent->visited - 1];

	int nNeighbors = degree(parent);
	struct VertexList** shuffledNeighbors = shuffleNeighbors(parent, nNeighbors);

	int nImageNeighbors = degree(currentImage);
	struct VertexList** shuffledImageNeighbors = shuffleNeighbors(currentImage, nImageNeighbors);

	int unassignedNeighborsOfRoot = 0;
	int processedNeighborsOfRoot = 0;

	for (int i=0; i<nNeighbors; ++i) {
		struct VertexList* child = shuffledNeighbors[i];
		if (child->endPoint->visited == 0) {
			++unassignedNeighborsOfRoot;
			++processedNeighborsOfRoot;
			for (int j=0; j<nImageNeighbors; ++j) {
				struct VertexList* embeddingEdge = shuffledImageNeighbors[j];
				if ((embeddingEdge->endPoint->visited == 0)
						&& (labelCmp(child->label, embeddingEdge->label) == 0)
						&& (labelCmp(child->endPoint->label, embeddingEdge->endPoint->label) == 0)) {

					// if the child vertex and edge labels fit, map the vertices to each other, mark it as a novel assignment
					child->endPoint->visited = embeddingEdge->endPoint->number + 1;
					child->flag = 1;
					embeddingEdge->endPoint->visited = 1;

					// and remember that the child vertex was assigned
					--unassignedNeighborsOfRoot;
					break; //...looping over the neighbors of the image of parent in g
				}
			}
		}
	}

	// check, if we could assign all neighbors of the currentRoot to some neighbors of its image.
	if (unassignedNeighborsOfRoot > 0) {
		free(shuffledNeighbors);
		free(shuffledImageNeighbors);
		return 0;
	}

	// if none of the neighbors needs to be newly asigned, we have embedded this branch.
	if (processedNeighborsOfRoot == 0) {
		free(shuffledNeighbors);
		free(shuffledImageNeighbors);
		return 1;
	}

	// if new neighbors were assigned, recurse to the newly assigned neighbors
	char embeddingWorked = 1;
	for (int i=0; i<nNeighbors; ++i) {
		struct VertexList* child = shuffledNeighbors[i];
		if (child->flag) {
			child->flag = 0;
			embeddingWorked = recursiveSubtreeIsomorphismSampler(child->endPoint, g);
		}
		if (!embeddingWorked) {
			break;
		}
	}
	free(shuffledNeighbors);
	free(shuffledImageNeighbors);
	return embeddingWorked;
}


/* may only be called if parent has at least one neighbor that is not yet matched to some image vertex.
 *
 * vertices of g have their ->visited values set to 1 if they are already matched. vertices of the tree h have their ->visited
 * values set to their image->number + 1.  Thus, we can match to all vertices that have ->visited = 0 */
static struct Graph* makeBipartiteInstanceFromVertices(struct Vertex* parent, struct Vertex* parentImage, struct GraphPool* gp) {

	/* shuffle neighbors of parent and parentImage */
	int nNeighbors = degree(parent);
	struct VertexList** shuffledNeighbors = shuffleNeighbors(parent, nNeighbors);

	int nImageNeighbors = degree(parentImage);
	struct VertexList** shuffledImageNeighbors = shuffleNeighbors(parentImage, nImageNeighbors);

	/* construct bipartite graph B(v,u) */
	struct Graph* B = createGraph(nNeighbors + nImageNeighbors, gp);

	/* add vertex numbers of original vertices to ->lowPoint of each vertex in B
	and keep a pinter to the edge to check later whether labels of edge and endpoint match.
	if parent is not the root of h, then there exists a neighbor in shuffledNeighbors that is already matched.
	if so, don't copy it to the list. */
	int i, j;
	for (i=0, j=0; j<nNeighbors; ++i, ++j) {
		if (shuffledNeighbors[j]->endPoint->visited != 0) {
			--i;
			continue;
		}
		B->vertices[i]->lowPoint = shuffledNeighbors[j]->endPoint->number;
		B->vertices[i]->label = ((char*)shuffledNeighbors[j]); // yeah, this is nasty!
	}
	/* store size of first partitioning set (without matched vertex, if it exists) */
	if (i == nNeighbors - 1) {
		B->number = i;
		nNeighbors = i;
		B->n -= 1;
		// dump last free vertex
		dumpVertex(gp->vertexPool, B->vertices[B->n]);
	} else {
		B->number = nNeighbors;
	}

	for (i=0; i<nImageNeighbors; ++i) {
		B->vertices[i+nNeighbors]->lowPoint = shuffledImageNeighbors[i]->endPoint->number;
		B->vertices[i+nNeighbors]->label = ((char*)shuffledImageNeighbors[i]); // yeah, this is nasty!
	}

	// garbage collection
	free(shuffledNeighbors);
	free(shuffledImageNeighbors);

	/* add edge (x,y) if edge and vertex label match and y is not yet used by the embedding */
	for (i=0; i<nNeighbors; ++i) {
		struct VertexList* xEdge = ((struct VertexList*)B->vertices[i]->label);
		for (j=nNeighbors; j<B->n; ++j) {
			struct VertexList* yEdge = ((struct VertexList*)B->vertices[j]->label);
			/* y has to be free */
			if (yEdge->endPoint->visited == 0) {
				if (labelCmp(xEdge->label, yEdge->label) == 0) {
					if (labelCmp(xEdge->endPoint->label, yEdge->endPoint->label) == 0) {
						addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
						++B->m;
					}
				}
			}
		}
	}

	return B;
}


/**
 * mixed bfs/dfs strategy. embed all children of a vertex, then call recursively.
 * similar to gaston dfs strategy in the pattern space, but for a single tree pattern
 * in a fixed graph transaction.
 */
static char recursiveSubtreeIsomorphismSamplerWithMatching(struct Vertex* parent, struct Graph* g, struct GraphPool* gp) {

	// base case: if parent is a leaf and its only neighbor is already assigned to a vertex in g, we are done
	if (isLeaf(parent) && parent->neighborhood->endPoint->visited) { return 1; }

	struct Vertex* parentImage = g->vertices[parent->visited - 1];
	struct Graph* B = makeBipartiteInstanceFromVertices(parent, parentImage, gp);
	int sizeOfMatching = bipartiteMatchingEvenMoreDirty(B);
	int nNeighbors = B->number;

	// is there a matching here covering all unmatched neighbors of parent?
	// if not, there is no subgraph iso here
	if (sizeOfMatching != nNeighbors) {
		dumpGraph(gp, B);
		return 0;
	}

	// mark all matched neighbors of currentImage as ->visited
	for (int i=0; i<nNeighbors; ++i) {
		// find match
		for (struct VertexList* e=B->vertices[i]->neighborhood; e!=NULL; e=e->next) {
			if (e->flag == 1) {
				// mark embedding vertex as mapped to current vertex
				((struct VertexList*)B->vertices[i]->label)->endPoint->visited = e->endPoint->lowPoint + 1;
				g->vertices[e->endPoint->lowPoint]->visited = 1;
				break;
			}
		}
	}

	// recurse to the matched vertices
	for (int i=0; i<nNeighbors; ++i) {
		for (struct VertexList* e=B->vertices[i]->neighborhood; e!=NULL; e=e->next) {
			if (e->flag == 1) {
				struct Vertex* newChild = ((struct VertexList*)e->startPoint->label)->endPoint;
				char embeddingWorked = recursiveSubtreeIsomorphismSamplerWithMatching(newChild, g, gp);
				if (embeddingWorked) {
					// we are done with this neighbor of parent
					break;
				} else {
					// there is no subtree iso here
					dumpGraph(gp, B);
					return 0;
				}
			}
		}
	}

	dumpGraph(gp, B);
	// so far, we have been successful
	return 1;
}


/**
 * create an array of the elements of v->neighbors.
 */
struct VertexList** getUncoveredNeighborArray(struct Vertex* v, int* uncoveredDegree) {
	*uncoveredDegree = 0;
	for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->visited == 0) {
			++(*uncoveredDegree);
		}
	}
	if (*uncoveredDegree > 0) {
		struct VertexList** edgeArray = malloc(*uncoveredDegree * sizeof(struct VertexList*));
		int i=0;
		for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->visited == 0) {
				edgeArray[i] = e;
				++i;
			}
		}
		return edgeArray;
	} else {
		return NULL;
	}
}


/**
Computes the order of two edges with a label on the edge and a label on the endPoint.
As we assume the label of the startPoints to be equal for all edges in the array, we do not check that!

This function returns
\[ negative value if P1 < P2 \]
\[ 0 if P1 = P2 \]
\[ positive value if P1 > P2 \]
and uses the comparison of label strings as total ordering.

It is compatible with qsort (hence, you have to pass VertexList** pointers to this function)
 */
static int compareLabeledNeighbors(const void* p1, const void* p2) {
	struct VertexList* e1 = *(struct VertexList**)p1;
	struct VertexList* e2 = *(struct VertexList**)p2;

	int cmp = labelCmp(e1->label, e2->label);
	if (cmp == 0) {
		return labelCmp(e1->endPoint->label, e2->endPoint->label);
	} else {
		return cmp;
	}
}


/**
 * TODO We can get rid of this function, as it is only required for shuffleInClasses below
 */
static inline int* getClassesArray(struct VertexList** neighbors, int deg) {
	int nClasses = 1;
	for (int i=0; i<deg-1; ++i) {
		if (compareLabeledNeighbors(&(neighbors[i]), &(neighbors[i+1])) != 0) {
			++nClasses;
		}
	}
	int* classesArray = malloc((nClasses + 1) * sizeof(int));
	classesArray[0] = nClasses;
	int classCounter = 1;
	for (int i=0; i<deg-1; ++i) {
		if (compareLabeledNeighbors(&(neighbors[i]), &(neighbors[i+1])) != 0) {
			classesArray[classCounter] = i + 1;
			++classCounter;
		}
	}
	assert(classCounter == nClasses);

	classesArray[nClasses] = deg;
	return classesArray;
}


/**
 * Shuffle the image neighbors; separately in each block (i.e., swap only items that have identical vertex and edge labels)
 * This is where the magic happens to generate maximum matchings uniformly at random for our particular situation.
 */
static void shuffleInClasses(struct VertexList** imageNeighbors, int* imageNeighborClasses) {
	int startIdx = 0;
	for (int i=1; i<imageNeighborClasses[0] + 1; ++i) {
		int size = imageNeighborClasses[i] - startIdx;
		shuffle(&(imageNeighbors[startIdx]), size);
		startIdx = imageNeighborClasses[i];
	}
}


/**
 *  Compute the number of maximum matchings in a complete bipartite graph on A + B vertices.
    A and B are required to be nonnegative.
    Corner Case: If at least one of the sides has zero vertices, there is one maximal matching: the empty set.
 */
static int n_max_matchings_complete(int A, int B) {

	int a = min(A, B);
	int b = max(A, B);
	int c = 1;

	if (a != 0) {
		for (int i=b-a+1; i<=b; ++i) {
			c *= i;
		}
	}

	return c;
}

int n_max_matchings() {
return 0;
}


static const void* EMPTY_MATCHING = ((void*)0x1);

static struct VertexList** uniformBlockMaximumMatching(struct Vertex* parent, struct Vertex* image, int* matchingSize, int* n_matchings, int computeEstimate) {
	int nUncoveredChildren;
	int nCandidateImages;
	struct VertexList** uncoveredChildren = getUncoveredNeighborArray(parent, &nUncoveredChildren);
	struct VertexList** candidateImages = getUncoveredNeighborArray(image, &nCandidateImages);

	if (nUncoveredChildren == 0) {
		// here we don't need to find a matching any more, as there are no neighbors left to be assigned.
		free(uncoveredChildren);
		free(candidateImages);
		*matchingSize = 0;
		return (void*)EMPTY_MATCHING; // encode maximum matching has size zero, but that is fine...
	}

	if (nUncoveredChildren > nCandidateImages) {
		// here, there can be no matching, as there are more uncovered neighbors than candidate images
		free(uncoveredChildren);
		free(candidateImages);
		*matchingSize = 0;
		return NULL;
	}

	assert(nUncoveredChildren > 0);
	assert(nCandidateImages > 0);
	assert(nUncoveredChildren <= nCandidateImages);

	qsort(uncoveredChildren, nUncoveredChildren, sizeof(struct VertexList*), &compareLabeledNeighbors);
	qsort(candidateImages, nCandidateImages, sizeof(struct VertexList*), &compareLabeledNeighbors);

	int* candidateImageClasses = getClassesArray(candidateImages, nCandidateImages);
	shuffleInClasses(candidateImages, candidateImageClasses);

	int currentChild = 0;
	int currentCandidate = 0;

	while ((currentChild < nUncoveredChildren)
			&& (currentCandidate < nCandidateImages)) {
		if (compareLabeledNeighbors(&(uncoveredChildren[currentChild]), &(candidateImages[currentCandidate])) == 0) {
			uncoveredChildren[currentChild]->endPoint->visited = candidateImages[currentCandidate]->endPoint->number + 1;
			candidateImages[currentCandidate]->endPoint->visited = 1;
			++currentChild;
			++currentCandidate;
		} else {
			++currentCandidate;
		}
	}

	if ((currentChild == nUncoveredChildren) && computeEstimate) {
		//matching worked, compute number of maximum matchings
		int A = 0;
		int B = 0;
		char newBlock = 1;
		*n_matchings = 1;

		currentChild = 0;
		currentCandidate = 0;

		while ((currentChild < nUncoveredChildren)
					&& (currentCandidate < nCandidateImages)) {
				if (compareLabeledNeighbors(&(uncoveredChildren[currentChild]), &(candidateImages[currentCandidate])) == 0) {
					if (newBlock) {
						newBlock = 0;
						(*n_matchings) *= n_max_matchings_complete(A, B);
						A = 0;
						B = 0;
					}
					++currentChild;
					++currentCandidate;
					++A;
					++B;
				} else {
					++currentCandidate;
					++B;
					newBlock = 1;
				}
			}
	}

	// garbage collection
	free(candidateImages);
	free(candidateImageClasses);

	if (currentChild == nUncoveredChildren) {
		//matching worked
		*matchingSize = nUncoveredChildren;
		// shuffle *covered* children to randomize the dfs/bfs strategy in case we hit the same image vertex twice in different 
		// runs of the embedding algorithm
		shuffle(uncoveredChildren, nUncoveredChildren);
		return uncoveredChildren; // which are now covered :)
	} else {
		free(uncoveredChildren);
		*matchingSize = 0;
		return NULL;
	}
}


/**
 * mixed bfs/dfs strategy. embed all children of a vertex, then call recursively.
 * similar to gaston dfs strategy in the pattern space, but for a single tree pattern
 * in a fixed graph transaction.
 */
static int recursiveSubtreeIsomorphismSamplerWithSampledMaximumMatching(struct Vertex* parent, struct Graph* g, struct GraphPool* gp, int computeEstimate) {

	struct Vertex* parentImage = g->vertices[parent->visited - 1];

	int matchingSize;
	int n_matchings = 0;
	struct VertexList** maximumMatching = uniformBlockMaximumMatching(parent, parentImage, &matchingSize, &n_matchings, computeEstimate);

	// base case: if there are no children that need to be matched, although parent is not a leaf, we are happy
	if (maximumMatching == EMPTY_MATCHING) {
		return 1;
	}

	// is there a matching here covering all unmatched neighbors of parent?
	// if not, there is no subgraph iso here
	if (maximumMatching == NULL) {
		return 0;
	}

	// recurse to the matched vertices
	for (int i=0; i<matchingSize; ++i) {
		int embeddingWorked = recursiveSubtreeIsomorphismSamplerWithSampledMaximumMatching(maximumMatching[i]->endPoint, g, gp, computeEstimate);
		if (!embeddingWorked) {
			free(maximumMatching);
			return 0;
		} else {
			n_matchings *= embeddingWorked;
		}
	}

	// so far, we have been successful
	free(maximumMatching);
	if (computeEstimate) {
		return n_matchings;
	} else {
		return 1;
	}
}


static void cleanupSubtreeIsomorphismSampler(struct Graph* h, struct Graph* g) {
	for (int v=0; v<h->n; ++v) {
		int image = h->vertices[v]->visited;
		h->vertices[v]->visited = 0;
		if (image) {
			g->vertices[image-1]->visited = 0;
		}
	}
}


/**
 * An implementation of the idea of the randomized embedding operator in
 *
 * Fürer, M. & Kasiviswanathan, S. P.:
 * Approximately Counting Embeddings into Random Graphs
 * in Combinatorics, Probability & Computing, 2014, 23, 1028-1056
 *
 * for the special case of trees. Basically, they try to embed the tree somewhere randomly
 * and output the probability of finding this embedding. In particular, the algorithm outputs
 * a value != 0 iff it has found an embedding. We forget about the probabilities and just
 * remember whether we have found an embedding by chance.
 *
 * The algorithm runs in O(|V(h)| * max(degree(h) * max(degree(g))),
 * where max(degree(g)) is the maximum degree of any vertex in g. Actually, I believe that the
 * same trick used to prove the runtime of Shamir & Tsur's simple subtree iso variant can
 * be applied here to give a runtime of O(|V(h)| * max(degree(g))).
 * 
 * The algorithm shuffles the neighbors of the current vertex in h before computing a maximal
 * matching greedily.
 *
 * The algorithm requires
 * - g->vertices[v]->visited = 0 for all v \in V(g).
 * - h to be a tree
 * - e->flag to be initialized to 0 for all edges e in h
 * - rand() to be initialized and working.
 *
 * The algorithm guarantees
 * - output != 0 iff it has found a subgraph isomorphism from h to g
 * - g->vertices[v]->visited = 0 for all v \in V(g) after termination
 */
char subtreeIsomorphismSampler(struct Graph* g, struct Graph* h) {

	// we root h at a random vertex
	struct Vertex* currentRoot = h->vertices[rand() % h->n];
	// and select a random image vertex
	struct Vertex* rootImage = g->vertices[rand() % g->n];

	char foundIso = 0;
	if (labelCmp(currentRoot->label, rootImage->label) == 0) {
		// if the labels match, we map the root to the image
		currentRoot->visited = rootImage->number + 1;
		rootImage->visited = 1;
		// and try to embed the rest of the tree h into g accordingly
		foundIso = recursiveSubtreeIsomorphismSampler(currentRoot, g);

		// cleanup
		cleanupSubtreeIsomorphismSampler(h, g);
	}

	return foundIso;
}



/**
 * An implementation of the idea of the randomized embedding operator in
 *
 * Fürer, M. & Kasiviswanathan, S. P.:
 * Approximately Counting Embeddings into Random Graphs
 * in Combinatorics, Probability & Computing, 2014, 23, 1028-1056
 *
 * for the special case of trees. Basically, they try to embed the tree somewhere randomly
 * and output the probability of finding this embedding. In particular, the algorithm outputs
 * a value != 0 iff it has found an embedding. We forget about the probabilities and just
 * remember whether we have found an embedding by chance.
 *
 * The algorithm runs in O(|V(h)| * max(degree(h) * max(degree(g))),
 * where max(degree(g)) is the maximum degree of any vertex in g. Actually, I believe that the
 * same trick used to prove the runtime of Shamir & Tsur's simple subtree iso variant can
 * be applied here to give a runtime of O(|V(h)| * max(degree(g))).
 *
 * This variant, in contrast to the one above, shuffles the neighbors of the current vertex
 * and the neighbors of its image, before computing a maximal matching greedily. This should result
 * in 'more randomness' and hence, when used repeatedly, in lower one-sided error of the 
 * embedding operator.
 *
 * The algorithm requires
 * - g->vertices[v]->visited = 0 for all v \in V(g).
 * - h to be a tree
 * - e->flag to be initialized to 0 for all edges e in h
 * - rand() to be initialized and working.
 *
 * The algorithm guarantees
 * - output != 0 iff it has found a subgraph isomorphism from h to g
 * - g->vertices[v]->visited = 0 for all v \in V(g) after termination
 */
char subtreeIsomorphismSamplerWithImageShuffling(struct Graph* g, struct Graph* h) {

	// we root h at a random vertex
	struct Vertex* currentRoot = h->vertices[rand() % h->n];
	// and select a random image vertex
	struct Vertex* rootImage = g->vertices[rand() % g->n];

	char foundIso = 0;
	if (labelCmp(currentRoot->label, rootImage->label) == 0) {
		// if the labels match, we map the root to the image
		currentRoot->visited = rootImage->number + 1;
		rootImage->visited = 1;
		// and try to embed the rest of the tree h into g accordingly
		foundIso = recursiveSubtreeIsomorphismSamplerWithShuffledImage(currentRoot, g);

		// cleanup
		cleanupSubtreeIsomorphismSampler(h, g);
	}

	return foundIso;
}


/**
 * An implementation of the idea of the randomized embedding operator in
 *
 * Fürer, M. & Kasiviswanathan, S. P.:
 * Approximately Counting Embeddings into Random Graphs
 * in Combinatorics, Probability & Computing, 2014, 23, 1028-1056
 *
 * for the special case of trees. Basically, they try to embed the tree somewhere randomly
 * and output the probability of finding this embedding. In particular, the algorithm outputs
 * a value != 0 iff it has found an embedding. We forget about the probabilities and just
 * remember whether we have found an embedding by chance.
 *
 * This variant computes a maximum matching for the children of each vertex. (In contrast, the other
 * variants above only compute maximal matchings.)
 *
 * The algorithm requires
 * - g->vertices[v]->visited = 0 for all v \in V(g).
 * - h to be a tree
 * - e->flag to be initialized to 0 for all edges e in h
 * - rand() to be initialized and working.
 *
 * The algorithm guarantees
 * - output != 0 iff it has found a subgraph isomorphism from h to g
 * - g->vertices[v]->visited = 0 for all v \in V(g) after termination
 */
char subtreeIsomorphismSamplerWithProperMatching(struct Graph* g, struct Graph* h, struct GraphPool* gp) {

	// we root h at a random vertex
	struct Vertex* currentRoot = h->vertices[rand() % h->n];
	// and select a random image vertex
	struct Vertex* rootImage = g->vertices[rand() % g->n];
	//	fprintf(stderr, "\nnew round: %i -> %i\n", currentRoot->number, rootEmbedding->number);

	char foundIso = 0;
	if (labelCmp(currentRoot->label, rootImage->label) == 0) {
		// if the labels match, we map the root to the image
		currentRoot->visited = rootImage->number + 1;
		rootImage->visited = 1;
		// and try to embed the rest of the tree h into g accordingly
		foundIso = recursiveSubtreeIsomorphismSamplerWithMatching(currentRoot, g, gp);

		// cleanup
		cleanupSubtreeIsomorphismSampler(h, g);
	}

	return foundIso;
}


static struct Vertex* getSuitableImage(struct Vertex* root, struct Graph* g, struct ListPool* lp, int* nCandidates) {
	*nCandidates = 0;
	struct VertexList* candidates = NULL;
	for (int v=0; v<g->n; ++v) {
		if (labelCmp(root->label, g->vertices[v]->label) == 0) {
			struct VertexList* tmp = getVertexList(lp);
			tmp->next = candidates;
			tmp->endPoint = g->vertices[v];

			candidates = tmp;
			++(*nCandidates);
		}
	}
	struct Vertex* image = NULL;
	if (*nCandidates > 0) {
		int index = rand() % (*nCandidates);
		struct VertexList* e = candidates;
		for (int i=0; i<index; ++i) {
			e=e->next;
		}
		image = e->endPoint;
		// garbage collection
		dumpVertexListLinearly(lp, candidates);
	}
	return image;
}


/**
 * This algorithm, similar to the variants above, tries to randomly embed a given tree pattern into a given graph.
 *
 * It uses the fact that the matching instances that we have to solve for each pair of pattern vertex and selected
 * image has a certain structure. This allows to sample a maximum matching that covers the not yet mapped children
 * of the pattern vertex, uniformly at random (if one exists) via a very simple process (and return NO, if no such
 * matching exists).
 *
 * This is an implementation of the idea of the randomized embedding operator in
 *
 * Fürer, M. & Kasiviswanathan, S. P.:
 * Approximately Counting Embeddings into Random Graphs
 * in Combinatorics, Probability & Computing, 2014, 23, 1028-1056
 *
 * for the special case of trees, without the necessity to compute all maximum matchings for each pattern vertex.
 *
 * The algorithm requires
 * - g->vertices[v]->visited = 0 for all v \in V(g).
 * - h to be a tree
 * - e->flag to be initialized to 0 for all edges e in h
 * - rand() to be initialized and working.
 *
 * The algorithm guarantees
 * - output != 0 iff it has found a subgraph isomorphism from h to g
 * - g->vertices[v]->visited = 0 for all v \in V(g) after termination
 */
int subtreeIsomorphismSamplerWithSampledMaximumMatching(struct Graph* g, struct Graph* h, struct GraphPool* gp, int computeEstimate) {

	// we root h at a random vertex
	struct Vertex* currentRoot = h->vertices[rand() % h->n];
	// and select a random image vertex with suitable label (if one exists)
	int nRootImageCandidates;
	struct Vertex* rootImage = getSuitableImage(currentRoot, g, gp->listPool, &nRootImageCandidates);
	//	struct Vertex* rootImage = g->vertices[rand() % g->n];


	int foundIso = 0;
	if (rootImage) {
		//	if (labelCmp(currentRoot->label, rootImage->label) == 0) {
		// if there is a candidate image we map the root to the image
		currentRoot->visited = rootImage->number + 1;
		rootImage->visited = 1;
		// and try to embed the rest of the tree h into g accordingly
		foundIso = recursiveSubtreeIsomorphismSamplerWithSampledMaximumMatching(currentRoot, g, gp, computeEstimate);

		// cleanup
		cleanupSubtreeIsomorphismSampler(h, g);
	}

	if (computeEstimate) {
		return foundIso * nRootImageCandidates;
	} else {
		return foundIso;
	}
}
