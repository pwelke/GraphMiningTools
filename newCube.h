/*
 * newCube.h
 *
 *  Created on: Feb 4, 2016
 *      Author: pascal
 */

#ifndef NEWCUBE_H_
#define NEWCUBE_H_

#include <malloc.h>

#include "bitSet.h"
#include "graph.h"
#include "inttypes.h"

 #define BITCUBE
// #define BYTECUBE
//#define INTCUBE

#ifdef INTCUBE
// CHARACTERISTICS TOOLING

struct SubtreeIsoDataStore {
	// TODO can be moved out to save 128bit per graph.
	int* postorder;
	struct Graph* g;

	struct Graph* h;
	int*** S;
	size_t elementsInS;
	int foundIso;
};

void createNewCubeForSingletonPattern(struct SubtreeIsoDataStore* info);
void createNewCubeForEdgePattern(struct SubtreeIsoDataStore* info);
void createNewCubeFromBase(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* new);
void dumpNewCube(int*** S, int x);

int containsCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
char checkSanityOfWrite(struct SubtreeIsoDataStore* data, struct Vertex* u, struct Vertex* v);
void addCharacteristic(struct SubtreeIsoDataStore* data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
int* rawCharacteristics(struct SubtreeIsoDataStore data, struct Vertex* u, struct Vertex* v);

void printNewCubeRow(int*** S, int v, int u);
void printNewSDanger(int* data, size_t length);
void printNewCube(int*** S, int gn, int hn);
void printNewCubeCondensed(int*** S, int gn, int hn);

void testNewCubeSizes(int*** S, int gn, int hn);

#endif

#ifdef BYTECUBE
// CHARACTERISTICS TOOLING

struct SubtreeIsoDataStore {
	// TODO can be moved out to save 128bit per graph.
	int* postorder;
	struct Graph* g;

	struct Graph* h;
	uint8_t*** S;
	size_t elementsInS;
	int foundIso;
};

void createNewCubeForSingletonPattern(struct SubtreeIsoDataStore* info);
void createNewCubeForEdgePattern(struct SubtreeIsoDataStore* info);
void createNewCubeFromBase(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* new);
void dumpNewCube(uint8_t*** S, int x);

int containsCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
char checkSanityOfWrite(struct SubtreeIsoDataStore* data, struct Vertex* u, struct Vertex* v);
void addCharacteristic(struct SubtreeIsoDataStore* data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
uint8_t* rawCharacteristics(struct SubtreeIsoDataStore data, struct Vertex* u, struct Vertex* v);

void printNewCubeRow(uint8_t*** S, int v, int u);
void printNewSDanger(uint8_t* data, size_t length);
void printNewCube(uint8_t*** S, int gn, int hn);
void printNewCubeCondensed(uint8_t*** S, int gn, int hn);

void testNewCubeSizes(uint8_t*** S, int gn, int hn);

#endif

#ifdef BITCUBE

struct SubtreeIsoDataStore {
	// TODO can be moved out to save 128bit per graph.
	int* postorder;
	struct Graph* g;

	struct Graph* h;
	uint8_t* S;
	size_t elementsInS;
	int foundIso;
};

uint8_t* createNewCube(size_t gn, size_t hn);
void createNewCubeForSingletonPattern(struct SubtreeIsoDataStore* info);
void createNewCubeForEdgePattern(struct SubtreeIsoDataStore* info);
void createNewCubeFromBase(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* new);
void dumpNewCube(uint8_t* S, int x);

int containsCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
char checkSanityOfWrite(struct SubtreeIsoDataStore* data, struct Vertex* u, struct Vertex* v);
void addCharacteristic(struct SubtreeIsoDataStore* data, struct Vertex* y, struct Vertex* u, struct Vertex* v);
//int* rawCharacteristics(struct SubtreeIsoDataStore data, struct Vertex* u, struct Vertex* v);

//void printNewCubeRow(uint8_t* S, int v, int u);
//void printNewSDanger(uint8_t* data, size_t length);
//void printNewCube(uint8_t* S, int gn, int hn);
void printNewCubeCondensed(uint8_t* S, int gn, int hn, FILE* out);
void printNewCube(uint8_t* S, int gn, int hn, FILE* out);

//void testNewCubeSizes(uint8_t* S, int gn, int hn);

#endif



#endif /* NEWCUBE_H_ */
