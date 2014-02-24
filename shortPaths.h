#ifndef SHORT_PATHS_H_
#define SHORT_PATHS_H_

struct PathGenerator {
	struct ShallowGraphPool* sgp;
	char** vertexLabels;
	char** edgeLabels;
	struct currentPath;
	int maxLength;
	int level;
	int* stack;
};

char containsPath(struct Graph* g, struct ShallowGraph* path);

#endif /* SHORT_PATHS_H_ */
