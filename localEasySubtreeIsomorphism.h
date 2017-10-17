#ifndef LOCAL_EASY_SUBTREE_ISO
#define LOCAL_EASY_SUBTREE_ISO

#include "subtreeIsoDataStoreList.h"

struct PostorderList{
	int* postorder;
	struct PostorderList* next;
};

/*
 want a tree on roots.
 edges should go from parent to child
 [x] each vertex should store its parent vertex (where, in g or in the tree?)
 [x] each vertex should store all its v-rooted components
*/
struct BlockTree{
	struct Graph* g;
	struct Vertex** roots;
	struct Vertex** parents;
	struct ShallowGraph** vRootedBlocks;
	int nRoots;
};

/*
 *  [x] each vertex should store spanning trees of the v-rooted components
 *  each vertex should store the set of characteristics
 *  and maybe some pruning info?...
 */
struct SpanningtreeTree{
	struct Graph* g;
	struct Vertex** roots;
	struct Vertex** parents;
	struct Graph** localSpanningTrees;
	struct SubtreeIsoDataStoreList** characteristics;
	struct PostorderList** localPostorders;
	int nRoots;
};


struct BlockTree getBlockTreeT(struct Graph* g, struct ShallowGraphPool* sgp);
struct Graph* blockConverter(struct ShallowGraph* edgeList, struct GraphPool* gp);
struct Graph* spanningTreeConverter(struct ShallowGraph* localTrees, struct Graph* component, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SpanningtreeTree getSampledSpanningtreeTree(struct BlockTree blockTree, int spanningTreesPerBlock, char removeDuplicates, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SpanningtreeTree getFullSpanningtreeTree(struct BlockTree blockTree, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void dumpSpanningtreeTree(struct SpanningtreeTree sptTree, struct GraphPool* gp);

void initCharacteristicsArrayForLocalEasy(struct SpanningtreeTree* sptTree);
void wipeCharacteristicsForLocalEasy(struct SpanningtreeTree sptTree);

char subtreeCheckForSpanningtreeTree(struct SpanningtreeTree* sptTree, struct Graph* h, struct GraphPool* gp);
char isProbabilisticLocalSampleSubtree(struct Graph* g, struct Graph* h, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char isLocalEasySubtree(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);

int getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySampling(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySamplingWithFiltering(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int getNumberOfSpanningTreesObtainedByLocalEasySamplingWithFiltering(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int getNumberOfSpanningTreesObtainedByLocalEasySampling(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp);
#endif
