#include <malloc.h>
#include "graph.h"



struct RootedTree {
	struct Vertex* root;
	int n;
};

struct Table {
	void* data;
	int itemSize;
	int n;
	int m;
};

struct Table* getTable(int n, int m, int size) {
	struct Table* table = malloc(sizeof(struct Table));
	table->data = malloc(n * m * size);
	table->n = n;
	table->m = m;
	table->itemSize = size;
	return table;
}

int dumpTable(struct Table* table) {
	free(table->data);
	free(table);
}

void* getValue(struct Table* table, int i, int j) {
	int pos = (i * table->n + j) * table->itemSize;
	/* cast to char to have pointer arithmetic for one byte steps */
	return &(((char*)table->data)[pos]);
}

void setValue(struct Table* table, int i, int j, int value) {
	int pos = (i * table->n + j) * table->itemSize;
	/* cast to char to have pointer arithmetic for one byte steps */
	((char*)table->data)[pos] = value;
} 

struct Table* recursiveSubtreeCheck(struct Vertex* v, struct Vertex* pv, struct Graph* H, struct Table* I, struct GraphPool* gp) {
	struct VertexList* e;
	struct Table** childTables;
	
	/* count number of children of v in G */
	int children = 0;
	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (!(e->endPoint == pv)) {
			++children;
		}
	}

	/* leaf case. return map */ 
	if (children == 0) {
		// TODO
		struct Table* Svu = getTable(0,0,1);
	}

	/* otherwise: compute maps for children recursively */
	childTables = malloc(children * sizeof(struct Table*));
	children = 0;
	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (!(e->endPoint == pv)) {
			childTables[children] = recursiveSubtreeCheck(e->endPoint, v, H, I, gp);
		++children;
		}
	}

	/* do the matching */
	// TODO
}

/**
g is the root of a tree G, h is the root of a tree H. We check, 
if H is a rooted subtree of G. **/
char isSubtree(struct Vertex* g, struct Graph* H, struct GraphPool* gp) {
	// TODO
	struct Vertex* h = H->vertices[0];
	return recursiveSubtreeCheck(g,g,h,h,gp);
}