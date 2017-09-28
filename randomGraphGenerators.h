/*
 * randomGraphGenerators.h
 *
 *  Created on: Sep 26, 2017
 *      Author: pascal
 */

#ifndef RANDOMGRAPHGENERATORS_H_
#define RANDOMGRAPHGENERATORS_H_

#include "graph.h"

void generateGaussianNoise(double* z0, double* z1, double mu, double sigma);
void generateIntegerGaussianNoise(int* z0, int* z1, double sigma);
void randomVertexLabels(struct Graph* g, int nVertexLabels);
void makeMinDegree1(struct Graph* g, struct GraphPool* gp);
double euclideanDistanceWrap(const int v, const int w, struct Graph* g);
double euclideanDistance(const int vx, const int vy, const int wx, const int wy);



struct Graph* erdosRenyi(int n, double p, struct GraphPool* gp);
struct Graph* erdosRenyiWithLabels(int n, double p, int nVertexLabels, int nEdgeLabels, struct GraphPool* gp);

struct Graph* barabasiAlbert(int n, int edgesAddedPerVertex, struct Graph* core, struct GraphPool* gp);
struct Graph* barabasiAlpha(int n, int edgesAddedPerVertex, double alpha, struct Graph* core, struct GraphPool* gp);

struct Graph* blockChainGenerator(int nBlocks, int blockSize, int nVertexLabels, int nEdgeLabels, double diagonalProbability, struct GraphPool* gp);

struct Graph* randomOverlapGraph(int n, double d, struct GraphPool* gp);
void moveOverlapGraph(struct Graph* g, double moveParameter, double d, struct GraphPool* gp);


#endif /* RANDOMGRAPHGENERATORS_H_ */
