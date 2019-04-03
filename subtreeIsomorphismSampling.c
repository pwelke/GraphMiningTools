#include <stdlib.h>
#include <stdio.h>
#include "subtreeIsomorphismSampling.h"
#include "subtreeIsoUtils.h"
#include "sampleSubtrees.h"
#include "bipartiteMatching.h"

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
	struct Vertex* rootEmbedding = g->vertices[rand() % g->n];

	char foundIso = 0;
	if (labelCmp(currentRoot->label, rootEmbedding->label) == 0) {
		// if the labels match, we map the root to the image
		currentRoot->visited = rootEmbedding->number + 1;
		rootEmbedding->visited = 1;
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
	struct Vertex* rootEmbedding = g->vertices[rand() % g->n];

	char foundIso = 0;
	if (labelCmp(currentRoot->label, rootEmbedding->label) == 0) {
		// if the labels match, we map the root to the image
		currentRoot->visited = rootEmbedding->number + 1;
		rootEmbedding->visited = 1;
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
	struct Vertex* rootEmbedding = g->vertices[rand() % g->n];
//	fprintf(stderr, "\nnew round: %i -> %i\n", currentRoot->number, rootEmbedding->number);

	char foundIso = 0;
	if (labelCmp(currentRoot->label, rootEmbedding->label) == 0) {
		// if the labels match, we map the root to the image
		currentRoot->visited = rootEmbedding->number + 1;
		rootEmbedding->visited = 1;
		// and try to embed the rest of the tree h into g accordingly
		foundIso = recursiveSubtreeIsomorphismSamplerWithMatching(currentRoot, g, gp);

		// cleanup
		cleanupSubtreeIsomorphismSampler(h, g);
	}

	return foundIso;
}


