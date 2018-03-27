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
 * A SupportSet contains information that is needed for the iterativeSubtreeIsomorphism algorithm.
 *
 * Such a list has a ->size and points to its ->first and ->last element. Each of the ->size elements is expected to have the same ->h,
 * but possibly different ->g's, i.e. we expect a SupportSet to represent the support set of a pattern in a graph database.
 */

void appendSupportSetElement(struct SupportSet* s, struct SupportSetElement* e) {
	if (s->last != NULL) {
		s->last->next = e;
		s->last = e;
	} else {
		s->first = s->last = e;
	}
	s->size += 1;
}

void appendSupportSetData(struct SupportSet* l, struct SubtreeIsoDataStore data) {
	struct SupportSetElement* e = calloc(1, sizeof(struct SupportSetElement));
	e->data = data;
	appendSupportSetElement(l, e);
}

void printSupportSet(struct SupportSet* l, FILE* out) {
	fprintf(out, "%zu elements: [", l->size);
	for (struct SupportSetElement* i=l->first; i!=NULL; i=i->next) {
		fprintf(out, "%i, ", i->data.g->number);
	}
	fprintf(out, "]\n");

}

void printSupportSetSparse(struct SupportSet* l, FILE* out) {
	fprintf(out, "%i:", l->first->data.h->number);
	for (struct SupportSetElement* e=l->first; e!=NULL; e=e->next) {
		fprintf(out, " %i", e->data.g->number);
	}
	fprintf(out, "\n");
}

void printSupportSetsSparse(struct SupportSet* lists, FILE* out) {
	for (struct SupportSet* l=lists; l!=NULL; l=l->next) {
		printSupportSetSparse(l, out);
	}
}

struct SupportSet* getSupportSet() {
	return calloc(1, sizeof(struct SupportSet));
}

void shallowdumpSupportSetElements(struct SupportSetElement* e) {
	if (e->next != NULL) {
		shallowdumpSupportSetElements(e->next);
	}
	free(e);
}

void dumpSupportSetCopy(struct SupportSet* s) {
	if (s->size > 0) {
		shallowdumpSupportSetElements(s->first);
	}
	free(s);
}

void dumpSupportSetElements(struct SupportSetElement* e) {
	if (e->next != NULL) {
		dumpSupportSetElements(e->next);
	}

	dumpNewCube(e->data.S, e->data.g->n);
	free(e);
}

void dumpSupportSet(struct SupportSet* s) {
	if (s->size > 0) {
		dumpSupportSetElements(s->first);
	}
	free(s);
}


void dumpSupportSetElementsWithPostorder(struct SupportSetElement* e, struct GraphPool* gp) {
	if (e->next != NULL) {
		dumpSupportSetElementsWithPostorder(e->next, gp);
	}

	dumpNewCube(e->data.S, e->data.g->n);
	dumpGraph(gp, e->data.h);
	free(e->data.postorder);
	free(e);
}

void dumpSupportSetWithPostorder(struct SupportSet* s, struct GraphPool* gp) {
	if (s->size > 0) {
		dumpSupportSetElementsWithPostorder(s->first, gp);
	}
	free(s);
}


/**
 * Assumes that each support set in the list supportSets corresponds to the same pattern but to different base graphs.
 * To correctly free the cubes (iterative subtree iso data structures), we need h->n. Hence, we need to dump h after
 * we dumped the support sets.
 */
void dumpSupportSetWithPattern(struct SupportSet* supportSets, struct GraphPool* gp) {
	struct Graph* h = supportSets->first->data.h;
	dumpSupportSet(supportSets);
	dumpGraph(gp, h);
}

struct SupportSet* intersectTwoSupportSets(struct SupportSet* l1, struct SupportSet* l2) {
	struct SupportSet* supportList = getSupportSet();

	struct SupportSetElement* a = l1->first;
	struct SupportSetElement* b = l2->first;

	/* Once one or the other list runs out -- we're done */
	while (a != NULL && b != NULL)
	{
		if (a->data.g->number == b->data.g->number)
		{
			appendSupportSetData(supportList, a->data);
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
struct SupportSet* intersectSupportSets(struct SupportSet* aprioriList) {
	if (aprioriList == NULL) {
		return NULL;
	}
	if (aprioriList->next == NULL) {
		return intersectTwoSupportSets(aprioriList, aprioriList); // copy
	}

	struct SupportSet* intersection = intersectTwoSupportSets(aprioriList, aprioriList->next);
	for (struct SupportSet* support=aprioriList->next->next; support!=NULL; support=support->next) {
		struct SupportSet* tmp = intersectTwoSupportSets(intersection, support);
		dumpSupportSetCopy(intersection);
		intersection = tmp;
	}

	return intersection;
}

char isSortedSupportSetOnPatterns(struct SupportSet* l) {
	if (l->size < 2) {
		return 1;
	}
	for (struct SupportSet* e=l; e->next!=NULL; e=e->next) {
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
struct SupportSet* getSupportSetsOfPatterns(struct SupportSet* allSupportSets, struct IntSet* patternIds) {
	assert(allSupportSets != NULL);
	assert(patternIds != NULL);
	assert(isSortedUniqueIntSet(patternIds));
	assert(isSortedSupportSetOnPatterns(allSupportSets));

	struct SupportSet* selectedSupportSets = NULL;

	struct SupportSet* a = allSupportSets;
	struct IntElement* b = patternIds->first;

	size_t assert_Found = 0;

	/* Once one or the other list runs out -- we're done */
	while (a != NULL && b != NULL)
	{
		if (a->first->data.h->number == b->value) {
			struct SupportSet* supportSet = shallowCopySupportSet(a);
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

struct SupportSet* shallowCopySupportSet(struct SupportSet* a) {
	struct SupportSet* copy = getSupportSet();
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
struct SupportSet* supportSetChangeHead(struct SupportSet* parentSupportSets, int parentIdToKeep) {
	assert(parentSupportSets != NULL);

	// don't change anything and return fast if list head is already correct
	if (parentSupportSets->first->data.h->number == parentIdToKeep) {
		return parentSupportSets;
	}
	for (struct SupportSet* e=parentSupportSets; e->next!=NULL; e=e->next) {
		if (e->next->first->data.h->number == parentIdToKeep) {
			// remove matching element from its position in the list
			struct SupportSet* match = e->next;
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
