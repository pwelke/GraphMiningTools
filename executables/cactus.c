#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "../loading.h"
#include "../cactustree.h"
#include "../iterativeSubtreeIsomorphism.h"


int main(int argc, char **argv){
    if(argc < 3){
        printf("No input files specified\n");
        return EXIT_FAILURE;
    }
    struct GraphPool *gPool;
    struct ShallowGraphPool * sgPool=NULL;
    if (!(gPool = createGraphPool(1,createVertexPool(10),createListPool(12)))){
        printf("Error initialising the graph pool");
        return EXIT_FAILURE;
    }
    struct Graph *graph, *pattern;
    graph = readSimpleFormat(argv[1], 1, gPool, 1);
    if(!graph){
        printf("Could not read file.\n");
        freeAllPools(gPool, sgPool);
        return EXIT_FAILURE;
    }
    pattern=readSimpleFormat(argv[2], 1, gPool, 1);
    if(!(pattern)){
        dumpGraph(gPool, graph);
        printf("Could not read pattern file.\n");
        freeAllPools(gPool, sgPool);
        return EXIT_FAILURE;
    }
    int i;
    struct VertexList *hlp;
    printf("Testing if graph works:\n");
    for(i=0;i<graph->n;++i){
        printf("%i %c:\n", graph->vertices[i]->number, *graph->vertices[i]->label);
        for(hlp = graph->vertices[i]->neighborhood; hlp; hlp=hlp->next){
            printf("\t(%i %c)", hlp->endPoint->number, *hlp->label);
        }
        printf("\n");
    }
    printf("Testing if pattern works:\n");
    for(i=0;i<pattern->n;++i){
        printf("%i %c:\n", pattern->vertices[i]->number, *pattern->vertices[i]->label);
        for(hlp = pattern->vertices[i]->neighborhood; hlp; hlp=hlp->next){
            printf("\t(%i %c)", hlp->endPoint->number, *hlp->label);
        }
        printf("\n");
    }
    if (!(sgPool = createShallowGraphPool(3, gPool->listPool))){
        printf("Could not initialise shallow graph pool");
        dumpGraph(gPool,graph);
        dumpGraph(gPool, pattern);
        freeAllPools(gPool,sgPool);
        return EXIT_FAILURE;
    }
    // cactusTreeSubIso(graph, pattern, gPool, sgPool);
//    char check = subtreeCheck(graph, pattern, gPool, sgPool);
    char check = isSubtree(graph, pattern, gPool);
    printf("Return value: %i\n", check);

    dumpGraph(gPool,graph);
    dumpGraph(gPool,pattern);

    freeAllPools(gPool, sgPool);
}

