#ifndef _INTSET_H_
#define _INTSET_H_

struct IntElement{
	int value;
	struct IntElement* next;
};

struct IntSet{
	struct IntElement* first;
	struct IntElement* last;
	struct IntSet* next;
	size_t size;
};

struct IntElement* getIntElement();
struct IntSet* getIntSet();
void dumpIntElements(struct IntElement* e);
void printIntSet(struct IntSet* s, FILE*out);
void printIntSetSparse(struct IntSet* s, int id, FILE* out);
void printIntSetSparseNoId(struct IntSet* s, FILE* out);
void printIntSetAsLibSvm(struct IntSet* s, int label, FILE* out);
void dumpIntSet(struct IntSet* s);
void appendInt(struct IntSet* s, int i);
void addIntSortedNoDuplicates(struct IntSet* s, int i);
void addIntSortedWithDuplicates(struct IntSet* s, int i);
void appendIntElement(struct IntSet* s, struct IntElement* e);
char containsInt(struct IntSet* s, int i);
struct IntElement* popIntElement(struct IntSet* s);
struct IntSet* intersectIntSet(const struct IntSet* setA, const struct IntSet* setB);
char isSortedUniqueIntSet(struct IntSet* s);
char isSortedIntSet(struct IntSet* s);

#endif
