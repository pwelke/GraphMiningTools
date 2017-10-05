/*
 * randomGraphGenerators.c
 *
 *  Created on: Sep 26, 2017
 *      Author: pascal
 */

#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "loading.h"
#include "randomGraphGenerators.h"

// Utility Functions

/**
 * Box-Muller Transform to create two normally distributed values.
 * We are happy that two are generated, as we are interested in moving 2-d points
 * in our geometric threshold graphs and return both values right away.
 *
 * Source: https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
 *
 * Uses rand() 2 times
 */
void generateGaussianNoise(double* z0, double* z1, double mu, double sigma) {
	static const double epsilon = DBL_EPSILON;
	static const double two_pi = 2.0*3.14159265358979323846;

	double u1, u2;
	do {
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while ( u1 <= epsilon );

	*z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2) * sigma + mu;
	*z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2) * sigma + mu;
}


/**
 * Box-Muller Transform to create two normally distributed values.
 * We are happy that two are generated, as we are interested in moving 2-d points
 * in our geometric threshold graphs and return both values right away.
 *
 * mu (i.e., the mean is given by the initialized values of the two double in/out variables
 *
 * Source: https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
 *
 * Uses rand() 2 times
 */
void generateIntegerGaussianNoise(int* z0, int* z1, double sigma) {
	static const double epsilon = DBL_EPSILON;
	static const double two_pi = 2.0*3.14159265358979323846;

	double u1, u2;
	do {
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while ( u1 <= epsilon );

	*z0 = *z0 + (sqrt(-2.0 * log(u1)) * cos(two_pi * u2) * sigma) * RAND_MAX;
	*z1 = *z1 + (sqrt(-2.0 * log(u1)) * sin(two_pi * u2) * sigma) * RAND_MAX;
}


/**
 * Randomly assign vertex labels from 0 to nVertexLabels-1 to the vertices.
 *
 * Uses rand() g->n times
 */
void randomVertexLabels(struct Graph* g, int nVertexLabels) {
	int i;
	for (i=0; i<g->n; ++i) {
		g->vertices[i]->label = intLabel(rand() % nVertexLabels);
		g->vertices[i]->isStringMaster = 1;
	}
}


// TODO labels
void makeMinDegree1(struct Graph* g, struct GraphPool* gp) {
	for (int v=1; v<g->n; ++v) {
		if (g->vertices[v]->neighborhood == NULL) {
			addEdgeBetweenVertices(v, v-1, intLabel(1), g, gp);
			g->vertices[v]->neighborhood->isStringMaster = 1;
		}
	}
	if (g->vertices[0]->neighborhood == NULL) {
		addEdgeBetweenVertices(0,1,intLabel(1), g, gp);
		g->vertices[0]->neighborhood->isStringMaster = 1;
	}
}


// ERDOS RENYI Graph Generators

/**
 * Create a graph in the ER model.
 * The returned graph will have exactly n vertices and the probability of any edge being present is p.
 * Note that vertices and edges have NULL labels.
 *
 * Uses rand() n * (n-1) / 2 times
 */
struct Graph* erdosRenyi(int n, double p, struct GraphPool* gp) {
	struct Graph* g = createGraph(n, gp);
	for (int i=0; i<n; ++i) {
		for (int j=i+1; j<n; ++j) {
			double value = rand() / (RAND_MAX + 1.0);
			if (value < p) {
				addEdgeBetweenVertices(i, j, NULL, g, gp);
			}
		}
	}
	return g;
}

/**
* Create a graph in the ER model.
* The returned graph will have exactly n vertices and the probability of any edge being present is p.
* Vertices and edges have int labels that are in the specified ranges.
*
* Uses rand() n * (n-1) / 2 + n + m times
 */
struct Graph* erdosRenyiWithLabels(int n, double p, int nVertexLabels, int nEdgeLabels, struct GraphPool* gp) {
	struct Graph* g = createGraph(n, gp);
	randomVertexLabels(g, nVertexLabels);

	for (int i=0; i<n; ++i) {
		for (int j=i+1; j<n; ++j) {
			double value = rand() / (RAND_MAX + 1.0);
			if (value < p) {
				// add a labeled edge and set one of the two resulting vertex lists as string master.
				addEdgeBetweenVertices(i, j, intLabel(rand() % nEdgeLabels), g, gp);
				g->vertices[i]->neighborhood->isStringMaster = 1;
			}
		}
	}
	return g;
}


// Chain of Blocks Generator


/**
 * Create a labeled graph consisting of a chain of nBlocks biconnected blocks of blockSize vertices each.
 * Each block consists of a cycle and a random number of diagonals influenced by diagonalProbability.
 * Two consecutive blocks share a single vertex.
 * A block between two other blocks contains an edge (on its Hamiltonian cycle) that connects the two articulation vertices
 * where it is joined to its neighbor blocks.
 *
 * The resulting graph has nBlocks * blockSize - nBlocks + 1 vertices and at least nBlocks * blockSize edges.
 *
 * If nVertexLabels is smaller than 1, then the vertices will be injectively labeled.
 *
 * Uses rand()
 */
struct Graph* blockChainGenerator(int nBlocks, int blockSize, int nVertexLabels, int nEdgeLabels, double diagonalProbability, struct GraphPool* gp) {

	// create empty graph of correct size
	int nVertices = nBlocks * blockSize - nBlocks + 1;
	struct Graph* g = createGraph(nVertices, gp);

	// add vertex labels
	if (nVertexLabels < 1) {
		for (int v=0; v<nVertices; ++v) {
			g->vertices[v]->label = intLabel(v);
			g->vertices[v]->isStringMaster = 1;
		}
	} else {
		randomVertexLabels(g, nVertexLabels);
	}

	// add cycle edges
	for (int blockStart=0; blockStart<nVertices-1; blockStart+=blockSize-1) {
		for (int v=blockStart; v<blockStart+blockSize; ++v) {
			for (int w=v+1; w<blockStart+blockSize; ++w) {
				if ((w - v == 1) || (rand() / ((double)RAND_MAX) <= diagonalProbability)) {
					addEdgeBetweenVertices(v, w, intLabel(rand() % nEdgeLabels), g, gp);
				}
			}
		}
		if (!isIncident(g->vertices[blockStart], g->vertices[blockStart+blockSize-1])) {
			addEdgeBetweenVertices(blockStart, blockStart+blockSize-1, intLabel(rand() % nEdgeLabels), g, gp);
		}
	}
	return g;
}


// BARABASI ALBERT Variants

/**
 * Create a Graph according to one Formulation of the Barabasi Albert Graph Model.
 *
 * The core is consumed.
 */
struct Graph* barabasiAlpha(int n, int edgesAddedPerVertex, double alpha, struct Graph* core, struct GraphPool* gp) {
	// min degree of core needs to be 1
	if ((getMinDegree(core) == 0) || (n < core->n) || (edgesAddedPerVertex > core->n)) {
		return NULL;
	}

	//m <= m_0
	struct Graph* g = getGraph(gp);
	setVertexNumber(g, n);

	// TODO copying of core can be sped up by directly copying in initialized vertex array of g
	for (int v=0; v<core->n; ++v) {
		// copy vertex into g
		g->vertices[v] = core->vertices[v];
		core->vertices[v] = NULL;

		// store degree of vertex in ->d for fast access
		g->vertices[v]->d = degree(g->vertices[v]);

		// set label of v
		g->vertices[v]->label = intLabel(1);
		g->vertices[v]->isStringMaster = 1;
	}
	g->m = core->m;

	for (int v=core->n; v<n; ++v) {
		g->vertices[v] = getVertex(gp->vertexPool);
		g->vertices[v]->number = v;
		g->vertices[v]->label = intLabel(1);
		g->vertices[v]->isStringMaster = 1;
	}

	for (int v=core->n; v<n; ++v) {
		if (rand() <= alpha * RAND_MAX) {
			int w = rand() % v;
			addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
			g->vertices[v]->d += 1;
			g->vertices[w]->d += 1;
			g->vertices[v]->neighborhood->isStringMaster = 1;
		} else {
			for (int i=0; i<edgesAddedPerVertex; ++i) {
				int randV = rand() % (2 * g->m);
				int find = 0;
				for (int w=0; w<v; ++w) {
					find += g->vertices[w]->d;
					if (randV < find) {
						if (!isNeighbor(g, v, w)) {
							addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
							g->vertices[v]->d += 1;
							g->vertices[w]->d += 1;
							g->vertices[v]->neighborhood->isStringMaster = 1;
						}
						break;
					}
				}
			}
		}
	}
	return g;
}

// core is consumed
struct Graph* barabasiAlbert(int n, int edgesAddedPerVertex, struct Graph* core, struct GraphPool* gp) {
	// min degree of core needs to be 1
	if ((getMinDegree(core) == 0) || (n < core->n) || (edgesAddedPerVertex > core->n)) {
		return NULL;
	}

	//m <= m_0
	struct Graph* g = getGraph(gp);
	setVertexNumber(g, n);

	// TODO copying of core can be sped up by directly copying in initialized vertex array of g
	for (int v=0; v<core->n; ++v) {
		// copy vertex into g
		g->vertices[v] = core->vertices[v];
		core->vertices[v] = NULL;

		// store degree of vertex in ->d for fast access
		g->vertices[v]->d = degree(g->vertices[v]);

		// set label of v
		g->vertices[v]->label = intLabel(1);
		g->vertices[v]->isStringMaster = 1;
	}
	g->m = core->m;

	for (int v=core->n; v<n; ++v) {
		g->vertices[v] = getVertex(gp->vertexPool);
		g->vertices[v]->number = v;
		g->vertices[v]->label = intLabel(1);
		g->vertices[v]->isStringMaster = 1;
	}

	for (int v=core->n; v<n; ++v) {
		for (int i=0; i<edgesAddedPerVertex; ++i) {
			int randV = rand() % (2 * g->m);
			int find = 0;
			for (int w=0; w<v; ++w) {
				find += g->vertices[w]->d;
				if (randV < find) {
					if (!isNeighbor(g, v, w)) {
						addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
						g->vertices[v]->d += 1;
						g->vertices[w]->d += 1;
						g->vertices[v]->neighborhood->isStringMaster = 1;
					}
					break;
				}
			}
		}
	}
	return g;
}


// RANDOM GEOMETRIC GRAPHS

inline double euclideanDistance(const int vx, const int vy, const int wx, const int wy) {
	double vxx = vx / (double)RAND_MAX;
	double vyy = vy / (double)RAND_MAX;
	double wxx = wx / (double)RAND_MAX;
	double wyy = wy / (double)RAND_MAX;
	double xdiff = vxx - wxx;
	double ydiff = vyy - wyy;
	double result = sqrt(xdiff * xdiff + ydiff * ydiff);
	return result;
}

inline double euclideanDistanceWrap(const int v, const int w, struct Graph* g) {
	return euclideanDistance(g->vertices[v]->d, g->vertices[v]->lowPoint, g->vertices[w]->d, g->vertices[w]->lowPoint);
}


void printOverlapGraphDotFormat(struct Graph* g, FILE* out) {
	fprintf(out, "graph %i {\n", g->number);
	for (int v=0; v<g->n; ++v) {
		fprintf(out, "%i [label=%s, pos=\"%lf,%lf\", pin=true, shape=point];\n", v, g->vertices[v]->label, g->vertices[v]->d / (double)RAND_MAX, g->vertices[v]->lowPoint / (double)RAND_MAX);
	}
	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->startPoint->number < e->endPoint->number) {
				fprintf(out, "%i -- %i;\n", e->startPoint->number, e->endPoint->number);
			}
		}
	}
	fprintf(out, "}\n");
}


/**
 * Create a graph
 */
struct Graph* randomOverlapGraph(int n, double d, struct GraphPool* gp) {
	struct Graph* g = createGraph(n, gp);

	// every vertex is a two-dimensional point
	for (int v=0; v<n; ++v) {
		g->vertices[v]->d = rand();
		g->vertices[v]->lowPoint = rand();
		g->vertices[v]->label = intLabel(1);
	}
	// add edge iff distance is smaller than d
	for (int v=0; v<n; ++v) {
		for (int w=v+1; w<n; ++w) {
			if (euclideanDistanceWrap(v, w, g) < d) {
				addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
			}
		}
	}
	return g;
}


/**
 * Create a graph
 */
struct Graph* randomOverlapGraphWithLabels(int n, double d, int nVertexLabels, struct GraphPool* gp) {
	struct Graph* g = createGraph(n, gp);

	// every vertex is a two-dimensional point
	for (int v=0; v<n; ++v) {
		g->vertices[v]->d = rand();
		g->vertices[v]->lowPoint = rand();
		g->vertices[v]->label = intLabel(rand() % nVertexLabels);
	}
	// add edge iff distance is smaller than d
	for (int v=0; v<n; ++v) {
		for (int w=v+1; w<n; ++w) {
			if (euclideanDistanceWrap(v, w, g) < d) {
				addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
			}
		}
	}
	return g;
}


void moveVertexGaussian(struct Vertex* v, double moveParameter) {
	generateIntegerGaussianNoise(&(v->d), &(v->lowPoint), moveParameter);
	// mirror in unit interval
	// stupid undefined behavior of abs(INT_MIN) requires to do it ourselves:
	int a = v->d;
	int b = v->lowPoint;
	v->d = a == INT_MIN ? RAND_MAX : abs(a);
	v->lowPoint = b == INT_MIN ? RAND_MAX : abs(b);
}


/**
 * Create a graph
 */
struct Graph* randomClusteredOverlapGraphWithLabels(int n, double d, int nClusters, double mu, struct GraphPool* gp) {

	if (nClusters > n) {
		fprintf(stderr, "Number of clusters is larger than number of vertices: %i > %i\n", nClusters, n);
		return NULL;
	}

	// ensure that nClusters divides number of vertices.
	int nodesPerCluster = n / nClusters;
	if (n != nodesPerCluster * nClusters) {
		fprintf(stderr, "Randomly generated graph will have %i vertices instead of %i vertices\n", nodesPerCluster * nClusters, n);
		n = nodesPerCluster * nClusters;
	}

	struct Graph* g = createGraph(n, gp);

	// every vertex is a two-dimensional point
	int i=0;
	for (int v=0; v<nClusters; ++v) {
		g->vertices[i]->d = rand();
		g->vertices[i]->lowPoint = rand();
		g->vertices[i]->label = intLabel(v);
		g->vertices[i]->isStringMaster = 1;
		for (int w=1; w<nodesPerCluster; ++w) {
			g->vertices[i+w]->d = g->vertices[i]->d;
			g->vertices[i+w]->lowPoint = g->vertices[i]->lowPoint;
			g->vertices[i+w]->label = intLabel(v);
			g->vertices[i+w]->isStringMaster = 1;
			moveVertexGaussian(g->vertices[i+w], mu);
		}
		i += nodesPerCluster;
	}

	// add edge iff distance is smaller than d
	for (int v=0; v<n; ++v) {
		for (int w=v+1; w<n; ++w) {
			if (euclideanDistanceWrap(v, w, g) < d) {
				addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
				g->vertices[v]->neighborhood->isStringMaster = 1;
			}
		}
	}
	return g;
}




// Due to strangeness in A
void moveOverlapGraph(struct Graph* g, double moveParameter, double d, struct GraphPool* gp) {

	// move vertices
	for (int v=0; v<g->n; ++v) {
		moveVertexGaussian(g->vertices[v], moveParameter);
	}

	// dump edges
	for (int v=0; v<g->n; ++v) {
		dumpVertexListRecursively(gp->listPool, g->vertices[v]->neighborhood);
		g->vertices[v]->neighborhood = NULL;
	}
	g->m = 0;

	// add edge iff distance is smaller than d
	for (int v=0; v<g->n; ++v) {
		for (int w=v+1; w<g->n; ++w) {
			if (euclideanDistanceWrap(v, w, g) < d) {
				addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
				g->vertices[v]->neighborhood->isStringMaster = 1;
			}
		}
	}
}
