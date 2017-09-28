/*
 * testsuite.c
 *
 *  Created on: Sep 26, 2017
 *      Author: pascal
 */

#include <stdio.h>
#include "minunit.h"

#include "../memoryManagement.h"
#include "../graph.h"
#include "../randomGraphGenerators.h"

int tests_run = 0;

struct VertexPool* vp = NULL;
struct ListPool* lp = NULL;
struct GraphPool* gp = NULL;
struct ShallowGraphPool* sgp = NULL;

static char* test_randomOverlapGraphN(int n) {
	struct Graph* g = randomOverlapGraph(n, 0.5, gp);
	mu_assert("error, wrong number of vertices", g->n == n);
	dumpGraph(gp, g);
	return 0;
}

static char* test_randomOverlapGraphM(int n, double p) {
	struct Graph* g = randomOverlapGraph(n, p, gp);

	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			mu_assert("error, edge exists for distance > d", euclideanDistanceWrap(e->startPoint->number, e->endPoint->number, g) < p);
		}
	}
	dumpGraph(gp, g);
	return 0;
}

static char* test_moveOverlapGraphM(int n, double move, double d) {
	struct Graph* g = randomOverlapGraph(n, d, gp);
	moveOverlapGraph(g, move, d, gp);

	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			mu_assert("error, edge exists for distance > d", euclideanDistanceWrap(e->startPoint->number, e->endPoint->number, g) < d);
		}
	}
	dumpGraph(gp, g);
	return 0;
}


static char * all_tests() {
	mu_run_test(test_randomOverlapGraphN(10));
	mu_run_test(test_randomOverlapGraphM(10, 0.5));
	mu_run_test(test_randomOverlapGraphM(10, 0.0));
	mu_run_test(test_randomOverlapGraphM(10, 0.5));
	mu_run_test(test_moveOverlapGraphM(10, 1, 0.5));
	mu_run_test(test_moveOverlapGraphM(10, 100, 0.5));
	mu_run_test(test_moveOverlapGraphM(10, 0.5, 0.5));
	return 0;
}

int main(int argc, char **argv) {
	vp = createVertexPool(10);
	lp = createListPool(10);
	gp = createGraphPool(10, vp, lp);
	sgp = createShallowGraphPool(10, lp);


	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}
