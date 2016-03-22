/*
 * subtreeIsoDataStoreList.h
 *
 *  Created on: Mar 16, 2016
 *      Author: pascal
 */

#ifndef SUBTREEISODATASTORELIST_H_
#define SUBTREEISODATASTORELIST_H_

#include "graph.h"
#include "iterativeSubtreeIsomorphism.h"
#include "intSet.h"

struct SubtreeIsoDataStoreElement {
	struct SubtreeIsoDataStore data;
	struct SubtreeIsoDataStoreElement* next;
};

struct SubtreeIsoDataStoreList {
	struct SubtreeIsoDataStoreElement* first;
	struct SubtreeIsoDataStoreElement* last;
	struct SubtreeIsoDataStoreList* next;
	size_t size;
	int patternId;
};

void appendSubtreeIsoDataStoreElement(struct SubtreeIsoDataStoreList* s, struct SubtreeIsoDataStoreElement* e);
void appendSubtreeIsoDataStore(struct SubtreeIsoDataStoreList* l, struct SubtreeIsoDataStore data);

void printSubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* l, FILE* out);
void printSubtreeIsoDataStoreListSparse(struct SubtreeIsoDataStoreList* l, FILE* out);
void printSubtreeIsoDataStoreListsSparse(struct SubtreeIsoDataStoreList* lists, FILE* out);

struct SubtreeIsoDataStoreList* getSubtreeIsoDataStoreList();

void shallowdumpSubtreeIsoDataStoreElements(struct SubtreeIsoDataStoreElement* e);
void dumpSubtreeIsoDataStoreListCopy(struct SubtreeIsoDataStoreList* s);
void dumpSubtreeIsoDataStoreElements(struct SubtreeIsoDataStoreElement* e);
void dumpSubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* s);
void dumpSubtreeIsoDataStoreElementsWithPostorder(struct SubtreeIsoDataStoreElement* e, struct GraphPool* gp);
void dumpSubtreeIsoDataStoreListWithPostorder(struct SubtreeIsoDataStoreList* s, struct GraphPool* gp);
void dumpSubtreeIsoDataStoreListWithH(struct SubtreeIsoDataStoreList* supportSets, struct GraphPool* gp);

struct SubtreeIsoDataStoreList* intersectTwoSupportSets(struct SubtreeIsoDataStoreList* l1, struct SubtreeIsoDataStoreList* l2);
struct SubtreeIsoDataStoreList* intersectSupportSets(struct SubtreeIsoDataStoreList* aprioriList);

char isSortedSubtreeIsoDataStoreListOnPatterns(struct SubtreeIsoDataStoreList* l);

struct SubtreeIsoDataStoreList* getSupportSetsOfPatterns(struct SubtreeIsoDataStoreList* allSupportSets, struct IntSet* patternIds);
struct SubtreeIsoDataStoreList* shallowCopySubtreeIsoDataStoreList(struct SubtreeIsoDataStoreList* a);
struct SubtreeIsoDataStoreList* subtreeIsoDataStoreChangeHead(struct SubtreeIsoDataStoreList* parentSupportSets, int parentIdToKeep);

#endif /* SUBTREEISODATASTORELIST_H_ */
