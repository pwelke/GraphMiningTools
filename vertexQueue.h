/*
 * vertexQueue.h
 *
 *  Created on: Feb 10, 2017
 *      Author: pascal
 */

#ifndef VERTEXQUEUE_H_
#define VERTEXQUEUE_H_

#include "graph.h"

void addToVertexQueue(struct Vertex* v, struct ShallowGraph* border, struct ShallowGraphPool* sgp);
struct Vertex* popFromVertexQueue(struct ShallowGraph* border, struct ShallowGraphPool* sgp);
struct Vertex* peekFromVertexQueue(struct ShallowGraph* queue, struct ShallowGraphPool* sgp);

#endif /* VERTEXQUEUE_H_ */
