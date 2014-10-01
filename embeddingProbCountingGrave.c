// /**
// return the histogram of vertices and edges in a db in a search tree.
// traverses the db once.

// the db is expected to have the format outputted by printStringsInSearchTree().
// both output parameters (frequent* ) should be initialized to "empty" structs.
// fileName specifies the file the db is contained in.
// mingraph and maxgraph specify a range in which to read patterns.
// */
// void getVertexAndEdgeHistograms(char* fileName, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
// 	int bufferSize = 100;
// 	int i = 0;
// 	FILE* stream = fopen(fileName, "r");
// 	struct ShallowGraph* patterns = NULL;
// 	int number;
	
// 	/* iterate over all graphs in the database */
// 	while (((i < maxGraph) || (maxGraph == -1)) && (patterns = streamReadPatterns(stream, bufferSize, &number, sgp))) {
// 		if (i >= minGraph) {
// 			struct ShallowGraph* pattern = patterns;

// 			/* frequency of an edge increases by one if there exists a pattern for the current graph (a spanning tree) 
// 			that contains the edge. Thus we need to find all edges contained in any spanning tree and then add them 
// 			to frequentEdges once omitting multiplicity */
// 			struct Vertex* containedEdges = getVertex(gp->vertexPool);

// 			/* the vertices contained in g can be obtained from a single spanning tree, as all spanning trees contain
// 			the same vertex set. However, to omit multiplicity, we again resort to a temporary searchTree */
// 			struct Vertex* containedVertices = getVertex(gp->vertexPool);

// 			/* get frequent vertices */
// 			struct Graph* patternGraph = treeCanonicalString2Graph(pattern, gp);
// 			int v;
// 			for (v=0; v<patternGraph->n; ++v) {
// 				/* See commented out how it would look if done by the book.
// 				However, this has to be fast and canonicalStringOfTree has
// 				too much overhead!
// 				    struct ShallowGraph* cString;
// 				    auxiliary->vertices[0]->label = patternGraph->vertices[v]->label;
// 				    cString = canonicalStringOfTree(auxiliary, sgp);
// 				    addToSearchTree(containedVertices, cString, gp, sgp); */
// 				struct VertexList* cString = getVertexList(sgp->listPool);
// 				cString->label = patternGraph->vertices[v]->label;
// 				containedVertices->d += addStringToSearchTree(containedVertices, cString, gp);
// 				containedVertices->number += 1;
// 			}
// 			/* set multiplicity of patterns to 1 and add to global vertex pattern set, print to file */
// 			resetToUnique(containedVertices);
// 			mergeSearchTrees(frequentVertices, containedVertices, 1, NULL, NULL, frequentVertices, 0, gp);
// 			dumpSearchTree(gp, containedVertices);
			
// 			/* get frequent Edges */
// 			/* set the lowest id given to any edge to the highest id of any frequent vertex */
// 			frequentEdges->lowPoint = frequentVertices->lowPoint;
// 			for ( ; pattern!=NULL; pattern=pattern->next) {
// 				if (patternGraph == NULL) {
// 					patternGraph = treeCanonicalString2Graph(pattern, gp);
// 				}
// 				for (v=0; v<patternGraph->n; ++v) {
// 					struct VertexList* e;
// 					for (e=patternGraph->vertices[v]->neighborhood; e!=NULL; e=e->next) {
// 						int w = e->endPoint->number;
// 						/* edges occur twice in patternGraph. just add them once to the search tree */
// 						if (w > v) {
// 							/* as for vertices, I use specialized code to generate 
// 							the canonical string of a single edge */
// 							struct VertexList* cString;
// 							if (strcmp(e->startPoint->label, e->endPoint->label) < 0) {
// 								/* cString = v e (w) */
// 								struct VertexList* tmp = getVertexList(gp->listPool);
// 								tmp->label = e->endPoint->label;

// 								cString = getTerminatorEdge(gp->listPool);
// 								tmp->next = cString;

// 								cString = getVertexList(gp->listPool);
// 								cString->label = e->label;
// 								cString->next = tmp;

// 								tmp = getInitialisatorEdge(gp->listPool);
// 								tmp->next = cString;

// 								cString = getVertexList(gp->listPool);
// 								cString->label = e->startPoint->label;
// 								cString->next = tmp;
// 							} else {
// 								/* cString = w e (v) */
// 								struct VertexList* tmp = getVertexList(gp->listPool);
// 								tmp->label = e->startPoint->label;

// 								cString = getTerminatorEdge(gp->listPool);
// 								tmp->next = cString;

// 								cString = getVertexList(gp->listPool);
// 								cString->label = e->label;
// 								cString->next = tmp;

// 								tmp = getInitialisatorEdge(gp->listPool);
// 								tmp->next = cString;

// 								cString = getVertexList(gp->listPool);
// 								cString->label = e->endPoint->label;
// 								cString->next = tmp;
// 							}
// 							/* add the string to the search tree */
// 							containedEdges->d += addStringToSearchTree(containedEdges, cString, gp);
// 							containedEdges->number += 1;
// 						} 
// 					}
// 				}
// 				dumpGraph(gp, patternGraph);
// 				patternGraph = NULL;
// 			}
// 			/* set multiplicity of patterns to 1 and add to global edge pattern set */
// 			resetToUnique(containedEdges);
// 			mergeSearchTrees(frequentEdges, containedEdges, 1, NULL, NULL, frequentEdges, 0, gp);
// 			dumpSearchTree(gp, containedEdges);
// 		}

// 		/* counting of read graphs and garbage collection */
// 		++i;
// 		dumpShallowGraphCycle(sgp, patterns);
// 	}
// 	fclose(stream);
// }


// /**
// Walk through the db, checking for each graph g \in db which refinements are subtrees of at least one of its spanning 
// trees. for all these refinements, the visited counter of its cString in currentLevel is increased. 
// */
// void scanDB(char* fileName, struct Vertex* currentLevel, struct Graph** refinements, struct Vertex** pointers, int n, int minGraph, int maxGraph, int threshold, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
// 	int bufferSize = 100;
// 	int i = 0;
// 	FILE* stream = fopen(fileName, "r");
// 	struct ShallowGraph* spanningTreeStrings = NULL;
// 	int number;
// 	struct CachedGraph* subtreeCache = initCachedGraph(gp, 200);
	
// 	int dbg_firstPrune = 0;
// 	int dbg_subset = 0;
// 	int dbg_found = 0;

// 	int** features = malloc(maxGraph * sizeof(int*));
// 	for (i=0; i<maxGraph; ++i) {
// 		features[i] = malloc((n + 1) * sizeof(int));
// 	}
// 	i = 0;
	
// 	/* iterate over all graphs in the database */
// 	while (((i < maxGraph) || (maxGraph == -1)) && (spanningTreeStrings = streamReadPatterns(stream, bufferSize, &number, sgp))) {
// 		if (i >= minGraph) {
			
// 			struct ShallowGraph* spanningTreeString;
// 			struct Graph* spanningTree = NULL;
// 			int refinement;

// 			features[i][n] = number;
// 			if (features[i][n] == 0) {
// 				fprintf(stderr, "Reading error. 0 was read\n");
// 			}
// 			for (refinement=0; refinement<n; ++refinement) {
// 				features[i][refinement] = 0;
// 			}
			
// 			/* if there is no frequent pattern from lower level contained in i, dont even start searching */
// 			if (pruning[i] != 0) {
// 				/* set d to one, meaning that all refinements have not yet been recognized to be subtree
// 				of current graph */
// 				/* for each spanning tree */
// 				for (spanningTreeString=spanningTreeStrings; spanningTreeString!=NULL; spanningTreeString=spanningTreeString->next) {
// 					/* convert streamed spanning tree string to graph */
// 					if (spanningTree != NULL) {
// 						int v;
// 						for (v=0; v<spanningTree->n; ++v) {
// 							dumpVertexListRecursively(gp->listPool, spanningTree->vertices[v]->neighborhood);
// 							spanningTree->vertices[v]->neighborhood = NULL;
// 						}
// 						treeCanonicalString2ExistingGraph(spanningTreeString, spanningTree, gp);
// 					} else {
// 						spanningTree = treeCanonicalString2Graph(spanningTreeString, gp);
// 					}
				
// 					/* for each refinement */
// 					for (refinement=0; refinement<n; ++refinement) {
// 						if (isSubset(pointers[refinement]->d, i)) {
// 							--dbg_subset;
// 							/* if refinement is not already found to be subtree of current graph */
// 							if (!features[i][refinement]) {
// 								--dbg_found;
// 								/* if refinement is contained in spanning tree */
// 								if (subtreeCheckCached(spanningTree, refinements[refinement], gp, subtreeCache)) {
// 									/* currentLevel refinementstring visited +1 and continue with next refinement */
// 									features[i][refinement] = 1;
// 									++pointers[refinement]->visited;
// 									++currentLevel->number;
// 								} else {
// 									features[i][refinement] = 0;
// 								}
// 							} else {
// 								++dbg_found;
// 							}
// 						} else {
// 							++dbg_subset;
// 						}
// 					}
// 				}
// 				dumpGraph(gp, spanningTree);
// 			} else {
// 				++dbg_firstPrune;
// 			}
// 		}

// 		/* counting of read graphs and garbage collection */
// 		++i;
// 		dumpShallowGraphCycle(sgp, spanningTreeStrings);
// 	}
// 	fprintf(stderr, "initPrune=%i, skips=%i, subsetPrune=%i\n", dbg_firstPrune, dbg_found, dbg_subset);
// 	dumpCachedGraph(subtreeCache);
// 	fclose(stream);

// 	/* pruning and output of frequent patterns */ 
// 	maxGraph = i; /* only loop through graphs that were processed */
// 	for (i=minGraph; i<maxGraph; ++i) {
// 		int refinement;
// 		pruning[i] = 0;
// 		for (refinement=0; refinement<n; ++refinement) {
// 			if ((features[i][refinement] == 1) && (pointers[refinement]->visited >= threshold)) {
// 				fprintf(keyValueStream, "%i %i\n", features[i][n], pointers[refinement]->lowPoint);
// 				//debug
// 				if (features[i][n] == 0) {
// 					fprintf(stderr, "Damn.\n");
// 				}
// 				addToPruningSet(pointers[refinement]->lowPoint, i);
// 			}
// 		}
// 	}

// 	for (i=0; i<maxGraph; ++i) {
// 		free(features[i]);
// 	}
// 	free(features);
// }

// /**
// Walk through the db, checking for each graph g \in db which refinements are subtrees of at least one of its spanning 
// trees. for all these refinements, the visited counter of its cString in currentLevel is increased. 
// */
// void scanDBold(char* fileName, struct Vertex* currentLevel, struct Graph** refinements, struct Vertex** pointers, int n, int minGraph, int maxGraph, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
// 	int bufferSize = 100;
// 	int i = 0;
// 	FILE* stream = fopen(fileName, "r");
// 	struct ShallowGraph* spanningTreeStrings = NULL;
// 	int number;
// 	struct CachedGraph* subtreeCache = initCachedGraph(gp, 200);
// 	char* found = malloc(n * sizeof(char));
	
// 	int dbg_firstPrune = 0;
// 	int dbg_subset = 0;
// 	int dbg_found = 0;
	
// 	/* iterate over all graphs in the database */
// 	while (((i < maxGraph) || (maxGraph == -1)) && (spanningTreeStrings = streamReadPatterns(stream, bufferSize, &number, sgp))) {
// 		if (i >= minGraph) {
			
// 			struct ShallowGraph* spanningTreeString;
// 			struct Graph* spanningTree = NULL;
// 			int refinement;
// 			int newBloomFilter = 0;

// 			/* if there is no frequent pattern from lower level contained in i, dont even start searching */
// 			if (pruning[i] != 0) {
// 				 set d to one, meaning that all refinements have not yet been recognized to be subtree
// 				of current graph 
// 				for (refinement=0; refinement<n; ++refinement) {
// 					found[refinement] = 0;
// 				}
// 				/* for each spanning tree */
// 				for (spanningTreeString=spanningTreeStrings; spanningTreeString!=NULL; spanningTreeString=spanningTreeString->next) {
// 					/* convert streamed spanning tree string to graph */
// 					if (spanningTree != NULL) {
// 						int v;
// 						for (v=0; v<spanningTree->n; ++v) {
// 							dumpVertexListRecursively(gp->listPool, spanningTree->vertices[v]->neighborhood);
// 							spanningTree->vertices[v]->neighborhood = NULL;
// 						}
// 						treeCanonicalString2ExistingGraph(spanningTreeString, spanningTree, gp);
// 					} else {
// 						spanningTree = treeCanonicalString2Graph(spanningTreeString, gp);
// 					}
				
// 					/* for each refinement */
// 					for (refinement=0; refinement<n; ++refinement) {
// 						if (isSubset(pointers[refinement]->d, i)) {
// 							/* if refinement is not already found to be subtree of current graph */
// 							if (!found[refinement]) {
// 								/* if refinement is contained in spanning tree */
// 								if (subtreeCheckCached(spanningTree, refinements[refinement], gp, subtreeCache)) {
// 									/* currentLevel refinementstring visited +1 and continue with next refinement */
// 									found[refinement] = 1;
// 									++pointers[refinement]->visited;
// 									++currentLevel->number;
// 									fprintf(keyValueStream, "%i %i\n", number, pointers[refinement]->lowPoint);
// 									newBloomFilter |= hashID(pointers[refinement]->lowPoint);
// 								}
// 							} else {
// 								++dbg_found;
// 							}
// 						} else {
// 							++dbg_subset;
// 						}
// 					}
// 				}
// 				pruning[i] = newBloomFilter;
// 				dumpGraph(gp, spanningTree);
// 			} else {
// 				++dbg_firstPrune;
// 			}
// 		}

// 		/* counting of read graphs and garbage collection */
// 		++i;
// 		dumpShallowGraphCycle(sgp, spanningTreeStrings);
// 	}
// 	fprintf(stderr, "initPrune=%i, skips=%i, subsetPrune=%i\n", dbg_firstPrune, dbg_found, dbg_subset);
// 	dumpCachedGraph(subtreeCache);
// 	free(found);
// 	fclose(stream);
// }


// /**
// Walk through the db, checking for each graph g \in db which refinements are subtrees of at least one of its spanning 
// trees. for all these refinements, the visited counter of its cString in currentLevel is increased. 
// */
// void scanInMemoryDB(struct Graph** tp, int* tnp, struct Vertex* currentLevel, struct Graph** refinements, 
// 		struct Vertex** pointers, int n, int minGraph, int maxGraph, int threshold, FILE* keyValueStream, 
// 		struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	
// 	int i = 0;
// 	struct CachedGraph* subtreeCache = initCachedGraph(gp, 200);
	
// 	int dbg_firstPrune = 0;
// 	int dbg_subset = 0;
// 	int dbg_found = 0;

// 	int** features = malloc(maxGraph * sizeof(int*));
// 	for (i=0; i<maxGraph; ++i) {
// 		features[i] = malloc((n + 1) * sizeof(int));
// 	}
	
// 	/* iterate over all graphs in the database */
// 	for (i=0; i<maxGraph; ++i) {
// 		if (i >= minGraph) {
			
// 			int refinement;

// 			features[i][n] = tnp[i];
// 			/* set d to one, meaning that all refinements have not yet been recognized to be subtree
// 			of current graph */
// 			for (refinement=0; refinement<n; ++refinement) {
// 				features[i][refinement] = 0;
// 			}
			
// 			/* if there is no frequent pattern from lower level contained in i, dont even start searching */
// 			if (pruning[i] != 0) {
// 				/* for each spanning tree */
// 				struct Graph* spanningTree;
// 				for (spanningTree=tp[i]; spanningTree!=NULL; spanningTree=spanningTree->next) {				
// 					/* for each refinement */
// 					for (refinement=0; refinement<n; ++refinement) {
// 						if (isSubset(pointers[refinement]->d, i)) {
// 							--dbg_subset;
// 							/* if refinement is not already found to be subtree of current graph */
// 							if (!features[i][refinement]) {
// 								--dbg_found;
// 								/* if refinement is contained in spanning tree */
// 								if (subtreeCheckCached(spanningTree, refinements[refinement], gp, subtreeCache)) {
// 									/* currentLevel refinementstring visited +1 and continue with next refinement */
// 									features[i][refinement] = 1;
// 									++pointers[refinement]->visited;
// 									++currentLevel->number;
// 								} else {
// 									features[i][refinement] = 0;
// 								}
// 							} else {
// 								++dbg_found;
// 							}
// 						} else {
// 							++dbg_subset;
// 						}
// 					}
// 					dumpGraph(gp, spanningTree);
// 				}
// 			} else {
// 				++dbg_firstPrune;
// 			}
// 		}
// 	}
// 	fprintf(stderr, "initPrune=%i, skips=%i, subsetPrune=%i\n", dbg_firstPrune, dbg_found, dbg_subset);
// 	dumpCachedGraph(subtreeCache);	

// 	/* pruning and output of frequent patterns */ 
// 	maxGraph = i; /* only loop through graphs that were processed */
// 	for (i=minGraph; i<maxGraph; ++i) {
// 		int refinement;
// 		pruning[i] = 0;
// 		for (refinement=0; refinement<n; ++refinement) {
// 			if ((features[i][refinement] == 1) && (pointers[refinement]->visited >= threshold)) {
// 				fprintf(keyValueStream, "%i %i\n", features[i][n], pointers[refinement]->lowPoint);
// 				//debug
// 				if (features[i][n] == 0) {
// 					fprintf(stderr, "Damn.\n");
// 				}
// 				addToPruningSet(pointers[refinement]->lowPoint, i);
// 			}
// 		}
// 	}

// 	for (i=0; i<maxGraph; ++i) {
// 		free(features[i]);
// 	}
// 	free(features);
// }

// /**
// return the histogram of vertices and edges in a db in a search tree.
// traverses the db once.

// the db is expected to have the format outputted by printStringsInSearchTree().
// both output parameters (frequent* ) should be initialized to "empty" structs.
// fileName specifies the file the db is contained in.
// mingraph and maxgraph specify a range in which to read patterns.
// */
// void getVertexAndEdgeHistogramsInMemory(struct Graph** tp, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
// 	int i = 0;
// 	struct compInfo* results = NULL;
// 	int resultSize = 0;
	
// 	/* iterate over all graphs in the database */
// 	for (i=minGraph; i<maxGraph; ++i) {
// 		struct Graph* patternGraph = tp[i];
// 		int v;

// 		/* frequency of an edge increases by one if there exists a pattern for the current graph (a spanning tree) 
// 		that contains the edge. Thus we need to find all edges contained in any spanning tree and then add them 
// 		to frequentEdges once omitting multiplicity */
// 		struct Vertex* containedEdges = getVertex(gp->vertexPool);

// 		/* the vertices contained in g can be obtained from a single spanning tree, as all spanning trees contain
// 		the same vertex set. However, to omit multiplicity, we again resort to a temporary searchTree */
// 		struct Vertex* containedVertices = getVertex(gp->vertexPool);

// 		/* init temporary result storage if necessary */
// 		int neededResultSize = patternGraph->m + 1;
// 		int resultPos = 0;
// 		if (neededResultSize > resultSize) {
// 			if (results) {
// 				free(results);
// 			}

// 			results = getResultVector(neededResultSize);
// 			resultSize = neededResultSize;
// 		}

// 		/* get frequent vertices */
// 		for (v=0; v<patternGraph->n; ++v) {
// 			/* See commented out how it would look if done by the book.
// 			However, this has to be fast and canonicalStringOfTree has
// 			too much overhead!
// 			    struct ShallowGraph* cString;
// 			    auxiliary->vertices[0]->label = patternGraph->vertices[v]->label;
// 			    cString = canonicalStringOfTree(auxiliary, sgp);
// 			    addToSearchTree(containedVertices, cString, gp, sgp); */
// 			struct VertexList* cString = getVertexList(sgp->listPool);
// 			cString->label = patternGraph->vertices[v]->label;
// 			containedVertices->d += addStringToSearchTree(containedVertices, cString, gp);
// 			containedVertices->number += 1;
// 		}
// 		/* set multiplicity of patterns to 1 and add to global vertex pattern set, print to file */
// 		resetToUnique(containedVertices);
// 		mergeSearchTrees(frequentVertices, containedVertices, 1, results, &resultPos, frequentVertices, 0, gp);
// 		dumpSearchTree(gp, containedVertices);

// 		/* write (graph->number, pattern id) pairs to stream */
// 		for (v=0; v<resultPos; ++v) {
// 			// TODO change i below to patternGraph->number for producing same result as levelwiseMain
// 			fprintf(keyValueStream, "%i %i\n", i, results[v].id);
// 		}
		
// 		/* get frequent Edges */
// 		resultPos = 0;
// 		for ( ; patternGraph!=NULL; patternGraph=patternGraph->next) {
// 			for (v=0; v<patternGraph->n; ++v) {
// 				struct VertexList* e;
// 				for (e=patternGraph->vertices[v]->neighborhood; e!=NULL; e=e->next) {
// 					int w = e->endPoint->number;
// 					/* edges occur twice in patternGraph. just add them once to the search tree */
// 					if (w > v) {
// 						/* as for vertices, I use specialized code to generate 
// 						the canonical string of a single edge */
// 						struct VertexList* cString;
// 						if (strcmp(e->startPoint->label, e->endPoint->label) < 0) {
// 							/* cString = v e (w) */
// 							struct VertexList* tmp = getVertexList(gp->listPool);
// 							tmp->label = e->endPoint->label;

// 							cString = getTerminatorEdge(gp->listPool);
// 							tmp->next = cString;

// 							cString = getVertexList(gp->listPool);
// 							cString->label = e->label;
// 							cString->next = tmp;

// 							tmp = getInitialisatorEdge(gp->listPool);
// 							tmp->next = cString;

// 							cString = getVertexList(gp->listPool);
// 							cString->label = e->startPoint->label;
// 							cString->next = tmp;
// 						} else {
// 							/* cString = w e (v) */
// 							struct VertexList* tmp = getVertexList(gp->listPool);
// 							tmp->label = e->startPoint->label;

// 							cString = getTerminatorEdge(gp->listPool);
// 							tmp->next = cString;

// 							cString = getVertexList(gp->listPool);
// 							cString->label = e->label;
// 							cString->next = tmp;

// 							tmp = getInitialisatorEdge(gp->listPool);
// 							tmp->next = cString;

// 							cString = getVertexList(gp->listPool);
// 							cString->label = e->endPoint->label;
// 							cString->next = tmp;
// 						}
// 						/* add the string to the search tree */
// 						containedEdges->d += addStringToSearchTree(containedEdges, cString, gp);
// 						containedEdges->number += 1;
// 					} 
// 				}
// 			}
// 		}
// 		/* set multiplicity of patterns to 1 and add to global edge pattern set */
// 		resetToUnique(containedEdges);
// 		mergeSearchTrees(frequentEdges, containedEdges, 1, results, &resultPos, frequentEdges, 0, gp);
// 		dumpSearchTree(gp, containedEdges);
		
// 		/* write (graph->number, pattern id) pairs to stream, add the patterns to the bloom
// 		filter of the graph (i) for pruning */
// 		for (v=0; v<resultPos; ++v) {
// 			// TODO change i below to patternGraph->number for producing same result as levelwiseMain
// 			fprintf(keyValueStream, "%i %i\n", i, results[v].id);
// 			addToPruningSet(results[v].id, i);
// 		}
// 	}
// 	if (results != NULL) {
// 		free(results);
// 	}
// }