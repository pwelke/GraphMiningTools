#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "../loading.h"
#include "../cactustree.h"
#include "../subtreeIsomorphism.h"
#include "../graph.h"
#include "../outerplanar.h"

int main(int argc, char **argv){
    if(argc<2){
        printf("Please specify input file as first argument.\n");
        return EXIT_FAILURE;
    }

    struct ListPool *lPool;
    struct GraphPool *gPool;
    struct ShallowGraphPool *sgPool;
    struct Graph *g, *h;
    struct VertexList *hlp;
    int i,j,k;
    char cResult, tResult;
    
    lPool=createListPool(100000);
    if(lPool==NULL){
        printf("couldn't allocate listPool\n");
        return EXIT_FAILURE;
    }
    gPool= createGraphPool(5000, createVertexPool(50000), lPool);
    sgPool=createShallowGraphPool(50000,lPool);
    if(gPool==NULL || sgPool==NULL){
        printf("couldn't allocate graph pools");
        freeListPool(lPool);
        freeAllPools(gPool, sgPool);
        return EXIT_FAILURE;
    }
    createFileIterator(argv[1],gPool);
    g=iterateFile(&aids99VertexLabel,&aids99EdgeLabel);
    while(g){
        for(i=2;i<20;++i){
            h=createGraph(i,gPool);
            if(h==NULL){
                printf("couldn't allocate path of size %i.\n",i);
                return EXIT_FAILURE;
            }
            for(j=0;j<i-1;++j) addEdgeBetweenVertices(j,j+1,NULL,h,gPool);
            cResult=cactusTreeSubIso(g,h,gPool,sgPool);
            tResult=subtreeCheck(g,h,gPool,sgPool);
            if(cResult != tResult){
                printf("\ndifferent results %i %i for graph %i (Tree? %i)and path of length %i.\n",cResult, tResult, isTree(g), g->number,i);
                for(k=0;k<g->n;++k){
                    printf("%i %c:\n", g->vertices[k]->number, *g->vertices[k]->label);
                        for(hlp = g->vertices[k]->neighborhood; hlp; hlp=hlp->next){
                            printf("\t(%i %c)", hlp->endPoint->number, *hlp->label);
                        }   
                        printf("\n");
                        printf(" %i %i %i", 1==1, 1==2, 3==1);
                }   

                return EXIT_FAILURE;
            }
        }
    }
}



