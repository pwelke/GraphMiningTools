#include <stdlib.h>
#include <malloc.h>
#include <limits.h>

#include "graph.h"
#include "listComponents.h"
#include "cactustree.h"
#include "bipartiteMatching.h"
// Test for subgraphisomorphismus between a `pattern` tree and a cactus `graph`
int cactusTreeSubIso(struct Graph * graph, struct Graph *pattern, struct GraphPool *gPool, struct ShallowGraphPool *sgPool){

    struct ShallowGraph *bComponents = listBiconnectedComponents(graph, sgPool);
    int numComponents =0;
    
    // Setting distances to vertex_0.
    struct ShallowGraph *shlp;
    struct VertexList *lhlp;
    struct Vertex ** representatives;
    struct Vertex * componentRepresentative, *rootRepresentative;
    struct Characteristics *res;
    int i = 0;
    representatives = malloc(graph->n * sizeof(struct Vertex *));
    //TODO remove debug
    /*
    printf("MAIN\n");
    for(shlp=bComponents;shlp;shlp=shlp->next){
        printf("Component: %i\n", numComponents++);
        for(lhlp=shlp->edges; lhlp; lhlp=lhlp->next){
            printf ("\t(%i, %i)",lhlp->startPoint->number,lhlp->endPoint->number);
        }
        printf("\n");
    }
    //TODO remove when debug is removed
    numComponents=0;
    */
    if(representatives == NULL){
        //TODO clean up
        return 0;
    }
    for(;i<graph->n;++i){
        representatives[i]=NULL;
        graph->vertices[i]->d=-1;
    }
    for(shlp=bComponents;shlp;shlp=shlp->next){
        ++numComponents;
        componentRepresentative = shlp->edges->startPoint;
        rootRepresentative = representatives[shlp->edges->startPoint->number];
        //We will initialize the LUT from old graph vertices to component numbers.
        for(lhlp=shlp->edges; lhlp; lhlp=lhlp->next){
            representatives[lhlp->endPoint->number]=componentRepresentative;
        }
        //Now reset the representative for the root as we may have set it.
        //This way the representative of a component root will only be set during processing of another component,
        //where it is not a root.
        representatives[shlp->edges->startPoint->number]=rootRepresentative;

    }
    //set entries in the LUT
    //and set all roots to unvisited.
    i=-1;
    for(shlp=bComponents;shlp;shlp=shlp->next){
        if(shlp->edges->startPoint->d< 0) shlp->edges->startPoint->d=++i;
        shlp->edges->startPoint->visited=0;
    }
    //printf("Allocating component tree\n");
    struct Graph * componentTree = createGraph(numComponents, gPool);
    struct ShallowGraph **rootedComponents=calloc(numComponents,sizeof(struct ShallowGraph *));
    struct ShallowGraph *tmp;
    int oldNumber, newNumber;
    struct Vertex *currentRoot;
    //TODO error behviour for componentTree 
    //printf("constructing component Tree\n");
    shlp=bComponents;
    while(shlp){
        currentRoot=shlp->edges->startPoint;
        oldNumber=currentRoot->number;
        newNumber=currentRoot->d;
        //No root should be processed twice, to avoid double edges.
        if(!(currentRoot->visited)){
            currentRoot->visited=1;
            //setting backreference to original graph.
            componentTree->vertices[newNumber]->d=oldNumber;
            //printf("testing for representative\n");
            if(representatives[oldNumber]){
                //printf("adding edge between original (%i,%i) and new (%i,%i)\n",representatives[shlp->edges->startPoint->number]->number, 
                        //shlp->edges->startPoint->number, newNumber, representatives[oldNumber]->d);
                addEdgeBetweenVertices(newNumber,representatives[oldNumber]->d,NULL,componentTree,gPool);
            }
            shlp->data=1;
            rootedComponents[newNumber]=shlp;
            shlp=shlp->next;
            rootedComponents[newNumber]->next=NULL; 
        }
        else{
            tmp=shlp->next;
            //data is used to keep track of componnents assigned to a root.
            shlp->data=rootedComponents[newNumber]->data+1;
            shlp->next=rootedComponents[newNumber];
            rootedComponents[newNumber]=shlp;
            shlp=tmp;
        }
    }
    free(representatives);
    res=processRootTree(componentTree->vertices[graph->vertices[0]->d], graph, rootedComponents, pattern, gPool, sgPool);
    for(i=0;i<numComponents;++i)
       dumpShallowGraphCycle(sgPool, rootedComponents[i]);
    free(rootedComponents);
    dumpGraph(gPool,componentTree);
    if(hasSolution(res)){
        freeCharacteristics(res);
        return 1;
    }
    freeCharacteristics(res);
    return 0;
}
struct Characteristics *processRootTree(struct Vertex *root, struct Graph *graph, struct ShallowGraph **rootedComponents, struct Graph *pattern, struct GraphPool *gPool, struct ShallowGraphPool *sgPool){
    struct VertexList *ehlp;
    struct VertexList **deletedEdges;
    struct Characteristics *chars=NULL, *newChars, *res;
    int numComponents, treeNumber=0;
    char exNext;
    root->visited=root->visited | 1;
    for(ehlp=root->neighborhood;ehlp;ehlp=ehlp->next){
        if(1^(ehlp->endPoint->visited)){
            res=processRootTree(ehlp->endPoint, graph, rootedComponents, pattern, gPool, sgPool);
            if(hasSolution(res)){
                freeCharacteristics(chars);
                return res;
            }
            chars=mergeCharacteristics(chars,res);
        }
    }
    //printf("root tree %i", root->number);
    //printCharacteristics(chars);

    numComponents=rootedComponents[root->number]->data;
    deletedEdges=malloc(numComponents*sizeof(struct VertexList *));
    newChars=NULL;
    initSpanningTree(rootedComponents[root->number], deletedEdges,1);
    treeNumber=0;
    do{
        newChars=processComponentTree(graph, graph->vertices[root->d], graph->vertices[root->d], treeNumber, rootedComponents, chars, newChars, pattern, gPool, sgPool);
        exNext=nextSpanningTree(rootedComponents[root->number], deletedEdges,1);
        ++treeNumber;
    }while(exNext);
    freeCharacteristics(chars);
    free(deletedEdges);
    return newChars;
}

struct Characteristics *processComponentTree(struct Graph *graph, struct Vertex *root, struct Vertex *componentRoot, int treeNumber, struct ShallowGraph **rootedComponents, struct Characteristics *oldChars, struct Characteristics *newChars, struct Graph *pattern, struct GraphPool *gPool, struct ShallowGraphPool *sgPool){
    struct VertexList *ehlp, **deletedEdges;
    struct Characteristics *ret=newChars;
    char exNext;
    int wTreeNumber=0;
    //printf("\nComproot %i root%i \n",componentRoot->number, root->number);
    for(ehlp=root->neighborhood;ehlp;ehlp=ehlp->next){
        //no need to check for visited as flags imply a arborescence with no back edges =]
        //printf("\t neighbour is %i flag: %i\n",ehlp->endPoint->number,ehlp->flag);
        if((ehlp->flag & 1)){
            ret=processComponentTree(graph, ehlp->endPoint, componentRoot, treeNumber, rootedComponents, oldChars, ret, pattern, gPool, sgPool);
            //printf("After returning from %i:\n",ehlp->endPoint->number);
            //printCharacteristics(ret);
        }
    }
    /*
    printf("\ncomproot number is %i \n",componentRoot->number);

    printf("\nroot number is %i\n",root->number);
    printf("----------old----------\n");
    printCharacteristics(oldChars);
    printf("--------new-----------\n");
    printCharacteristics(ret);
    */
    int i =0;
    if((componentRoot->number != root->number) || root->number==0){
        if(root->d <0){// not a component root
            for(;i<pattern->n;++i)
                ret=characteristics(root, treeNumber, 0, oldChars, ret, pattern->vertices[i], pattern->n, gPool);
        }
        else{
            deletedEdges = malloc((rootedComponents[root->d]->data)*sizeof(struct VertexList *));
            initSpanningTree(rootedComponents[root->d], deletedEdges,2);
            wTreeNumber=0;
            do{
                for(i=0;i<pattern->n;++i)
                    ret= characteristics(root, treeNumber, wTreeNumber, oldChars, ret, pattern->vertices[i], pattern->n, gPool);
                exNext=nextSpanningTree(rootedComponents[root->d], deletedEdges,2);
                ++wTreeNumber;
            }while(exNext);
            free(deletedEdges);
        }
    }
    return ret;
}

struct Characteristics *characteristics(struct Vertex *w, const int treeID, const int wTreeID, struct Characteristics *oldCharacteristics, struct Characteristics *newCharacteristics, struct Vertex *patternVertex, int patternSize, struct GraphPool *gPool){
    struct Characteristics *ret=newCharacteristics;
    struct VertexList *edgehlp, *edgehlp2, *edgehlp3, *edgehlp4, *tmprm=NULL;
    struct Graph *bipartite;
    int i=0, j=0, numTree=0, numPattern, matchingSize, numRem=0;

    //TODO single vertex pattern.
    //printf("gV %i pV %i\n",w->number, patternVertex->number);
    if(isLeaf(patternVertex)){
        //printf("leaf %i\n",w->number);
        for(edgehlp=w->neighborhood; edgehlp; edgehlp=edgehlp->next)
            if(((edgehlp->flag & 1) && (checkCharacteristic(newCharacteristics,edgehlp->endPoint->number,patternVertex->number, patternVertex->neighborhood->endPoint->number, treeID)))
                    ||((edgehlp->flag & 2) && (checkCharacteristic(oldCharacteristics,edgehlp->endPoint->number,patternVertex->number, patternVertex->neighborhood->endPoint->number, wTreeID)))){
                freeCharacteristics(ret);
                return insertCharacteristic(NULL, patternSize, w->number, patternVertex->number, patternVertex->number, treeID);
            }
        return insertCharacteristic(ret,patternSize,w->number, patternVertex->neighborhood->endPoint->number, patternVertex->number, treeID);
    }
    
    //printf("Checking neighbours of gV\n");
    for(edgehlp=w->neighborhood; edgehlp;edgehlp=edgehlp->next){
        //printf("\t(%i %i)" ,edgehlp->endPoint->number, edgehlp->flag);
        if((edgehlp->flag)){//part of the tree
           ++numTree;
        }
    }
    //printf("\n");
    numPattern=degree(patternVertex);
    if(numPattern-1>numTree) return ret;
    edgehlp=patternVertex->neighborhood;
    bipartite=createGraph(numPattern+numTree, gPool);
    //printf("\n nP %i nt %i  T %i wT %i\n",numPattern, numTree, treeID, wTreeID);
    for(;i<numPattern;++i){
        bipartite->vertices[i]->lowPoint =0;
        //setting vertexnumber for neighborhood.
        edgehlp->endPoint->d=i;
        edgehlp2=w->neighborhood;
        bipartite->vertices[i]->d=edgehlp->endPoint->number;
        for(j=0;j<numTree;++j){
            while(!(edgehlp2->flag)) edgehlp2=edgehlp2->next;
            /*
            printf("checking edge ");
            if(edgehlp2) printf(" (gN %i %i) ", edgehlp2->endPoint->number ,edgehlp2->flag);
            if((edgehlp)) printf(" (pN %i) ", edgehlp->endPoint->number);
            printf("\n");
            */
            if( (((edgehlp2->flag & 1) && checkCharacteristic(newCharacteristics,edgehlp2->endPoint->number,patternVertex->number, edgehlp->endPoint->number,treeID))
                    ||((edgehlp2->flag & 2) && checkCharacteristic(oldCharacteristics, edgehlp2->endPoint->number, patternVertex->number, edgehlp->endPoint->number,wTreeID)))){
                edgehlp3=getVertexList(gPool->listPool);
                edgehlp4=getVertexList(gPool->listPool);
                edgehlp3->label = (char *)edgehlp4;
                edgehlp3->startPoint=bipartite->vertices[i];
                ++(bipartite->vertices[i]->lowPoint);
                edgehlp3->endPoint=bipartite->vertices[numPattern+j];
                addEdge(edgehlp3->startPoint,edgehlp3);
                edgehlp4->label = (char *)edgehlp3;
                edgehlp4->endPoint=bipartite->vertices[i];
                edgehlp4->startPoint=bipartite->vertices[numPattern+j];
                addEdge(edgehlp4->startPoint,edgehlp4);
                ++(bipartite->m);
                //printf("bedge %i,%i\t\n",i,numPattern+j);
            }
            edgehlp2=edgehlp2->next;
        }
        //printf("%i=%i\n",degree(bipartite->vertices[i]),bipartite->vertices[i]->lowPoint);
        edgehlp=edgehlp->next;
    }
    //printf("\n");
    if(bipartite->m<numPattern-1){
        dumpGraph(gPool, bipartite);
        return ret;
    }
    bipartite->number=numPattern;
    initBipartite(bipartite);
    matchingSize=bipartiteMatchingFastAndDirty(bipartite, gPool); 
    //printf("\nmatching size is %i\n", matchingSize);
    if(matchingSize==numPattern){//we have a matching that covers.
        //printf("\n\n\nAND IT MATCHES!!!!111elf\n\n\n");
        freeCharacteristics(ret);
        dumpGraph(gPool,bipartite);
        return insertCharacteristic(NULL, patternSize, w->number, patternVertex->number, patternVertex->number, treeID);
    }
    // now start removing neighbours.
    /*
    int p;
    struct VertexList *debug;
    printf("\n complete bipartie\n");
        for(p=0;p<numPattern+numTree;++p){
            printf("%i",bipartite->vertices[p]->number);
            for(debug=bipartite->vertices[p]->neighborhood;debug;debug=debug->next) printf("  (%i cb  %i)", debug->startPoint->number, debug->endPoint->number);
        }
    */
    for(j=0;j<numPattern;++j){
        //printf("rem vert %i ",j);
        //printf("%i %i \n",bipartite->vertices[j]->lowPoint , degree(bipartite->vertices[i]));
        if(bipartite->m - bipartite->vertices[j]->lowPoint >= numPattern-1){
            tmprm=bipartite->vertices[j]->neighborhood;
            bipartite->vertices[j]->neighborhood=NULL;
            for(edgehlp=tmprm;edgehlp;edgehlp=edgehlp->next){
                deleteEdge(bipartite, edgehlp->endPoint->number, j);
                ++numRem;
            }
            
            bipartite->m -= numRem;
            /*
            printf("after removal " );
            for(p=0;p<numPattern+numTree;++p){
                printf("%i",bipartite->vertices[p]->number);
                for(debug=bipartite->vertices[p]->neighborhood;debug;debug=debug->next) printf("  (%i ar %i)", debug->startPoint->number, debug->endPoint->number);
            }
            */
            initBipartite(bipartite);
            matchingSize=bipartiteMatchingFastAndDirty(bipartite, gPool);
            //printf("\nmatching size is %i\n", matchingSize);
            if(matchingSize ==numPattern-1){//we have a matching that covers.
                //printf("\nsmall matching\n");
                ret=insertCharacteristic(ret, patternSize, w->number, bipartite->vertices[j]->d, patternVertex->number, treeID);
            }
        
        //reinsert removed Edges.
            bipartite->vertices[j]->neighborhood=tmprm;
            for(edgehlp=tmprm;edgehlp;edgehlp=edgehlp->next){
                edgehlp2=(struct VertexList *)edgehlp->label;
                addEdge(edgehlp2->startPoint,edgehlp2);
            }
            /*
            printf("\nAfter reinsertion: \n");
            for(p=0;p<numPattern+numTree;++p){
                printf("%i",bipartite->vertices[p]->number);
                for(debug=bipartite->vertices[p]->neighborhood;debug;debug=debug->next) printf("  (%i ai %i)", debug->startPoint->number, debug->endPoint->number);
            }
            */
            bipartite->m += numRem;
            numRem=0;
        }
    }
    dumpGraph(gPool,bipartite);
    return ret;
}

void initSpanningTree( struct ShallowGraph *rootedComponent, struct VertexList **deletedEdges, int mask){
    int i=0;
    struct ShallowGraph *shlp;
    struct VertexList *edgehlp;
    //printf("Initializing spanning tree for vertex %i\n", rootedComponent->edges->startPoint->number);
    for(shlp=rootedComponent; shlp; shlp=shlp->next){
        if(shlp->m > 1){
            //printf("Deleteing edge %i %i", shlp->edges->startPoint->number, shlp->edges->endPoint->number);
            deletedEdges[i]=shlp->edges;
            unmarkEdgeBetweenVertices(deletedEdges[i]->startPoint, deletedEdges[i]->endPoint,mask);
            for(edgehlp=deletedEdges[i]->next;edgehlp;edgehlp=edgehlp->next){
                //printf("marking edge reverse to %i %i", edgehlp->startPoint->number, edgehlp->endPoint->number);
                markEdgeBetweenVertices(edgehlp->endPoint, edgehlp->startPoint, mask);
            }
        }
        else{
            deletedEdges[i]=NULL;
            markEdgeBetweenVertices(shlp->edges->startPoint, shlp->edges->endPoint,mask);
        }
        i++;
    }
}




char nextSpanningTree(struct ShallowGraph *rootedComponent, struct VertexList **deletedEdges, int mask){
    int i =-1;
    struct ShallowGraph *shlp=rootedComponent;
    struct VertexList *ehlp;
    while(shlp){
        if(deletedEdges[++i]==NULL) shlp=shlp->next;
        else{//indicator for component of size > 1
            //Reinsert Edge into Graph
            markEdgeBetweenVertices(deletedEdges[i]->startPoint,deletedEdges[i]->endPoint,mask);
            //advance insertion point
            if(deletedEdges[i]->next==NULL){//The last edge removed was the last of the component.
                //reset to start of list, therfore we would have an already processed spanning tree and do not break but advance the next position.
                deletedEdges[i]=shlp->edges;
                unmarkEdgeBetweenVertices(shlp->edges->startPoint, shlp->edges->endPoint,mask);
                //we now have to reverse all edges, as we removed the first of that component/circle.
                for(ehlp=shlp->edges->next; ehlp; ehlp=ehlp->next){
                    unmarkEdgeBetweenVertices(ehlp->startPoint,ehlp->endPoint,mask);
                    markEdgeBetweenVertices(ehlp->endPoint, ehlp->startPoint,mask);
                }
                shlp=shlp->next;
            }
            else{
                deletedEdges[i]=deletedEdges[i]->next;
                //now remove the edge pointing in the other direction.
                unmarkEdgeBetweenVertices(deletedEdges[i]->endPoint, deletedEdges[i]->startPoint,mask);
                return 1;//we do not have to advance any further, this is a new spanning tree.
            }
        }
    }
    //We did not return early therefore we have now computed all possible spanning trees
    //No we unmark all the edges.
    for(shlp=rootedComponent;shlp;shlp=shlp->next){
        for(ehlp=shlp->edges;ehlp;ehlp=ehlp->next){
            unmarkEdgeBetweenVertices(ehlp->startPoint,ehlp->endPoint,mask);
            unmarkEdgeBetweenVertices(ehlp->endPoint,ehlp->startPoint,mask);
        }
    }
    return 0;
}
void markEdge(struct VertexList *e, int mask){
    e->flag=e->flag|mask;
    //printf("flag of (%i, %i) is %i\n",e->startPoint->number, e->endPoint->number, e->flag);
}

void unmarkEdge(struct VertexList *e, int mask){
    e->flag=~mask & e->flag;
    //printf("flag of (%i, %i) is %i\n",e->startPoint->number, e->endPoint->number, e->flag);
}

void markEdgeBetweenVertices(struct Vertex *start, struct Vertex *end, int mask){
    struct VertexList *hlp=start->neighborhood;
    for(;hlp;hlp=hlp->next){
        if(hlp->endPoint==end){
            markEdge(hlp, mask);
            return;
        }
    }
}

void unmarkEdgeBetweenVertices(struct Vertex *start, struct Vertex *end, int mask){
    struct VertexList *hlp=start->neighborhood;
    for(;hlp;hlp=hlp->next){
        if(hlp->endPoint==end){
            unmarkEdge(hlp, mask);
            return;
        }
    }
}

struct Characteristics *allocateCharacteristics(int size, int master){
    struct Characteristics *ret;
    int i=0, j=0;
    ret=malloc(sizeof(struct Characteristics));
    if (!ret) return NULL;
    ret->size=size;
    ret->masterID=master;
    ret->treeIDs=malloc(size*sizeof(struct TreeList **));
    if(!(ret->treeIDs)){
        free(ret);
        return NULL;
    }
    
    for(;i<size;++i){
        ret->treeIDs[i]=calloc(size,sizeof(struct TreeList*));
        if(!(ret->treeIDs[i])){
            for(j=0;j<i;++j){
                free(ret->treeIDs[j]);
            }
            free(ret->treeIDs);
            return NULL;
        }
    }
    
    return ret;
}

void freeCharacteristics(struct Characteristics *characteristics){
    struct Characteristics *chlp1,*chlp2;
    struct TreeList *hlp1,*hlp2;
    int i, j;
    chlp1=characteristics;
    while(chlp1){
        for(i=0;i<chlp1->size;++i){
            for(j=0;j<chlp1->size;++j){
                hlp1=chlp1->treeIDs[i][j];
                while(hlp1){
                    hlp2=hlp1->next;
                    free(hlp1);
                    hlp1=hlp2;
                }
            }
            free(chlp1->treeIDs[i]);
        }
        free(chlp1->treeIDs);
        chlp2=chlp1->next;
        free(chlp1);
        chlp1=chlp2;
    }
}

struct Characteristics *mergeCharacteristics(struct Characteristics *one, struct Characteristics *two){
    struct Characteristics *start, *hlp1=one, *hlp2=two, *hlp3;
    int i=0;
    if(one == NULL) return two;
    if(two == NULL) return one;
    /*
    printf("\n\t\t\tmerging chars\n\n");
    printCharacteristics(one);
    printf("\n===============\n");
    printCharacteristics(two);
    printf("\t both are not null\n");
    printf("\t %i\n",hlp1);
    printf("\t %i\n", hlp2->masterID);
    */
    if(hlp1->masterID==hlp2->masterID){
   // printf("\t equal\n");
        hlp3=hlp2;
        hlp2=hlp2->next;
        hlp1->treeIDs=mergeTreeArrays(hlp1->treeIDs, hlp3->treeIDs, hlp1->size);
        for(;i<hlp3->size;++i){
            free(hlp3->treeIDs[i]);
        }
        free(hlp3->treeIDs);
        free(hlp3);
    }
    if(hlp1->masterID>hlp2->masterID){
    //printf("\t first larger\n");
        hlp3=hlp1;
        hlp1=hlp2;
        hlp2=hlp3;
    }
    start=hlp1;
    hlp1=start->next;
    hlp3=start;
    while(hlp1 && hlp2){
    //printf("\t while loop\n");
        if(hlp1->masterID<hlp2->masterID){
            hlp3->next=hlp1;
            hlp3=hlp1;
            hlp1=hlp1->next;
        }
        else{
            hlp3->next=hlp2;
            hlp3=hlp2;
            if(hlp1->masterID==hlp3->masterID){
                hlp3->treeIDs=mergeTreeArrays(hlp3->treeIDs, hlp1->treeIDs, hlp1->size);
                for(i=0;i<hlp1->size;++i){
                    free(hlp1->treeIDs[i]);
                }
                free(hlp1->treeIDs);
                hlp2=hlp1->next;
                free(hlp1);
                hlp1=hlp2;
            }
            hlp2=hlp3->next;
        }
    }
    if(hlp1) hlp3->next=hlp1;
    else hlp3->next=hlp2;
    //printCharacteristics(start);
    return start;
}

struct Characteristics *insertCharacteristic(struct Characteristics * characteristics, int size, int masterID, int patternRoot, int patternSubRoot, int treeID){
    struct Characteristics *start=characteristics, *hlp1=characteristics, *hlp2;
    //printf("\tchar %i %i %i %i\n", masterID, patternRoot,patternSubRoot,treeID);
    if(hlp1 && hlp1->masterID<=masterID){
        //printf("\t\t\tinster at middle of chars\n");
        while(hlp1->next && hlp1->next->masterID <= masterID){
            hlp1=hlp1->next;
        }
        if(hlp1->masterID<masterID){
       // printf("\t\t\tinster at new Entry of chars\n");
            hlp2=allocateCharacteristics(size, masterID);
            hlp2->next=hlp1->next;
            hlp1->next=hlp2;
            hlp1=hlp2;
        }
        // o/w hlp1 is already at the equal entry.
    }
    else{//hlp1==NULL or beginning is larger.
        //printf("\t\t\tinster at beginning of chars\n");
        start=allocateCharacteristics(size,masterID);
        start->next=hlp1;//hlp1 is either NULL or the beginning.
        hlp1=start;
        
        // o/w we have already the right entry as it has the correct masterID.
    }
   // printCharacteristics(hlp1);
    //printf("insert %i %i %i at master %i\n",patternRoot, patternSubRoot, treeID,hlp1->masterID);
    //printTL(hlp1->treeIDs[patternRoot][patternSubRoot]);
    hlp1->treeIDs[patternRoot][patternSubRoot]=insertTree(hlp1->treeIDs[patternRoot][patternSubRoot],treeID);
    //printTL(hlp1->treeIDs[patternRoot][patternSubRoot]);
   // printCharacteristics(hlp1);
    //printf("char for return \n");
    //printCharacteristics(start);
    return start;
}

char hasSolution(struct Characteristics *characteristic){
    struct Characteristics *chlp=characteristic;
    int i;
    for(;chlp;chlp=chlp->next){
        for(i=0;i<chlp->size;++i){
            if(chlp->treeIDs[i][i]){
 //               printf("\n\nFOUND SOMETHING\n\n)");
                return 1;
            }
        }
    }
    return 0;
}



struct TreeList ***mergeTreeArrays(struct TreeList ***one, struct TreeList ***two, int size){
    int i=0,j=0;
    for(;i<size;++i){
        for(j=0;j<size;++j){
            one[i][j]=mergeTreeLists(one[i][j], two[i][j]);
        }
        free(two[i]);
    }
    free(two);
    return one;
}

struct TreeList *mergeTreeLists(struct TreeList *one, struct TreeList *two){
    struct TreeList *hlp1=one, *hlp2=two, *hlp3, *start;
    if(!one) return two;
    if(!two) return one;
    if(one->treeID==two->treeID){//can only happen once as we keep the list sorted and compressed.
        hlp1=one->next;
        free(one);
    }
    //hlp1 may be null, if one had only one element.
    if(!hlp1) return two;
    if(hlp1->treeID>hlp2->treeID){
        hlp3=hlp1;
        hlp1=hlp2;
        hlp2=hlp1;
    }
    start=hlp1;
    hlp1=hlp1->next;
    hlp3=start;
    while(hlp1 && hlp2){
        if(hlp1->treeID < hlp2->treeID){
            hlp3->next=hlp1;
            hlp1=hlp1->next;
        }
        else{
            hlp3->next=hlp2;
            if(hlp1->treeID==hlp2->treeID){
                hlp2=hlp1->next;
                free(hlp1);
                hlp1=hlp2;
                hlp2=hlp3;
            }
            hlp2=hlp2->next;
        }
        hlp3=hlp3->next;
    }
    if(hlp1) hlp3->next=hlp1;
    if(hlp2) hlp3->next=hlp2;
    return start;
}



struct TreeList* insertTree(struct TreeList *trees, int treeID){
    struct TreeList *treehlp=trees, *treehlp2=NULL;
    if(treehlp){
        if(treehlp->treeID <= treeID){
            while(treehlp->next && treehlp->next->treeID <= treeID){
                treehlp=treehlp->next;
            }
            if(treehlp->treeID<treeID){
                //printf("Inserting new tree in the middle\n");
                treehlp2=treehlp->next;
                treehlp->next=malloc(sizeof(struct TreeList));
                treehlp=treehlp->next;
                treehlp->treeID=treeID;
                treehlp->next=treehlp2;
            }
            return trees;
        }
    }
    //printf("insert at the beginning\n");
    treehlp2=malloc(sizeof(struct TreeList));
    treehlp2->treeID=treeID;
    treehlp2->next=treehlp;//treehlp is either null or has a larger treeID
    return treehlp2;
}
char checkCharacteristic(struct Characteristics *characteristics, const int masterID, const int patternRoot, const int patternSubRoot, const int treeID){
    struct Characteristics *hlp=characteristics;
    for(; hlp && hlp->masterID < masterID; hlp=hlp->next);
    if (hlp && hlp->masterID==masterID) return checkTreeList(hlp->treeIDs[patternRoot][patternSubRoot], treeID);
    return 0;
}

char checkTreeList(struct TreeList *treeList, const int treeID){
    struct TreeList *hlp=treeList;
    for(;hlp && hlp->treeID < treeID;hlp=hlp->next);
    if(hlp && hlp->treeID == treeID) return 1;
    return 0;
}






int bfs(struct Graph *graph){

    struct Vertex **queue;
    struct VertexList *hlp;
    int i;
    queue=malloc(graph->n * sizeof(struct Vertex*));
    if(queue==NULL){
        printf("Could not initalise queue for BFS");
        return EXIT_FAILURE;
    }
    for(i=0;i<graph->n;++i) graph->vertices[i]->visited=0;

    for(i=0; i<graph->n;++i) graph->vertices[i]->visited=0;
    //TODO  disconnected graph here or preProcessing??
    queue[0] = graph->vertices[0];
    queue[0]->visited=1;
    queue[0]->d = 0; //TODO is using d ok???
    int end = 0;
    for(i=0;i<graph->n;++i){
        for(hlp=queue[i]->neighborhood;hlp;hlp=hlp->next) if(!hlp->endPoint->visited){
            queue[++end]=hlp->endPoint;
            queue[end]->visited=1;
            queue[end]->d=queue[i]->d+1;
        }

    } 
    free(queue);
    return EXIT_SUCCESS;
}

    
void printCharacteristics(struct Characteristics *chars){
    struct Characteristics *hlp;
    struct TreeList *thlp;
    int i,j ,k;
    for(hlp=chars; hlp; hlp=hlp->next){
        k=0;
        printf("\n\tCA %li TIA %li",(long) hlp, (long)hlp->treeIDs);
        /*
    for (i=0;i<hlp->size;++i){
        printf("\t%i\n",hlp->treeIDs[i]);
        for(j=0;j<hlp->size;++j) printf("\t %i", hlp->treeIDs[i][j]);
        printf("\n\n");
    }
    */
    printf("\n\n");
        for(i=0;i<hlp->size;++i){
            for(j=0;j<hlp->size;++j){
                for(thlp=hlp->treeIDs[i][j];thlp;thlp=thlp->next){
                    printf("\t[%i %i %i %i ]",  hlp->masterID, i, j, thlp->treeID);
                    if(++k % 5 == 0){
                        printf("\n\t\t");
                        k=0;
                    }
                }
            }
        }
    }
    printf("\n");
}

void printTL(struct TreeList *tl){
    struct TreeList *hlp=tl;
    printf("\t[TL| ");
    for(;hlp;hlp=hlp->next) printf("%i   ",hlp->treeID);
    printf("\n");
}


void freeAllPools(struct GraphPool *gPool, struct ShallowGraphPool *sgPool){
    if(gPool!=NULL){

        freeListPool(gPool->listPool);
        freeVertexPool(gPool->vertexPool);
        freeGraphPool(gPool);
    }
    if(sgPool!=NULL) freeShallowGraphPool(sgPool);
}
