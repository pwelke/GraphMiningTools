#include <malloc.h>
#include "intSet.h"

struct IntElement* getIntElement() {
	struct IntElement* e = malloc(sizeof(struct IntElement));
	e->value = 0;
	e->next = NULL;
	return e;
}

struct IntSet* getIntSet() {
	struct IntSet* s = malloc(sizeof(struct IntSet));
	s->first = NULL;
	s->last = NULL;
	s->size = 0;
	return s;
}

void dumpIntElements(struct IntElement* e) {
	if (e->next != NULL) {
		dumpIntElements(e->next);
	}
	free(e);
}

void printIntSet(struct IntSet* s, FILE*out) {
	fprintf(out, "%zu elements: [", s->size);
	for (struct IntElement* i=s->first; i!=NULL; i=i->next) {
		fprintf(out, "%i, ", i->value);
	}
	fprintf(out, "]\n");
}

void printIntSetSparseNoId(struct IntSet* s, FILE* out) {
	for (struct IntElement* i=s->first; i!=NULL; i=i->next) {
		fprintf(out, " %i", i->value);
	}
	fprintf(out, "\n");
}

void printIntSetAsLibSvm(struct IntSet* s, int label, FILE* out) {
	fprintf(out, "%i", label);
	for (struct IntElement* i=s->first; i!=NULL; i=i->next) {
		fprintf(out, " %i:1", i->value);
	}
	fprintf(out, "\n");
}

void printIntSetSparse(struct IntSet* s, int id, FILE* out) {
	fprintf(out, "%i:", id);
	printIntSetSparseNoId(s, out);
}

void dumpIntSet(struct IntSet* s) {
	if (s->size > 0) {
		dumpIntElements(s->first);
	}
	free(s);
}

char containsInt(struct IntSet* s, int i) {
	for (struct IntElement* e=s->first; e!=NULL; e=e->next) {
		if (e->value == i) {
			return 1;
		}
	}
	return 0;
}

void appendInt(struct IntSet* s, int i) {
	struct IntElement* e = getIntElement();
	e->value = i;
	if (s->last != NULL) {
		s->last->next = e;
		s->last = e; 
	} else {
		s->first = s->last = e;
	}
	s->size += 1;
}

/**
 * Add int value to list of ints. creates duplicates, if you add an element that is already contained in the list.
 * if you want no duplicates, use addIntSortedNoDuplicates
 */
void addIntSortedWithDuplicates(struct IntSet* s, int i) {
	struct IntElement* newElement = getIntElement();
	newElement->value = i;
	if (s->last != NULL) {
		if (s->last->value <= i) {
			s->last->next = newElement;
			s->last = newElement;
		} else {
			if (s->first->value > i) {
				newElement->next = s->first;
				s->first = newElement;
			} else {
				for (struct IntElement* e=s->first; e!=NULL; e=e->next) {
					if (e->next->value > i) {
						newElement->next = e->next;
						e->next = newElement;
						break;
					}
				}
			}
		}
	} else {
		s->first = s->last = newElement;
	}
	s->size += 1;
}

/**
 * Add int value to list of ints. creates no duplicates: if you try to add an element that is already contained in the list the list size does not increase.
 * if you want duplicates, use addIntSortedWithDuplicates
 */
void addIntSortedNoDuplicates(struct IntSet* s, int i) {
	if (s->last != NULL) {
		if (s->last->value == i) {
			return;
		}
		if (s->last->value < i) {
			struct IntElement* newElement = getIntElement();
			newElement->value = i;
			s->last->next = newElement;
			s->last = newElement;
		} else {
			if (s->first->value == i) {
				return;
			}
			if (s->first->value > i) {
				struct IntElement* newElement = getIntElement();
				newElement->value = i;
				newElement->next = s->first;
				s->first = newElement;
			} else {
				for (struct IntElement* e=s->first; e!=NULL; e=e->next) {
					if (e->next->value == i) {
						return;
					}
					if (e->next->value > i) {
						struct IntElement* newElement = getIntElement();
						newElement->value = i;
						newElement->next = e->next;
						e->next = newElement;
						break;
					}
				}
			}
		}
	} else {
		struct IntElement* newElement = getIntElement();
		newElement->value = i;
		s->first = s->last = newElement;
	}
	s->size += 1;
}

void appendIntElement(struct IntSet* s, struct IntElement* e) {
	if (s->last != NULL) {
		s->last->next = e;
		s->last = e; 
	} else {
		s->first = s->last = e;
	}
	s->size += 1;
}

struct IntElement* popIntElement(struct IntSet* s) {
	struct IntElement* e = s->first;
	if (e != NULL) { 
		s->first = e->next;
		if (e->next == NULL) {
			s->last = NULL;
		}
		s->size -= 1;
		e->next = NULL;
	}
	return e;
}

/* Return the intersection of two IntSets. IntSets are assumed to be sorted and will not be changed.
taken from: http://www.geeksforgeeks.org/intersection-of-two-sorted-linked-lists/ */
struct IntSet* intersectIntSet(const struct IntSet* setA, const struct IntSet* setB) {
  struct IntSet* intersection = getIntSet();
  struct IntElement* a = setA->first;
  struct IntElement* b = setB->first;
 
  /* Once one or the other list runs out -- we're done */
  while (a != NULL && b != NULL)
  {
    if (a->value == b->value)
    {
       appendInt(intersection, a->value);
       a = a->next;
       b = b->next;
    }
    else if (a->value < b->value) /* advance the smaller list */      
       a = a->next;
    else
       b = b->next;
  }
  return intersection;
}

char isSortedIntSet(struct IntSet* s) {
	if (s->size < 2) {
		return 1;
	}
	for (struct IntElement* e=s->first; e->next!=NULL; e=e->next) {
		if (e->value > e->next->value) {
			return 0;
		}
	}
	return 1;
}

char isSortedUniqueIntSet(struct IntSet* s) {
	if (s->size < 2) {
		return 1;
	}
	for (struct IntElement* e=s->first; e->next!=NULL; e=e->next) {
		if (e->value >= e->next->value) {
			return 0;
		}
	}
	return 1;
}

//int main(int argc, char* argv) {
//	struct IntSet* s = getIntSet();
//
//	printIntSet(s, stdout);
//	addIntSortedNoDuplicates(s, 5);
//	printIntSet(s, stdout);
//	addIntSortedNoDuplicates(s, 5);
//	printIntSet(s, stdout);
//	addIntSortedNoDuplicates(s, 2);
//	printIntSet(s, stdout);
//	addIntSortedNoDuplicates(s, 3);
//	printIntSet(s, stdout);
//	addIntSortedNoDuplicates(s, 2);
//	printIntSet(s, stdout);
//	addIntSortedNoDuplicates(s, 3);
//	printIntSet(s, stdout);
//
//	dumpIntSet(s);
//
//}
