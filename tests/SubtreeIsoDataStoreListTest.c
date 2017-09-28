/*
 * SubtreeIsoDataStoreListTest.c
 *
 *  Created on: Mar 16, 2016
 *      Author: pascal
 */

#include <assert.h>
#include "../subtreeIsoDataStoreList.h"


struct SubtreeIsoDataStoreList* createMockLists(struct GraphPool* gp) {

	int numbers[3] = {128, 163, 164};

	struct SubtreeIsoDataStoreList* l1 = getSubtreeIsoDataStoreList();
	struct Graph* h = getGraph(gp);
	h->number = 1;

	for (int i=0; i<3; ++i) {
		struct Graph* g = getGraph(gp);
		g->number = numbers[i];
		struct SubtreeIsoDataStore s = {0};
		s.g = g;
		s.h = h;
		appendSubtreeIsoDataStore(l1, s);
	}

	struct SubtreeIsoDataStoreList* l2 = getSubtreeIsoDataStoreList();
	h = getGraph(gp);
	h->number = 2;

	for (int i=1; i<3; ++i) {
		struct Graph* g = getGraph(gp);
		g->number = numbers[i];
		struct SubtreeIsoDataStore s = {0};
		s.g = g;
		s.h = h;
		appendSubtreeIsoDataStore(l2, s);
	}

	l1->next = l2;
	return l1;

}

int main(int argc, char **argv) {
	struct GraphPool* gp = createGraphPool(5, createVertexPool(1), createListPool(1));

	struct SubtreeIsoDataStoreList* l = createMockLists(gp);

	printSubtreeIsoDataStoreList(l, stdout);
	printSubtreeIsoDataStoreList(l->next, stdout);

	struct SubtreeIsoDataStoreList* intersection = intersectSupportSets(l);
	printSubtreeIsoDataStoreList(intersection, stdout);
}

