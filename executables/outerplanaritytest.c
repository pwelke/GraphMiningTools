#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../cs_Outerplanar.h"
#include "../listComponents.h"
#include "../listCycles.h"
#include "../outerplanar.h"
#include "../treeCenter.h"


/**
 * Print --help message
 */
int printHelp() {
#include "../o/help/outerplanaritytestHelp.help"
	unsigned char* help = executables_outerplanaritytestHelp_txt;
	int len = executables_outerplanaritytestHelp_txt_len;
	if (help != NULL) {
		int i=0;
		for (i=0; i<len; ++i) {
			fputc(help[i], stdout);
		}
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Could not read help file\n");
		return EXIT_FAILURE;
	}
}


/**
 * create a graph that is isomorphic to the induced subgraph defined by edgeList
 * this implementation ensures that the vertices in the resulting graph correspond to the order in
 * which the vertices appear in edgeList. E.g. g->vertices[0] == edgeList->edges->startPoint.
 * 
 * this variant stores the old ->number of vertices in ->lowPoint of the new vertices
 */
struct Graph* shallowGraphToGraphWithIDs(struct ShallowGraph* edgeList, struct GraphPool* gp) {
	struct Graph* g = getGraph(gp);
	struct VertexList* e;
	int n = 0;
	int i;

	/* clear all ->lowPoint s */
	for (e=edgeList->edges; e; e=e->next) {
		e->startPoint->lowPoint = 0;
		e->endPoint->lowPoint = 0;
	}

	/* count number of distinct vertices
	 * and number vertices accordingly*/
	for (e=edgeList->edges; e; e=e->next) {
		if (e->startPoint->lowPoint == 0) {
			++n;
			e->startPoint->lowPoint = n;
		}
		if (e->endPoint->lowPoint == 0) {
			++n;
			e->endPoint->lowPoint = n;
		}
	}

	/* set vertex number of new Graph to n, initialize stuff*/
	setVertexNumber(g, n);
	g->m = edgeList->m;

	for (i=0; i<n; ++i) {
		g->vertices[i] = getVertex(gp->vertexPool);
		g->vertices[i]->number = i;
	}

	/* add copies of edges and labels of vertices */
	for (e=edgeList->edges; e; e=e->next) {
		struct VertexList* f = getVertexList(gp->listPool);
		f->startPoint = g->vertices[e->startPoint->lowPoint - 1];
		f->endPoint = g->vertices[e->endPoint->lowPoint - 1];
		f->label = e->label;
		f->startPoint->label = e->startPoint->label;
		f->endPoint->label = e->endPoint->label;

        // difference to shallowGraphToGraph
        f->startPoint->lowPoint = e->startPoint->number;
        f->endPoint->lowPoint = e->endPoint->number;
        // end difference

		addEdge(g->vertices[e->startPoint->lowPoint - 1], f);
		addEdge(g->vertices[e->endPoint->lowPoint - 1], inverseEdge(f, gp->listPool));
	}

	return g;
} 


void listHamiltonianCycles(struct ShallowGraph* biconnectedComponents, char extendToHamiltonianBlocks, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
    /**
    An outerplanar graph is a graph that can be drawn in the plane such that
    (1) edges only intersect at vertices and
    (2) each vertex can be reached from the outer face without crossing an edge.

    A graph is outerplanar if and only if each of its biconnected components is outerplanar.
    */ 
    struct ShallowGraph* comp;
    char isOuterplanar = 1;
    char isFirstCycle = 1; // for pretty printing
    int nonOuterplanarHamiltonianCycles = 0;
    int componentNumber;

    printf(", \"hamiltonianCycles\": {");

    for (comp = biconnectedComponents, componentNumber=-1; comp!=NULL; comp=comp->next) {
        if (comp->m > 1) {
            struct Graph* block = shallowGraphToGraphWithIDs(comp, gp);
            // returns null if block is not outerplanar
            struct ShallowGraph* hamiltonianCycle = __getCycleAndDiagonals(block, sgp);

            if (hamiltonianCycle != NULL) {
                if (isFirstCycle) {
                    isFirstCycle = 0;
                } else {
                    printf(", ");
                }
                printf("%i: [%i", componentNumber, hamiltonianCycle->edges->startPoint->lowPoint);

                struct VertexList* e;
                for (e = hamiltonianCycle->edges->next; e!=NULL; e=e->next) {
                    printf(", %i", e->startPoint->lowPoint);
                }
                printf("]");

                
                
                // cleanup
                dumpShallowGraphCycle(sgp, hamiltonianCycle);

            } else {
                isOuterplanar = 0;
                if (extendToHamiltonianBlocks) {
                    struct Graph* newBlock = shallowGraphToGraphWithIDs(comp, gp);
                    struct Graph* blockCopy = cloneGraph(newBlock, gp);
                    struct ShallowGraph* hamiltonianCycles = listCyclesOfLength(blockCopy, blockCopy->n, sgp);
                    if (hamiltonianCycles) {
                        // listCyclesOfLength returns a cycle of ShallowGraphs which needs to be broken
                        hamiltonianCycles->prev->next = NULL;
                        rebaseShallowGraphs(hamiltonianCycles, block);

                        for (struct ShallowGraph* h=hamiltonianCycles; h!=NULL; h=h->next) {
                            nonOuterplanarHamiltonianCycles += 1;
                            if (isFirstCycle) {
                                printf("[%i", h->edges->startPoint->lowPoint);
                                isFirstCycle = 0;
                            } else {
                                printf(", [%i", h->edges->startPoint->lowPoint);
                            }
                            struct VertexList* e;
                            for (e = h->edges->next; e!=NULL; e=e->next) {
                                printf(", %i", e->startPoint->lowPoint);
                            }
                            printf("]");
                        }
                        // cleanup
                        dumpShallowGraphCycle(sgp, hamiltonianCycles);
                    }  
                    dumpGraph(gp, blockCopy);
                }
            }
            --componentNumber;
            dumpGraph(gp, block);
        } 	
    }

    if (isOuterplanar) {
        printf("}, \"isOuterplanar\": true");
    } else {
        printf("}, \"isOuterplanar\": false");
    }

    if (extendToHamiltonianBlocks) {
        if (nonOuterplanarHamiltonianCycles) {
            printf(", \"hasNonOuterplanarHamiltonianBlock\": true, \"nonOuterplanarHamiltonianCycles\": %i", nonOuterplanarHamiltonianCycles);
        } else {
            printf(", \"hasNonOuterplanarHamiltonianBlock\": false, \"nonOuterplanarHamiltonianCycles\": %i", nonOuterplanarHamiltonianCycles);
        }
    }
}

void addSpiderWebEdges(struct BBTree* bbTree, struct ShallowGraphPool* sgp) {

}


char printShortcutEdges(struct Vertex* lastInteresting, struct Vertex* v, char notFirst, int* originalIDs) {
    v->visited = 1;
    if (v->lowPoint) {
        if (lastInteresting) {
            char isNeighbor = 0;
            for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
                if (e->endPoint == lastInteresting) {
                    isNeighbor = 1;
                    break;
                }
            }
            if (!isNeighbor) {
                if (notFirst) {
                    printf(", ");
                }
                printf("[%d, %d]", originalIDs[lastInteresting->number], originalIDs[v->number]);
                notFirst = 1;
            }
        }
    }
    for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
        if (e->endPoint->visited == 0) {
            if (v->lowPoint) {
                notFirst = printShortcutEdges(v, e->endPoint, notFirst, originalIDs);
            } else {
                notFirst = printShortcutEdges(lastInteresting, e->endPoint, notFirst, originalIDs);
            }
        }
    }
    return notFirst;
}

void addShortcutEdges(struct BBTree* bbTree) {
    int root = 0;
    /* mark interesting vertices, keep the last leaf or join as root */
    for (int v=0; v<bbTree->tree->n; ++v) {
        bbTree->tree->vertices[v]->visited = 0;
        if (degree(bbTree->tree->vertices[v]) != 2) {
            bbTree->tree->vertices[v]->lowPoint = 1;
            root = v;
        } else {
            bbTree->tree->vertices[v]->lowPoint = 0;
        } 
    }
    if (bbTree->blocks) {
        for (int v=0; v<bbTree->blocks->n; ++v) {
            bbTree->blocks->vertices[v]->lowPoint = 2;
        }
    }

    printf(", \"shortcutEdges\": [");
    printShortcutEdges(NULL, bbTree->tree->vertices[root], 0, bbTree->originalIDs);
    printf("]");
}

void printBlockVertices(struct BBTree* bbTree) {

    printf(", \"blocks\": {");
    if (bbTree->blocks) {
        for (int i=0; i<bbTree->blocks->n; ++i) {
            if (i>0) { printf(", "); }
            printf("\"%d\": [", -1 - i);
            char start = 1;
            // init vertices in g (!)
            for (struct VertexList* e=bbTree->blockComponents[i]->edges; e!=NULL; e=e->next) {
                e->startPoint->visited = 0;
                e->endPoint->visited = 0;
            }
            for (struct VertexList* e=bbTree->blockComponents[i]->edges; e!=NULL; e=e->next) {
                if (e->startPoint->visited == 0) {
                    if (start) { 
                        start = 0; 
                    } else { 
                        printf(", ");
                    }
                    printf("%d", e->startPoint->number);
                    e->startPoint->visited = 1;
                }
                if (e->endPoint->visited == 0) {
                    printf(", ");
                    printf("%d", e->endPoint->number);
                    e->endPoint->visited = 1;
                }
            }
            putc(']', stdout);
        }
    }
    putc('}', stdout);
}


int main(int argc, char** argv) {

    /* object pools */
    struct ListPool *lp;
    struct VertexPool *vp;
    struct ShallowGraphPool *sgp;
    struct GraphPool *gp;

    /* pointer to the current graph which is returned by the input iterator  */
    struct Graph* g = NULL;

    char extendToHamiltonianBlocks = 0;
    char spiderWeb = 0;
    char shortcutting = 0;
    char isFirstGraph = 1;

    /* parse command line arguments */
    int arg;
    const char* validArgs = "hlsw";
    for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
        switch (arg) {
        case 'h': // Help
            printHelp();
            return EXIT_SUCCESS;
        case 'l': // List Hamiltonian cycles in non-outerplanar Hamiltonian blocks
            extendToHamiltonianBlocks = 1;
            break;
        case 'w': // spider Web connectivity
            spiderWeb = 1;
            break;
        case 's':
            shortcutting = 1;
            break;
        case '?':
            return EXIT_FAILURE;
            break;
        }
    }

    /* init object pools */
    lp = createListPool(10000);
    vp = createVertexPool(10000);
    sgp = createShallowGraphPool(1000, lp);
    gp = createGraphPool(100, vp, lp);

    /* initialize the stream to read graphs from 
        check if there is a filename present in the command line arguments 
        if so, open the file, if not, read from stdin */
    if (optind < argc) {
        char* filename = argv[optind];
        /* if the present filename is not '-' then init a file iterator for that file name */
        if (strcmp(filename, "-") != 0) {
            createFileIterator(filename, gp);
        } else {
            createStdinIterator(gp);
        }
    } else {
        createStdinIterator(gp);
    }

    printf("[");

    /* iterate over all graphs in the database */
    while ((g = iterateFile())) {

        // list handling for graphs
        if (isFirstGraph) {
            isFirstGraph = 0;
        } else {
            printf(",");
        }

        /* if there was an error reading some graph the returned n will be -1 */
        if (g->n != -1) {
            if (g->m < 1) {
                printf("\n{\"graph\": %i, \"hamiltonianCycles\": [], \"isOuterplanar\": true", g->number);
            } else {
                printf("\n{\"graph\": %i", g->number);
                
                struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
                listHamiltonianCycles(biconnectedComponents, extendToHamiltonianBlocks, gp, sgp);

                if (shortcutting || spiderWeb) {
                    struct BBTree* bbTree = createFancyBlockAndBridgeTree(biconnectedComponents, g, gp, sgp);

                    printBlockVertices(bbTree);

                    if (shortcutting) {
                        addShortcutEdges(bbTree);
                    } 
                    if (spiderWeb) {
                        addSpiderWebEdges(bbTree, sgp);
                    }
                    dumpFancyBBTree(gp, sgp, bbTree);
                } else {
                    /* cleanup */
                    dumpShallowGraphCycle(sgp, biconnectedComponents);
                }
            }
        } 

        putc('}', stdout);

        /* garbage collection */
        dumpGraph(gp, g);
    }

    printf("\n]\n");

    /* global garbage collection */
    destroyFileIterator();
    freeGraphPool(gp);
    freeShallowGraphPool(sgp);
    freeListPool(lp);
    freeVertexPool(vp);

    return EXIT_SUCCESS;
}	