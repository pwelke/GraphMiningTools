#ifndef LOCAL_EASINESS_H_
#define LOCAL_EASINESS_H_ 

long int* computeLocalEasinessExactly(struct ShallowGraph* biconnectedComponents, int n, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int getMinLocalEasiness(struct Graph* g, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int getMaxLocalEasiness(struct Graph* g, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp);



#endif