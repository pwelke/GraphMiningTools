#ifndef CS_PARSING_H_
#define CS_PARSING_H_

#include <stdio.h>

/* caching */
struct VertexList* getTerminatorEdge(struct ListPool *p);
struct VertexList* getInitialisatorEdge(struct ListPool *p);
char getTerminatorSymbol();
char getInitialisatorSymbol();

/* output */
void printCanonicalString(struct ShallowGraph *s, FILE* stream);
void printCanonicalStrings(struct ShallowGraph *s, FILE* stream);
char* canonicalStringToChar(struct ShallowGraph* string);

/* input */
struct ShallowGraph* parseCString(FILE* stream, char* buffer, struct ShallowGraphPool* sgp);

#endif
