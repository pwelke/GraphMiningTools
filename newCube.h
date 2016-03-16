/*
 * newCube1.h
 *
 *  Created on: Feb 4, 2016
 *      Author: pascal
 */

#ifndef NEWCUBE_H_
#define NEWCUBE_H_

#include <malloc.h>

#include "bitSet.h"
#include "graph.h"

// #define INTCUBE
#define INTCUBE

#ifdef INTCUBE
// CHARACTERISTICS TOOLING

struct SubtreeIsoDataStore {
	// TODO can be moved out to save 128bit per graph.
	int* postorder;
	struct Graph* g;

	struct Graph* h;
	int*** S;
//	char* S;
	size_t elementsInS;
	int foundIso;
};

/** Utility data structure creator.
Cube will store, what is called S in the paper. */
int*** createNewCube(int x, int y);
void createNewBaseCubeFast(struct SubtreeIsoDataStore* info);
void createNewCubeFromBaseFast(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* new);
void dumpNewCubeFast(int*** S, int x, int y);
int containsCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
char checkSanityOfWrite(struct SubtreeIsoDataStore* data, struct Vertex* u, struct Vertex* v);
// assumes that
void addCharacteristic(struct SubtreeIsoDataStore* data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
/** Print a single entry in the cube */
void printNewS(int*** S, int v, int u);
void printNewSDanger(int* data, size_t length);
void printNewCube(int*** S, int gn, int hn);
void printNewCubeCondensed(int*** S, int gn, int hn);
void testSizes(int*** S, int gn, int hn);
int* rawCharacteristics(struct SubtreeIsoDataStore data, struct Vertex* u, struct Vertex* v);

#endif

#ifdef BITCUBE

struct SubtreeIsoDataStore {
	// TODO can be moved out to save 128bit per graph.
	int* postorder;
	struct Graph* g;

	struct Graph* h;
//	int*** S;
	char* S;
	size_t elementsInS;
	int foundIso;
};

/** Utility data structure creator.
Cube will store, what is called S in the paper. */
char* createNewCube(int x, int y);
void createNewBaseCubeFast(struct SubtreeIsoDataStore* info);
void createNewCubeFromBaseFast(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* new);
void dumpNewCubeFast(char* S, int x, int y);
int containsCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
char checkSanityOfWrite(struct SubtreeIsoDataStore* data, struct Vertex* u, struct Vertex* v);
// assumes that
void addCharacteristic(struct SubtreeIsoDataStore* data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
/** Print a single entry in the cube */
#endif



#endif /* NEWCUBE_H_ */
