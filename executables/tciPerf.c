#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

#include "../loading.h"
#include "../cactustree.h"
#include "../subtreeIsomorphismLabeled.h"
#include "../graph.h"
#include "../outerplanar.h"
#include "../intMath.h"

int main(int argc, char **argv){
    if(argc<5 || (argv[3][0]=='t' && argc <6)){
        printf("use as perf <input file> <output file> <pattern class> <pattern size> \n");
        printf("\twhere <pattern class is one of:\n");
        printf("\t\t- t for full trees, followed by <pattern size> = <depth> <num childs>\n");
        printf("\t\t- p for paths of length <pattern size>\n");
        printf("\t\t- r for a random tree with <pattern size> vertices\n");
        return EXIT_FAILURE;
    }

    struct ListPool *lPool;
    struct GraphPool *gPool;
    struct ShallowGraphPool *sgPool;
    struct Graph *g, *h;
    struct VertexList *hlp;
    int i,j,k,l,size1,size2;
    char cResult, tResult;
    double seconds, ratio;
    clock_t time1, time2;
    FILE *outFile;
    
    lPool=createListPool(2000);
    if(lPool==NULL){
        printf("couldn't allocate listPool\n");
        return EXIT_FAILURE;
    }
    gPool= createGraphPool(50, createVertexPool(500), lPool);
    sgPool=createShallowGraphPool(50000,lPool);
    if(gPool==NULL || sgPool==NULL){
        printf("couldn't allocate graph pools");
        freeListPool(lPool);
        freeAllPools(gPool, sgPool);
        return EXIT_FAILURE;
    }
    size1=atoi(argv[4]);
    switch (argv[3][0]){
        case 't':
            size2=atoi(argv[5]);
            printf("Tree with %i levels, %i childs ==> %i vertices\n",size1, size2, (pow(size2,size1+1)-1)/(size2-1));
            h=createGraph((pow(size2,size1+1)-1)/(size2-1),gPool);
            h->m=h->n -1;
            for(i=0;i<size1;++i){
                l=(pow(size2,i+1)-1)/(size2-1);
                for(j=(pow(size2,i)-1)/(size2-1);j<l;++j){
                    for(k=1;k<=size2;++k){
                        printf("%i %i\n",j,j*size2+k);
                        addEdgeBetweenVertices(j,j*size2+k, NULL ,h, gPool);
                    }
                }
            }
            break;
        case 'p':
            h=createGraph(size1,gPool);
            for(j=0;j<size1-1;++j){
                    addEdgeBetweenVertices(j,j+1,NULL,h,gPool);
            }
            break;
        case 'r':
            h=createGraph(size1,gPool);
            printf("allocated %i vertices\n",h->n);
            srand(time(NULL));
            for(i=0;i<size1;++i)h->vertices[i]->d=i;
            do{
                j=rand() % size1;
                k=rand() % size1;
                printf("\n");
                if(j!=k && h->vertices[j]->d != h->vertices[k]->d){
                    addEdgeBetweenVertices(j,k,NULL,h, gPool);
                    printf("%i %i\n",j,k);
                        // could be done with union-find but no b/c of reasons...
                    if(h->vertices[j]->d < h->vertices[k]->d){
                        l=h->vertices[k]->d;
                        for(i=0;i<size1;++i) {
                            if (h->vertices[i]->d==l){
                                h->vertices[i]->d=h->vertices[j]->d;
                            }
                        }
                    }
                    else{
                        l=h->vertices[j]->d;
                        for(i=0;i<size1;++i){
                            if(h->vertices[i]->d==l){
                                h->vertices[i]->d=h->vertices[k]->d;
                            }
                        }
                    }
                }
            }while(h->m<size1-1);
            break;
        default:
            printf("Try again\n");
            return EXIT_FAILURE;
    }

        for(k=0;k<h->n;++k){
            printf("%i %i:\n", h->vertices[k]->number, h->vertices[k]->d);
                for(hlp = h->vertices[k]->neighborhood; hlp; hlp=hlp->next){
                    printf("\t(%i)", hlp->endPoint->number);
                }   
                printf("\n");
        }
                        

    outFile=fopen(argv[2], "w+");

    createFileIterator(argv[1],gPool);
    g=iterateFile(&aids99VertexLabel,&aids99EdgeLabel);
    i=0;
    while(g){
        printf("Testing graph %i\n",g->number);
        if((g->m != g->n -1) || !isConnected(g)){
            dumpGraph(gPool,g);
            g=iterateFile(&aids99VertexLabel,&aids99EdgeLabel);
            continue;
        }
        printf("is a tree\n");
            time1=clock();
            cResult=cactusTreeSubIso(g,h,gPool,sgPool);
            time2=clock();
            seconds=((double) (time2-time1))/CLOCKS_PER_SEC;
            time1=clock();
            tResult=subtreeCheckL(g,h,gPool,sgPool);
            time2=clock();
            ratio=((double) (time2-time1))/CLOCKS_PER_SEC;
            fprintf(outFile,"%c %.10f %.10f %.20f\n", tResult + 42 + 3*(1-tResult), seconds , ratio , seconds/ratio);
            if(cResult != tResult){
                printf("\ndifferent results %i %i for graph %i (Tree? %i)and graph of size %i.\n",cResult, tResult, g->number, isTree(g),h->n);
                for(k=0;k<g->n;++k){
                    printf("%i :\n", g->vertices[k]->number);
                        for(hlp = g->vertices[k]->neighborhood; hlp; hlp=hlp->next){
                            printf("\t(%i %c)", hlp->endPoint->number, *hlp->label);
                        }   
                        printf("\n");
                }
                printf("\npatterni %i \n",h->n);
                for(k=0;k<h->n;++k){
                    printf("%i:\n", h->vertices[k]->number);
                        for(hlp = h->vertices[k]->neighborhood; hlp; hlp=hlp->next){
                            printf("\t(%i)", hlp->endPoint->number);
                        }   
                        printf("\n");
                }

                return EXIT_FAILURE;
            }
        if(cResult) ++i;       
        dumpGraph(gPool,g);
        g=iterateFile(&aids99VertexLabel,&aids99EdgeLabel);
    }
    printf("There were %i matches\n", i);
    destroyFileIterator();
    fclose(outFile);
    dumpGraph(gPool,h);
    freeVertexPool(gPool->vertexPool);
    freeListPool(gPool->listPool);
    freeGraphPool(gPool);
    freeShallowGraphPool(sgPool);
    
}



