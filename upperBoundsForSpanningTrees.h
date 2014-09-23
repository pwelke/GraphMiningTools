#ifndef UPPER_BOUNDS_FOR_SPANNING_TREES_H_
#define UPPER_BOUNDS_FOR_SPANNING_TREES_H_

long int nCr(long int n, long int r);
long int getEstimate(struct Graph* g, long int estimateFunction(long int, long int), char checkOuterplanarity, struct ShallowGraphPool* sgp, struct GraphPool* gp);
long int getGoodEstimate(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);
long int getGoodEstimatePrecomputedBlocks(struct Graph* g, struct ShallowGraph* biconnectedComponents, struct ShallowGraphPool* sgp, struct GraphPool* gp);
long int outerplanarBound(long int m, long int n);

#endif /* UPPER_BOUNDS_FOR_SPANNING_TREES_H_ */