#ifndef SAMPLE_SUBTREES_H_
#define SAMPLE_SUBTREES_H_ 

struct Graph* sampleSpanningTreeFromCactus(struct Graph* original, struct ShallowGraph* biconnectedComponents, struct GraphPool* gp);

struct ShallowGraph* sampleSpanningTreeEdgesFromCactus(struct ShallowGraph* biconnectedComponents, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTreesUsingWilson(struct Graph* g, int k, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTreesUsingKruskalOnce(struct Graph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTreesUsingKruskal(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTreesUsingListing(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTreesUsingMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTreesUsingPartialListingMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTreesUsingCactusMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* listBridgeForest(struct Graph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* listOrSampleSpanningTrees(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp);

void shuffle(struct VertexList** array, size_t n);

#endif