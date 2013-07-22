#include <malloc.h>
#include <limits.h>

#include "graph.h"
#include "canonicalString.h"
#include "treeCenter.h"

int* getLeaves(struct Graph* tree) {
	int v;
	int i = 1;
	int* leaves = malloc((tree->n + 1) * sizeof(int));

	for (v=0; v<tree->n; ++v) {
		tree->vertices[v]->lowPoint = -1;

		if (isLeaf(tree->vertices[v])) {
			tree->vertices[v]->lowPoint = 0;
			leaves[i] = v;
			++i;
		}
	} 
	leaves[0] = i;
	return leaves;
}

void printParents(struct Graph* tree, int* parents) {
	int i;
	for(i=1; i<parents[0]; ++i) {
		if (parents[i] != -1) {
			printf("(%i, %i) ", parents[i], tree->vertices[parents[i]]->lowPoint);
		} else {
			printf("(%i, %i) ", parents[i], -1);
		}
	}
	printf("\n");
	fflush(stdout);
}

/**
leaves contains parents of leaf nodes or -1 if a parent already came up before 
*/
void getParents(int step, struct Graph* tree, int* leaves) {
	int v;
	struct VertexList* e;

	for (v=1; v<leaves[0]; ++v) {
		if (tree->vertices[leaves[v]]->lowPoint == step) {
			int neighbors = 0;
			struct Vertex* parent;
			for (e=tree->vertices[leaves[v]]->neighborhood; e!=NULL; e=e->next) {
				if ((e->endPoint->lowPoint == -1) || (e->endPoint->lowPoint >= step)) {
					++neighbors;
					parent = e->endPoint;
				}
			}
			if (neighbors == 1) {
				if (parent->lowPoint == step + 1) {
					leaves[v] = -1;
				} else {
					parent->lowPoint = step + 1;
					leaves[v] = parent->number;
				}
			} else {
				++tree->vertices[leaves[v]]->lowPoint;
				leaves[v] = -1;
			}
		} else {
			leaves[v] = -1;
		}
	}
}

void compressLeaves(int* leaves) {
	int i;
	int j;

	for (i=j=1; i<leaves[0]; ++i) {
		if (leaves[i] != -1) {
			leaves[j] = leaves[i];
			++j;
		}
	}
	leaves[0] = j;	
}

int* findTreeCenter(struct Graph* tree) {
	int step;
	int* leaves = getLeaves(tree);

	for (step=0; 1; ++step) {
		if (leaves[0] == 2) {
			/* if there is only a single leaf left, it is the center */
			break;
		}
		if (leaves[0] == 3) {
			/* if there are two vertices left, and they are connected, 
			they form the center togeter */
			if (existsEdge(tree, leaves[1], leaves[2])) {
				break;
			}
		}

		getParents(step, tree, leaves);

		// //debug
		// printf("p%i ", step);
		// printParents(tree, leaves);

		compressLeaves(leaves);
		
		// //debug
		// printf("c ");
		// printParents(tree, leaves);
	} 

	return leaves;
}


struct ShallowGraph* treeCenterCanonicalString(struct Graph* tree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* cString;
	int* center = findTreeCenter(tree);
	printParents(tree, center);


	cString = canonicalStringOfRootedTree(tree->vertices[center[1]], tree->vertices[center[1]], sgp);
	
	if (center[0] == 3) {
		struct ShallowGraph* cString2 = canonicalStringOfRootedTree(tree->vertices[center[2]], tree->vertices[center[2]], sgp);
		if (lexicographicComparison(cString, cString2) < 0) {
			dumpShallowGraph(sgp, cString2);
		} else {
			dumpShallowGraph(sgp, cString);
			cString = cString2;
		}
	}
	free(center);
	return cString;
}
