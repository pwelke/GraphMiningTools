/*
 * subtreeIsoDataStoreList.c
 *
 *  Created on: Mar 16, 2016
 *      Author: pascal
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "intSet.h"
#include "subtreeIsoDataStoreList.h"

/**
 * A SubtreeIsoDataStoreList contains information that is needed for the iterativeSubtreeIsomorphism algorithm.
 *
 * Such a list has a ->size and points to its ->first and ->last element. Each of the ->size elements is expected to have the same ->h,
 * but possibly different ->g's, i.e. we expect a SubtreeIsoDataStoreList to represent the support set of a pattern in a graph database.
 */

void appendSubtreeIsoDataStoreElement(struct SubtreeIsoDataStoreList* s, struct SubtreeIsoDataStoreElement* e) {
	if (s->last != NULL) {
		s->last->next = e;
		s->last = e;
	} else {
		s->first = s->last = e;
	}
	s->size += 1;
}

void appendSubtreeIsoDataStore(struct SubtreeIsoDataStoreList* l, struct SubtreeIsoDataStore data) {
	struct SubtreeIsoDataStoreElement* e = calloc(1, sizeof(struct SubtreeIsoDataStoreElement));
	e->data = data;
	appendSubtreeIsoDataStoreElement(l, e);
}

void printSubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* l, FILE* out) {
	fprintf(out, "%zu elements: [", l->size);
	for (struct SubtreeIsoDataStoreElement* i=l->first; i!=NULL; i=i->next) {
		fprintf(out, "%i, ", i->data.g->number);
	}
	fprintf(out, "]\n");

}

void printSubtreeIsoDataStoreListSparse(struct SubtreeIsoDataStoreList* l, FILE* out) {
	fprintf(out, "%i:", l->first->data.h->number);
	for (struct SubtreeIsoDataStoreElement* e=l->first; e!=NULL; e=e->next) {
		fprintf(out, " %i", e->data.g->number);
	}
	fprintf(out, "\n");
}

void printSubtreeIsoDataStoreListsSparse(struct SubtreeIsoDataStoreList* lists, FILE* out) {
	for (struct SubtreeIsoDataStoreList* l=lists; l!=NULL; l=l->next) {
		printSubtreeIsoDataStoreListSparse(l, out);
	}
}

struct SubtreeIsoDataStoreList* getSubtreeIsoDataStoreList() {
	return calloc(1, sizeof(struct SubtreeIsoDataStoreList));
}

void shallowdumpSubtreeIsoDataStoreElements(struct SubtreeIsoDataStoreElement* e) {
	if (e->next != NULL) {
		shallowdumpSubtreeIsoDataStoreElements(e->next);
	}
	free(e);
}

void dumpSubtreeIsoDataStoreListCopy(struct SubtreeIsoDataStoreList* s) {
	if (s->size > 0) {
		shallowdumpSubtreeIsoDataStoreElements(s->first);
	}
	free(s);
}

void dumpSubtreeIsoDataStoreElements(struct SubtreeIsoDataStoreElement* e) {
	if (e->next != NULL) {
		dumpSubtreeIsoDataStoreElements(e->next);
	}

	dumpNewCube(e->data.S, e->data.g->n);
	free(e);
}

void dumpSubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* s) {
	if (s->size > 0) {
		dumpSubtreeIsoDataStoreElements(s->first);
	}
	free(s);
}


void dumpSubtreeIsoDataStoreElementsWithPostorder(struct SubtreeIsoDataStoreElement* e, struct GraphPool* gp) {
	if (e->next != NULL) {
		dumpSubtreeIsoDataStoreElementsWithPostorder(e->next, gp);
	}

	dumpNewCube(e->data.S, e->data.g->n);
	dumpGraph(gp, e->data.h);
	free(e->data.postorder);
	free(e);
}

void dumpSubtreeIsoDataStoreListWithPostorder(struct SubtreeIsoDataStoreList* s, struct GraphPool* gp) {
	if (s->size > 0) {
		dumpSubtreeIsoDataStoreElementsWithPostorder(s->first, gp);
	}
	free(s);
}


/**
 * Assumes that each support set in the list supportSets corresponds to the same pattern but to different base graphs.
 * To correctly free the cubes (iterative subtree iso data structures), we need h->n. Hence, we need to dump h after
 * we dumped the support sets.
 */
void dumpSubtreeIsoDataStoreListWithH(struct SubtreeIsoDataStoreList* supportSets, struct GraphPool* gp) {
	struct Graph* h = supportSets->first->data.h;
	dumpSubtreeIsoDataStoreList(supportSets);
	dumpGraph(gp, h);
}

struct SubtreeIsoDataStoreList* intersectTwoSupportSets(struct SubtreeIsoDataStoreList* l1, struct SubtreeIsoDataStoreList* l2) {
	struct SubtreeIsoDataStoreList* supportList = getSubtreeIsoDataStoreList();

	struct SubtreeIsoDataStoreElement* a = l1->first;
	struct SubtreeIsoDataStoreElement* b = l2->first;

	/* Once one or the other list runs out -- we're done */
	while (a != NULL && b != NULL)
	{
		if (a->data.g->number == b->data.g->number)
		{
			appendSubtreeIsoDataStore(supportList, a->data);
			a = a->next;
			b = b->next;
		}
		else if (a->data.g->number < b->data.g->number) /* advance the smaller list */
			a = a->next;
		else
			b = b->next;
	}
	return supportList;

}


/**
 * Return the intersection of the sets in aprioriList.
 * The function returns a new list, iff aprioriList != NULL
 */
struct SubtreeIsoDataStoreList* intersectSupportSets(struct SubtreeIsoDataStoreList* aprioriList) {
	if (aprioriList == NULL) {
		return NULL;
	}
	if (aprioriList->next == NULL) {
		return intersectTwoSupportSets(aprioriList, aprioriList); // copy
	}

	struct SubtreeIsoDataStoreList* intersection = intersectTwoSupportSets(aprioriList, aprioriList->next);
	for (struct SubtreeIsoDataStoreList* support=aprioriList->next->next; support!=NULL; support=support->next) {
		struct SubtreeIsoDataStoreList* tmp = intersectTwoSupportSets(intersection, support);
		dumpSubtreeIsoDataStoreListCopy(intersection);
		intersection = tmp;
	}

	return intersection;
}

char isSortedSubtreeIsoDataStoreListOnPatterns(struct SubtreeIsoDataStoreList* l) {
	if (l->size < 2) {
		return 1;
	}
	for (struct SubtreeIsoDataStoreList* e=l; e->next!=NULL; e=e->next) {
		if (e->first->data.h->number >= e->next->first->data.h->number) {
			return 0;
		}
	}
	return 1;
}


/**
 *
 * input: a list of support sets allSupportSets, a list of pattern ids patternIds
 * output: a list of support sets which is a sublist of allSupportSets, such that each support set corresponds to an id in patternIds
 * (comparison for intersection takes place on allSupportSet->first->data.h->number and patternId->value
 *
 * Basically a sorted list intersection with different type lists, hence assumes sorted lists.
 *
 * Expects both inputs to be not NULL.
 * Guarantees that the output list has as many elements as parentIds, i.e. that all requested support sets were found
 */
struct SubtreeIsoDataStoreList* getSupportSetsOfPatterns(struct SubtreeIsoDataStoreList* allSupportSets, struct IntSet* patternIds) {
	assert(allSupportSets != NULL);
	assert(patternIds != NULL);
	assert(isSortedUniqueIntSet(patternIds));
	assert(isSortedSubtreeIsoDataStoreListOnPatterns(allSupportSets));

	struct SubtreeIsoDataStoreList* selectedSupportSets = NULL;

	struct SubtreeIsoDataStoreList* a = allSupportSets;
	struct IntElement* b = patternIds->first;

	size_t assert_Found = 0;

	/* Once one or the other list runs out -- we're done */
	while (a != NULL && b != NULL)
	{
		if (a->first->data.h->number == b->value) {
			struct SubtreeIsoDataStoreList* supportSet = shallowCopySubtreeIsoDataStoreList(a);
			supportSet->next = selectedSupportSets;
			selectedSupportSets = supportSet;

			a = a->next;
			b = b->next;

			++assert_Found;
		}
		else if (a->first->data.h->number < b->value) /* advance the smaller list */
			a = a->next;
		else
			b = b->next;
	}

	assert(assert_Found == patternIds->size);
	return selectedSupportSets;

}

struct SubtreeIsoDataStoreList* shallowCopySubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* a) {
	struct SubtreeIsoDataStoreList* copy = getSubtreeIsoDataStoreList();
	copy->first = a->first;
	copy->last = a->last;
	copy->patternId = a->patternId;
	copy->size = a->size;
	return copy;
}


/**
 * Changes the order of the elements in parentSupportSets such that the element that matches parentIdToKeep
 * with element->data->h->number is at the head of the list.
 *
 * expects parentIdToKeep to be in parentSupportSets. (hence also expects  that parentsupportsets->size > 0)
 */
struct SubtreeIsoDataStoreList* subtreeIsoDataStoreChangeHead(struct SubtreeIsoDataStoreList* parentSupportSets, int parentIdToKeep) {
	assert(parentSupportSets != NULL);

	// don't change anything and return fast if list head is already correct
	if (parentSupportSets->first->data.h->number == parentIdToKeep) {
		return parentSupportSets;
	}
	for (struct SubtreeIsoDataStoreList* e=parentSupportSets; e->next!=NULL; e=e->next) {
		if (e->next->first->data.h->number == parentIdToKeep) {
			// remove matching element from its position in the list
			struct SubtreeIsoDataStoreList* match = e->next;
			e->next = e->next->next;
			// push matching element to head of list
			match->next = parentSupportSets;
			parentSupportSets = match;
			break; // stop searching for the list
		}
	}

	// first element is now correct
	assert(parentSupportSets->first->data.h->number == parentIdToKeep);

	return parentSupportSets;
}
