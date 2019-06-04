/*
 * subtreeIsomorphismSampling.h
 *
 *  Created on: Oct 12, 2018
 *      Author: pascal
 */

#ifndef SUBTREEISOMORPHISMSAMPLING_H_
#define SUBTREEISOMORPHISMSAMPLING_H_

#include "graph.h"

char subtreeIsomorphismSampler(struct Graph* g, struct Graph* h);
char subtreeIsomorphismSamplerWithImageShuffling(struct Graph* g, struct Graph* h);
char subtreeIsomorphismSamplerWithProperMatching(struct Graph* g, struct Graph* h, struct GraphPool* gp);
char subtreeIsomorphismSamplerWithSampledMaximumMatching(struct Graph* g, struct Graph* h, struct GraphPool* gp);

struct VertexList** shuffleNeighbors(struct Vertex* v, int degV);


#endif /* SUBTREEISOMORPHISMSAMPLING_H_ */
