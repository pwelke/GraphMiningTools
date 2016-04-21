/*
 * importantSubtrees.c
 *
 *  Created on: Apr 21, 2016
 *      Author: pascal
 */

#include <stddef.h>

#include "graph.h"
#include "connectedComponents.h"

char isImportantSubtreeAbsolute(struct Graph* g, struct Graph* h, int absFreq, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* connectedComponents = getConnectedComponents(g, sgp);
	for (struct ShallowGraph* component=connectedComponents; component!=NULL; component=component->next) {
		struct Graph* subgraph = shallowGraphToGraph() // todo shallow struct Graph that just points to the vertices, no copying.
	}
}
