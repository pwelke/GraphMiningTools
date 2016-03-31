#include <stdio.h>
#include <string.h>
#ifndef PATTERN_EXTRACTOR_H_
#define PATTERN_EXTRACTOR_H_

#include <stdint.h>
#include "../graph.h"

int printHelp();

static uint32_t jenkinsOneAtATimeHash_step(char *key, uint32_t hash);
static uint32_t jenkinsOneAtATimeHash_finalize(uint32_t hash);
static uint32_t pathHashCode(struct ShallowGraph* path);
static struct VertexList* hasEdge(struct Vertex* v, struct Vertex* w);
uint32_t fingerprintTriple(struct Vertex* u, struct Vertex* v, struct Vertex* w, struct VertexList* uv, struct VertexList* vw, struct VertexList* wu, struct ShallowGraphPool* sgp);
struct IntSet* getTripletFingerprintsBruteForce(struct Graph* g, struct ShallowGraphPool* sgp);
struct IntSet* getTripletFingerprintsTriangulation(struct Graph* g, struct ShallowGraphPool* sgp);
int main(int argc, char** argv);

#endif // PATTERN_EXTRACTOR_H_
