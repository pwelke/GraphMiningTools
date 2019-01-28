#include <stdlib.h>
#include <stdio.h>
#include "subtreeIsomorphismSampling.h"
#include "subtreeIsoUtils.h"
#include "sampleSubtrees.h"

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
 * for the special case of trees. Basically, we try to embed the tree somewhere randomly
 * and output the probability of finding this embedding. In particular, the algorithm outputs
 * a value != 0 iff it has found an embedding.
 *
 * The algorithm runs in O(|V(h)| * max(degree(g))),
 * where max(degree(g)) is the maximum degree of any vertex in g.
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
 * for the special case of trees. Basically, we try to embed the tree somewhere randomly
 * and output the probability of finding this embedding. In particular, the algorithm outputs
 * a value != 0 iff it has found an embedding.
 *
 * The algorithm runs in O(|V(h)| * max(degree(g))),
 * where max(degree(g)) is the maximum degree of any vertex in g.
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


