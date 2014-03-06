/*
 * outerplanar.h
 *
 *  Created on: Feb 12, 2012
 *      Author: pascal
 */

#ifndef OUTERPLANAR_H_
#define OUTERPLANAR_H_


/**
 * Data structure for Block and Bridge Trees. Given an outerplanar Graph G,
 * tree contains all vertices in V(G) that are contained in more than one block
 * or are incident to a bridge. (some of the positions in tree->vertices are NULL)
 * blocks contains a vertex for each block in blockComponents. We have
 * blocks->vertices[i]->number = -(i+1). This is to distinguish between block
 * vertices and original vertices.
 * blockComponents contains a ShallowGraph for each block with corresponding indices
 * Be aware that the vertices, that are referenced by edges in blockComponents, are
 * different objects than those in tree or blocks.
 */
struct BBTree{
	struct Graph* tree;
	struct Graph* blocks;
	struct ShallowGraph** blockComponents;
};

char isMaximalOuterplanar(struct Graph* g, struct ShallowGraphPool* sgp);
char isOuterplanar(struct ShallowGraph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp);

struct BBTree* createBlockAndBridgeTree(struct ShallowGraph* list, struct Graph *original, struct GraphPool* gp, struct ShallowGraphPool *sgp);
void dumpBBTree(struct GraphPool* gp, struct ShallowGraphPool* sgp, struct BBTree* tree);

#endif /* OUTERPLANAR_H_ */
