/*
 * SubtreeIsoDataStoreListTest.c
 *
 *  Created on: Mar 16, 2016
 *      Author: pascal
 */

#include <assert.h>

#include "../supportSet.h"


struct SupportSet* createMockLists(struct GraphPool* gp) {

	int numbers[3] = {128, 163, 164};

	struct SupportSet* l1 = getSupportSet();
	struct Graph* h = getGraph(gp);
	h->number = 1;

	for (int i=0; i<3; ++i) {
		struct Graph* g = getGraph(gp);
		g->number = numbers[i];
		struct SubtreeIsoDataStore s = {0};
		s.g = g;
		s.h = h;
		appendSupportSetData(l1, s);
	}

	struct SupportSet* l2 = getSupportSet();
	h = getGraph(gp);
	h->number = 2;

	for (int i=1; i<3; ++i) {
		struct Graph* g = getGraph(gp);
		g->number = numbers[i];
		struct SubtreeIsoDataStore s = {0};
		s.g = g;
		s.h = h;
		appendSupportSetData(l2, s);
	}

	l1->next = l2;
	return l1;

}

int main(int argc, char **argv) {
	struct GraphPool* gp = createGraphPool(5, createVertexPool(1), createListPool(1));

	struct SupportSet* l = createMockLists(gp);

	printSupportSet(l, stdout);
	printSupportSet(l->next, stdout);

	struct SupportSet* intersection = intersectSupportSets(l);
	printSupportSet(intersection, stdout);
}

