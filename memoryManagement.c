#include <malloc.h>
#include "graph.h"

int MEM_DEBUG = 0;

/**
Object pool creation method. One can specify a number of elements that will be allocated as array.
This method can be optimized further by excluding the part for allocation in a way that it is possible 
to allocate memory blocks later on
*/
struct ListPool* createListPool(unsigned int initNumberOfElements){
	struct ListPool* newPool;
	if ((newPool = malloc(sizeof(struct ListPool)))) {
		if ((newPool->unused = malloc(initNumberOfElements * sizeof(struct VertexList)))) {
			unsigned int i;
			/* we have allocated an array, but need a list, thus set ponters accordingly */
			for (i=0; i<initNumberOfElements-1; ++i) {
				(newPool->unused[i]).next = &(newPool->unused[i+1]);
			}
			(newPool->unused[initNumberOfElements-1]).next = NULL;
			newPool->poolPointer = newPool->unused;
			newPool->initNumberOfElements = initNumberOfElements;
		} else {
			printf("Error while initializing object pool for list elements\n");
			return NULL;
		}
	} else {
		printf("Error while initializing object pool for list elements\n");
		return NULL;
	}
	return newPool;
}


/**
Frees the object pool and all elements in its unused list
*/
void freeListPool(struct ListPool *p) {
	while (p->unused) {
		p->tmp = p->unused->next;
		if ((p->unused < p->poolPointer) || (p->unused > &(p->poolPointer[p->initNumberOfElements-1]))) {
			free(p->unused);
		}
		p->unused = p->tmp;
	}
	free(p->poolPointer);
	free(p);
}
	

/**
Get a List Element out of ObjectPool p. If p does not contain unused elements, new memory is
allocated.

The output of this function is initialized with zero / null values.
*/
struct VertexList* getVertexList(struct ListPool* p){
	if (p->unused) {
		/* get first of the unused elements */
		p->tmp = p->unused;
		p->unused = p->tmp->next;
	} else {
		/* allocate memory for new struct */
		if (!(p->tmp = malloc(sizeof(struct VertexList)))) {
			/* malloc did not work */
			printf("Error allocating memory\n");
			return NULL;
		} 
	}
	
	/* The malloc worked or we had an element left. So we initialize the struct */
	wipeVertexList(p->tmp);

	return p->tmp;
		
}


/** 
Set all variables in the struct to 0 or NULL
*/
void wipeVertexList(struct VertexList* e) {
	e->startPoint = NULL;
	e->endPoint = NULL;
	e->label = NULL;

	e->next = NULL;

	e->used = 0;
	e->isStringMaster = 0;
	e->flag = 0;
}


/** 
Return unused element to the pool. This method should always replace free during the algorithm.
If the dumped element is the holder of label information, then that string is freed, so be careful to dump the 
"real" edge not before using one of its copies.
*/
void dumpVertexList(struct ListPool* p, struct VertexList* l) {
	/* free the label string, if l manages it */
	if (l->isStringMaster) {
		free(l->label);
	}

	if (MEM_DEBUG) {
		free(l);
	} else {
		/* add l to the unused list */
		l->next = p->unused;
		p->unused = l;
	}
}


/**
 * Given a pointer to a VertexList struct e, this method dumps all VertexList structs
 * that are reachable from e by following the ->next pointers.
 */
void dumpVertexListRecursively(struct ListPool* p, struct VertexList* e) {
	if (e) {
		dumpVertexListRecursively(p, e->next);
		dumpVertexList(p, e);
	}
}


/******************************/


/**
Object pool creation method. One can specify a number of elements that will be allocated as array.
This method can be optimized further by excluding the part for allocation in a way that it is possible 
to allocate memory blocks later on
*/
struct VertexPool* createVertexPool(unsigned int initNumberOfElements){
	struct VertexPool* newPool;
	if ((newPool = malloc(sizeof(struct VertexPool)))) {
		if ((newPool->unused = malloc(initNumberOfElements * sizeof(struct Vertex)))) {
			unsigned int i;
			/* we have allocated an array, but need a list, thus set ponters accordingly */
			for (i=0; i<initNumberOfElements-1; ++i) {
				(newPool->unused[i]).next = &(newPool->unused[i+1]);
			}
			(newPool->unused[initNumberOfElements-1]).next = NULL;
			newPool->poolPointer = newPool->unused;
			newPool->initNumberOfElements = initNumberOfElements;
		} else {
			printf("Error while initializing object pool for list elements\n");
			return NULL;
		}
	} else {
		printf("Error while initializing object pool for list elements\n");
		return NULL;
	}
	
	return newPool;
}


/**
Frees the object pool and all elements in its unused list
*/
void freeVertexPool(struct VertexPool *p) {
	while (p->unused) {
		p->tmp = p->unused->next;
		if ((p->unused < p->poolPointer) || (p->unused > &(p->poolPointer[p->initNumberOfElements-1]))) {
			free(p->unused);
		}
		p->unused = p->tmp;
	}
	free(p->poolPointer);
	free(p);
}


/**
Obtain a vertex struct from a vertex pool. If the pool does not have any free vertices, a new struct will be malloc'd.
The returned struct is initialized with zero/null values
*/
struct Vertex* getVertex(struct VertexPool* p){
		if (p->unused) {
		/* get first of the unused elements */
		p->tmp = p->unused;
		p->unused = p->tmp->next;
	} else {
		/* allocate memory for new struct */
		if (!(p->tmp = malloc(sizeof(struct Vertex)))) {
			/* malloc did not work */
			printf("Error allocating memory\n");
			return NULL;
		} 
	}
	
	/* The malloc worked or we had an element left. So we initialize the struct */
	wipeVertex(p->tmp);
	
	return p->tmp;
}


/** 
Set all variables in the struct to 0 or NULL
*/
void wipeVertex(struct Vertex* v) {
	v->label = NULL;
	v->next = NULL;
	v->neighborhood = NULL;
	v->number = 0;
	v->visited = 0;
	v->isStringMaster = 0;
	v->lowPoint = 0;
	v->d = 0;
}


/** 
Return unused element to the pool. This method should always replace free during the algorithm.
If the dumped element is the holder of label information, then that string is freed, so be careful to dump the 
"real" vertex not before using one of its copies.
*/
void dumpVertex(struct VertexPool* p, struct Vertex* v){
	
	if (v->isStringMaster) {
		free(v->label);
	}

	if (MEM_DEBUG) {
		free(v);
	} else {
		/* add v to the unused list */
		v->next = p->unused;
		p->unused = v;
	}
}


/************************************/


/**
Object pool creation method. One can specify a number of elements that will be allocated as array.
This method can be optimized further by excluding the part for allocation in a way that it is possible
to allocate memory blocks later on
*/
struct GraphPool* createGraphPool(unsigned int initNumberOfElements, struct VertexPool* vp, struct ListPool* lp) {
	struct GraphPool* newPool;

	if ((newPool = malloc(sizeof(struct GraphPool)))) {
		/* set the listpool and vertexpool of the graphpool according to the input. good for dumping actions */
		newPool->vertexPool = vp;
		newPool->listPool = lp;

		
		if ((newPool->unused = malloc(initNumberOfElements * sizeof(struct Graph)))) {
			unsigned int i;
			/* we have allocated an array, but need a list, thus set ponters accordingly */
			for (i=0; i<initNumberOfElements-1; ++i) {
				(newPool->unused[i]).next = &(newPool->unused[i+1]);
			}
			(newPool->unused[initNumberOfElements-1]).next = NULL;
			newPool->poolPointer = newPool->unused;
			newPool->initNumberOfElements = initNumberOfElements;
		} else {
			printf("Error while initializing object pool for list elements\n");
			return NULL;
		}
	} else {
		printf("Error while initializing object pool for list elements\n");
		return NULL;
	}
	
	return newPool;
}


/* free a graph pool and every graph struct contained in it */
void freeGraphPool(struct GraphPool *p) {
	while (p->unused) {
		p->tmp = p->unused->next;
		if ((p->unused < p->poolPointer) || (p->unused > &(p->poolPointer[p->initNumberOfElements-1]))) {
			free(p->unused);
		}
		p->unused = p->tmp;
	}
	free(p->poolPointer);
	free(p);
}


/**
 * Get a graph from the specified GraphPool
 */
struct Graph* getGraph(struct GraphPool* p) {
	if (p->unused) {
		/* get first of the unused elements */
		p->tmp = p->unused;
		p->unused = p->tmp->next;
	} else {
		/* allocate memory for new struct */
		if (!(p->tmp = malloc(sizeof(struct Graph)))) {
			/* malloc did not work */
			printf("Error allocating memory\n");
			return NULL;
		} 
	}
	
	/* malloc worked or there was an unused element left, initialize stuff */
	wipeGraph(p->tmp);
	
	return p->tmp;
}


/** 
Set all variables in the struct to 0 or NULL
*/
void wipeGraph(struct Graph* g) {
	g->n = 0;
	g->m = 0;
	g->number = 0;
	g->activity = 0;
	g->vertices = NULL;
	g->next = NULL;
}


/**
dumpGraph dumps all vertices and edges contained in g into the respective pools and frees the 
vertices array of g and g itself
*/
void dumpGraph(struct GraphPool* p, struct Graph *g) {
	struct VertexList *tmp;
	int i;
	/* dump content */
	if (g->vertices) {
		for (i=0; i<g->n; ++i) {
			if (g->vertices[i]) {
				while (g->vertices[i]->neighborhood) {
					tmp = g->vertices[i]->neighborhood->next;
					dumpVertexList(p->listPool, g->vertices[i]->neighborhood);
					g->vertices[i]->neighborhood = tmp;
				}
				dumpVertex(p->vertexPool, g->vertices[i]);
			}
		}
	
		free(g->vertices);
	}
	
	if (MEM_DEBUG) {
		free(g);
	} else {
		/* append g to the unused list */
		g->next = p->unused;
		p->unused = g;
	}
}

/** Dump a list of struct Graphs. (e.g. the result of partitionIntoForestAndCycles()) **/
void dumpGraphList(struct GraphPool* gp, struct Graph* g) {
	if (g->next != NULL) {
		dumpGraphList(gp, g->next);
	}
	dumpGraph(gp, g);
}


/**********************************************/
			

/**
Object pool creation method. One can specify a number of elements that will be allocated as array.
This method can be optimized further by excluding the part for allocation in a way that it is possible 
to allocate memory blocks later on
*/
struct ShallowGraphPool* createShallowGraphPool(unsigned int initNumberOfElements, struct ListPool* lp){
	struct ShallowGraphPool* newPool;
	
	if ((newPool = malloc(sizeof(struct ShallowGraphPool)))) {
		/* set the listpool of the graphpool according to the input. good for dumping actions */
		newPool->listPool = lp;
		if ((newPool->unused = malloc(initNumberOfElements * sizeof(struct ShallowGraph)))) {
			unsigned int i;
			/* we have allocated an array, but need a list, thus set ponters accordingly */
			for (i=0; i<initNumberOfElements-1; ++i) {
				(newPool->unused[i]).next = &(newPool->unused[i+1]);
			}
			(newPool->unused[initNumberOfElements-1]).next = NULL;
			newPool->poolPointer = newPool->unused;
			newPool->initNumberOfElements = initNumberOfElements;
		} else {
			printf("Error while initializing object pool for list elements\n");
			return NULL;
		}
	} else {
		printf("Error while initializing object pool for list elements\n");
		return NULL;
	}
	
	return newPool;
}


/**
 * free ShallowGraphPool struct and any ShallowGraph in it.
 */
void freeShallowGraphPool(struct ShallowGraphPool *p) {
	while (p->unused) {
		p->tmp = p->unused->next;
		if ((p->unused < p->poolPointer) || (p->unused > &(p->poolPointer[p->initNumberOfElements-1]))) {
			free(p->unused);
		}
		p->unused = p->tmp;
	}
	free(p->poolPointer);
	free(p);
}


/**
Standard Object Pool getter
*/
struct ShallowGraph* getShallowGraph(struct ShallowGraphPool *p) {
	
	if (p->unused) {
		/* get first of the unused elements */
		p->tmp = p->unused;
		p->unused = p->tmp->next;
	} else {
		/* allocate memory for new struct */
		if (!(p->tmp = malloc(sizeof(struct ShallowGraph)))) {
			/* malloc did not work */
			printf("Error allocating memory\n");
			return NULL;
		} 
	}
	
	/* The malloc worked or we had an element left. So we initialize the struct */
	wipeShallowGraph(p->tmp);
	
	return p->tmp;
}


/** 
Set all variables in the struct to 0 or NULL
*/
void wipeShallowGraph(struct ShallowGraph* g) {
	g->next = NULL;
	g->prev = NULL;
	g->m = 0;
	g->edges = NULL;
	g->lastEdge = NULL;
	g->data = 0;
}


/**
Standard Object Pool dumper
 */
void dumpShallowGraph(struct ShallowGraphPool *p, struct ShallowGraph* g) {

	/* dump edges */
	while (g->edges) {
		p->listPool->tmp = g->edges;
		g->edges = g->edges->next;
		dumpVertexList(p->listPool, p->listPool->tmp);
	}

	if (MEM_DEBUG) {
		free(g);
	} else {
		/* add g to the unused list */
		g->next = p->unused;
		p->unused = g;
	}
}


/**
 * Dump a cycle or list of ShallowGraphs
 */
void dumpShallowGraphCycle(struct ShallowGraphPool *p, struct ShallowGraph* g) {
	struct ShallowGraph *idx = g, *tmp;
	/* to cope with the possibility that g points to a list and not to a cycle, lazy check for NULL */
	while (idx && (idx->next != g)){
		tmp = idx;
		idx = idx->next;
		dumpShallowGraph(p, tmp);
	}
	/* if g did point to a cycle of length 1 */
	if (idx) {
		dumpShallowGraph(p, idx);
	}

}


/* does not fit anywhere */

/**
 * Clone an array of chars to a newly allocated array of the same size.
 */
char* copyString(char* string) {
	int i;
	char* copy;

	for (i=0; string[i] != 0; ++i);

	copy = malloc((i + 1) * sizeof(char));

	for (i=0; string[i] != 0; ++i) {
		copy[i] = string[i];
	}
	copy[i] = 0;

	return copy;
}
