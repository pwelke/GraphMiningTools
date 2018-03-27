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

struct SupportSetElement {
	struct SubtreeIsoDataStore data;
	struct SupportSetElement* next;
};

struct SupportSet {
	struct SupportSetElement* first;
	struct SupportSetElement* last;
	struct SupportSet* next;
	size_t size;
	int patternId;
};

void appendSupportSetElement(struct SupportSet* s, struct SupportSetElement* e);
void appendSupportSetData(struct SupportSet* l, struct SubtreeIsoDataStore data);

void printSupportSet(struct SupportSet* l, FILE* out);
void printSupportSetSparse(struct SupportSet* l, FILE* out);
void printSupportSetsSparse(struct SupportSet* lists, FILE* out);

struct SupportSet* getSupportSet();

void shallowdumpSupportSetElements(struct SupportSetElement* e);
void dumpSupportSetCopy(struct SupportSet* s);
void dumpSupportSetElements(struct SupportSetElement* e);
void dumpSupportSet(struct SupportSet* s);
void dumpSupportSetElementsWithPostorder(struct SupportSetElement* e, struct GraphPool* gp);
void dumpSupportSetWithPostorder(struct SupportSet* s, struct GraphPool* gp);
void dumpSupportSetWithPattern(struct SupportSet* supportSets, struct GraphPool* gp);

struct SupportSet* intersectTwoSupportSets(struct SupportSet* l1, struct SupportSet* l2);
struct SupportSet* intersectSupportSets(struct SupportSet* aprioriList);

char isSortedSupportSetOnPatterns(struct SupportSet* l);

struct SupportSet* getSupportSetsOfPatterns(struct SupportSet* allSupportSets, struct IntSet* patternIds);
struct SupportSet* shallowCopySupportSet(struct SupportSet* a);
struct SupportSet* supportSetChangeHead(struct SupportSet* parentSupportSets, int parentIdToKeep);

#endif /* SUBTREEISODATASTORELIST_H_ */
