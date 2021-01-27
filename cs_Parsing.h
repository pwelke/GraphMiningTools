#ifndef CS_PARSING_H_
#define CS_PARSING_H_

#include <stdio.h>
#include "graph.h"

/* caching */
// standard cache size for parseCString (in byte)
#define CS_STRING_CACHE_SIZE 2048

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
struct ShallowGraph* string2cstring(char* str, struct ShallowGraphPool* sgp);

#endif
