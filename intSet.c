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

void dumpIntSet(struct IntSet* s) {
	if (s->size > 0) {
		dumpIntElements(s->first);
	}
	free(s);
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
